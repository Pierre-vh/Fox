//----------------------------------------------------------------------------//
// Part of the Fox project, licensed under the MIT license.
// See LICENSE.txt in the project root for license information.      
// File : Driver.cpp                      
// Author : Pierre van Houtryve                      
//----------------------------------------------------------------------------//

#include "Fox/Driver/Driver.hpp"

#include "Fox/AST/ASTDumper.hpp"
#include "Fox/AST/ASTContext.hpp"
#include "Fox/AST/Decl.hpp"
#include "Fox/BC/BCBuilder.hpp"
#include "Fox/BC/Instruction.hpp"
#include "Fox/BC/BCModule.hpp"
#include "Fox/BCGen/BCGen.hpp"
#include "Fox/Common/LLVM.hpp"
#include "Fox/Common/DiagnosticVerifier.hpp"
#include "Fox/Lexer/Lexer.hpp"
#include "Fox/Parser/Parser.hpp"
#include "Fox/Sema/Sema.hpp"
#include "Fox/VM/VM.hpp"

#include <fstream>


using namespace fox;

Driver::Driver(std::ostream& os): out(os), diagEngine_(srcMgr_, os),
  ctxt_(srcMgr_, diagEngine_) {}

bool Driver::processFile(string_view filepath) {
  auto finish = [&]() {
    auto chrono = createChrono("ASTContext::reset()");
    ctxt_.reset();
    return !diagEngine_.hadAnyError(); 
  };

  // Remove quotes if there's quotes around the file
  if ((filepath.front() == '"') && (filepath.back() == '"'))
    filepath = filepath.substr(1, filepath.size()-2);

  // Load the file in the source manager
  auto result = srcMgr_.readFile(filepath);
  FileID file = result.first;
  if (!file) {
    out << "Could not open file \"" << filepath << "\"\n"
      "\tReason:" << toString(result.second) << '\n';
    return false;
  }

	// (Verify Mode) Create the DiagnosticVerifier
  std::unique_ptr<DiagnosticVerifier> dv;
  if (isVerifyModeEnabled()) {
    dv = std::make_unique<DiagnosticVerifier>(diagEngine_, srcMgr_);
    // Parse the file
    dv->parseFile(file);
    // Enable the verify mode in the diagnostic engine
    diagEngine_.enableVerifyMode(dv.get());
  }

  // The parsed unit
  UnitDecl* unit = nullptr;

  {
    // Do lexing
    Lexer lex(srcMgr_, diagEngine_, file);
    {
      auto chrono = createChrono("Lexing");
      lex.lex();
    }

    // Dump tokens if needed
    if (getDumpTokens()) {
      auto& toks = lex.getTokens();
      out << toks.size() << " tokens found in '" << srcMgr_.getFileName(file) << "'\n";
      for (Token& tok : toks) {
        out << "    ";
        tok.dump(out, srcMgr_, /*printFileName*/ false);
      }
    }

    // Parse the file if we did not have any error
    if(!diagEngine_.hadAnyError()) {
      auto chrono = createChrono("Parsing");
      unit = Parser(ctxt_, lex).parseUnit(ctxt_.getIdentifier("TestUnit"));
    }
  }

  auto canContinue = [&](){
    return (unit != nullptr) && !diagEngine_.hadAnyError();
  };

  // Dump alloc if needed
  if (getDumpAlloc()) {
    out << "Allocator Dump:\n";
    ctxt_.dumpAllocator();
  }
 
  // Semantic analysis
  if(canContinue() && !isParseOnly()) {
    Sema s(ctxt_);
    s.checkUnitDecl(unit);
  }

  // Dump AST if needed, and if the unit isn't null
  if (unit && getDumpAST()) {
    auto chrono = createChrono("ASTDumper");
    out << "AST Dump:\n";
    ASTDumper(srcMgr_, out, 1).print(unit);
  }


  // (Verify mode) Finish Verification
  if (verify_) {
    assert(dv && "DiagnosticVerifier is null");
    // Return directly after finishing. We won't do BCGen or execute
    // anything in verify mode
    return finish(), dv->finish();
  }

  if(!canContinue())
    return finish();

  if(!(willRun() || getDumpBCGen()))
    return finish();

  BCModule theModule;
  BCGen generator(ctxt_, theModule);
  generator.genUnit(unit);

  if (getDumpBCGen())
    theModule.dump(out);
  if (willRun()) {
    VM vm(theModule);
    // Use '!' because we want to return EXIT_SUCCESS when 
    // the function returns nothing (0)
    return !vm.call(theModule.getFunction(0)).raw;
  }

  return true;
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

bool Driver::getDumpBCGen() const {
  return dumpBCGen_;
}

void Driver::setDumpBCGen(bool val) {
  dumpBCGen_ = val;
}

bool Driver::getDumpAST() const {
  return dumpAST_;
}

void Driver::setDumpAST(bool val) {
  dumpAST_ = val;
}

bool Driver::getDumpTokens() const {
  return dumpTokens_;
}

void Driver::setDumpTokens(bool val) {
  dumpTokens_ = val;
}

bool Driver::willRun() const {
  return run_;
}

void Driver::setRun(bool val) {
  run_ = val;
}

bool Driver::isParseOnly() const {
  return parseOnly_;
}

void Driver::setIsParseOnly(bool val) {
  parseOnly_ = val;
}

Driver::RAIIChrono Driver::createChrono(string_view label) {
  return RAIIChrono(*this, label);
}

bool Driver::doCL(int argc, char* argv[]) {
  // Must have 2 args, first is executable path, second should
  // be filepath => argc must be >= 2
  if (argc < 2) {
    out << "Not enough args\n";
    return false;
  }

  // Get file path
  std::string filepath = argv[1];
  if((filepath.front() == '"') && (filepath.back() == '"'))
    filepath = filepath.substr(1, filepath.size()-2);

  for(int idx = 2; idx < argc; ++idx) {
    string_view str(argv[idx]);
    if (str == "-verify")
      setVerifyModeEnabled(true);
    else if (str == "-werr")
      diagEngine_.setWarningsAreErrors(true);
    else if (str == "-dump-ast")
      setDumpAST(true);
    else if(str == "-parse-only")
      setIsParseOnly(true);
    else if(str == "-dump-bcgen") 
      setDumpBCGen(true);
    else if(str == "-dump-tokens") 
      setDumpTokens(true);
    else if(str == "-run") 
      setRun(true);
    else {
      // TODO: Emit a diagnostic for this.
      out << "Unknown argument \"" << str << "\"\n";
      return false;
    }
  }
 
  return processFile(filepath);
}
