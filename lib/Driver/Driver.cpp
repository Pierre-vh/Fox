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

Driver::Driver(std::ostream& os): os_(os), diags(srcMgr, os_),
  ctxt(srcMgr, diags) {}

bool Driver::processFile(const std::string& filepath) {
  // Load the file in the source manager
  auto fid = srcMgr.loadFromFile(filepath);
  if (!fid) {
    getOS() << "Could not open file \"" << filepath << "\"\n";
    return false;
  }

	// (Verify Mode) Create the DiagnosticVerifier
	std::unique_ptr<DiagnosticVerifier> dv;
	if (isVerifyModeEnabled()) {
		dv = std::make_unique<DiagnosticVerifier>(diags, srcMgr);
    // Parse the file
		dv->parseFile(fid);
    // Enable the verify mode in the diagnostic engine
    diags.enableVerifyMode(dv.get());
	}

	// Do lexing
  Lexer lex(ctxt);
  {
    auto chrono = createChrono("Lexing");
    lex.lexFile(fid);
  }

  // Stop if we had errors
  if (diags.getErrorsCount())
    return false;

  Parser psr(ctxt, lex.getTokenVector());

  UnitDecl* unit;
  // Do parsing
  {
    auto chrono = createChrono("Parsing");
    unit = psr.parseUnit(fid, ctxt.getIdentifier("TestUnit"));
  }

  auto canContinue = [&](){
    return (unit != nullptr) && !ctxt.hadErrors();
  };

  // Dump alloc if needed
  if (getDumpAlloc()) {
    getOS() << "\nDumping allocator:\n";
    ctxt.getAllocator().dump();
  }

  // Semantic analysis
	if(canContinue()) {
    Sema s(ctxt);
    s.checkDecl(unit);
  }

  // Dump AST if needed, and if the unit isn't null
  if (unit && getDumpAST()) {
    auto chrono = createChrono("AST Printing");
    getOS() << "\nAST Printing:\n";
    ASTDumper(srcMgr, getOS(), 1).print(unit);
  }

  // (Verify mode) Check that all diags were emitted
  if (verify_) {
    assert(dv && "DiagnosticVerifier is null");
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

bool Driver::isVerifyModeEnabled() const {
  return verify_;
}

void Driver::setVerifyModeEnabled(bool val) {
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
      setVerifyModeEnabled(true);
    else if (str == "-werr")
      diags.setWarningsAreErrors(true);
    else if (str == "-dump-ast")
      setDumpAST(true);
    else {
      getOS() << "Unknown argument \"" << str << "\"\n";
      return false;
    }
  }
 
  return processFile(filepath);
}
