//----------------------------------------------------------------------------//
// This file is a part of The Moonshot Project.        
// See LICENSE.txt for license info.            
// File : Driver.cpp                      
// Author : Pierre van Houtryve                      
//----------------------------------------------------------------------------//

#include "Fox/Driver/Driver.hpp"
#include "Fox/Lexer/Lexer.hpp"
#include "Fox/Parser/Parser.hpp"
#include "Fox/AST/ASTDumper.hpp"
#include "Fox/AST/ASTContext.hpp"
#include "Fox/Sema/Sema.hpp"
#include "Fox/AST/Decl.hpp"
#include "Fox/Common/LLVM.hpp"
#include "Fox/Common/DiagnosticVerifier.hpp"
#include <fstream>

using namespace fox;

Driver::Driver(std::ostream& os): os_(os), diags(srcMgr, os_) {}

bool Driver::processFile(const std::string& filepath) {
  // Load the file in the source manager
  auto fid = srcMgr.loadFromFile(filepath);
  if (!fid) {
    getOS() << "Could not open file \"" << filepath << "\"\n";
    return false;
  }

	// Enable verify mode if needed
	std::unique_ptr<DiagnosticVerifier> dv;
	if (getVerifyMode() != VerifyMode::Disabled) {
		dv = std::make_unique<DiagnosticVerifier>(diags, srcMgr);
		dv->parseFile(fid);
    diags.enableVerifyMode(dv.get());
	}

	// Do lexing
  Lexer lex(diags, srcMgr, ctxt);
  {
    auto chrono = createChrono("Lexing");
    lex.lexFile(fid);
  }

  // Stop if we had errors
  if (diags.getErrorsCount())
    return false;

  Parser psr(diags, srcMgr, ctxt, lex.getTokenVector());

  UnitDecl* unit;
  // Do parsing
  {
    auto chrono = createChrono("Parsing");
    unit = psr.parseUnit(fid, ctxt.identifiers.getUniqueIdentifierInfo("TestUnit"), /* is main unit */ true);
  }

  // Stop if we had errors or if the unit is invalid
  if (!unit || diags.getErrorsCount())
    return (diags.getErrorsCount() == 0);

  // Dump alloc if needed
  if (getDumpAlloc()) {
    getOS() << "\nDumping allocator:\n";
    ctxt.getAllocator().dump(getOS());
  }

  // Semantic analysis testing stuff
  for (auto& decl : unit->getDecls()) {
    if (FuncDecl* fn = dyn_cast<FuncDecl>(decl)) {
      CompoundStmt* body = fn->getBody();
      for (auto& node : body->getNodes()) {
        if (auto* expr = node.getIf<Expr>())
          node = Sema(ctxt, diags).typecheckExpr(expr);
      }
    }
  }

  // Dump AST if needed
  if (getDumpAST()) {
    auto chrono = createChrono("AST Printing");
    getOS() << "\nAST Dump:\n";
    ASTDumper(srcMgr, getOS(), 1).visit(ctxt.getMainUnit());
  }

  // (Verify mode) Check that all diags were emitted if we're
  // in verify-strict mode.
  if (getVerifyMode() == VerifyMode::Normal) {
    assert(dv && "DiagnosticVerifier is null in Strict mode");
    dv->reportUnemittedDiags();
  }

  // Release the memory
  {
    auto chrono = createChrono("Release");
    ctxt.reset();
  }

  return (diags.getErrorsCount() == 0);
}

bool Driver::getPrintChrono() const {
  return chrono_;
}

void Driver::setPrintChrono(bool val) {
  chrono_ = val;
}

Driver::VerifyMode Driver::getVerifyMode() const {
  return verify_;
}

void Driver::setVerifyMode(VerifyMode val) {
  verify_ = val;
}

bool Driver::getDumpAlloc() const {
  return dumpAlloc_;
}

void Driver::setDumpAlloc(bool val) {
  dumpAlloc_ = val;
}

bool Driver::getDumpAST() const {
  return dumpAST_;
}

void Driver::setDumpAST(bool val) {
  dumpAST_ = val;
}

Driver::RAIIChrono Driver::createChrono(string_view label) {
  return RAIIChrono(*this, label);
}

std::ostream& Driver::getOS() {
  return os_;
}

bool Driver::doCL(int argc, char* argv[]) {
  // Must have 2 args, first is executable path, second should
  // be filepath => argc must be >= 2
  if (argc < 2) {
    getOS() << "Not enough args\n";
    return false;
  }

  // Get file path
  std::string filepath = argv[1];

  for(int idx = 2; idx < argc; ++idx) {
    string_view str(argv[idx]);
    if (str == "-verify")
      setVerifyMode(VerifyMode::Normal);
    else if (str == "-verify-soft")
      setVerifyMode(VerifyMode::Soft);
    else if (str == "-werr")
      diags.setWarningsAreErrors(true);
    else if (str == "-dump_ast")
      setDumpAST(true);
    else if (str == "-dump_alloc")
      setDumpAlloc(true);
    else if (str == "-print_chrono")
      setPrintChrono(true);
    else {
      getOS() << "Unknown argument \"" << str << "\"\n";
      return false;
    }
  }
 
  return processFile(filepath);
}
