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
#include "llvm/ADT/Optional.h"
#include <chrono>
#include <fstream>


using namespace fox;

namespace {
  class RAIITimer {
    public:
      std::ostream& out;
      std::chrono::high_resolution_clock::time_point beg;
      string_view label;

      RAIITimer(std::ostream& out, string_view label) 
        : out(out), label(label) {
        beg = std::chrono::high_resolution_clock::now();
      }

      ~RAIITimer() {
        auto end = std::chrono::high_resolution_clock::now();
        auto micro = std::chrono::duration_cast
          <std::chrono::microseconds>(end-beg).count();
        auto milli = std::chrono::duration_cast
          <std::chrono::milliseconds>(end-beg).count();
        out 
          << label << " time:" 
          << micro << " microseconds | " 
          << milli << " milliseconds\n";
      }
  };

  Optional<RAIITimer> createTimer(Driver& driver, string_view label) {
    // Don't create the timer if we don't want to time the stages.
    if(!driver.options.timeStages)
      return None;
    return RAIITimer(driver.out, label);
  }

  string_view cleanupPath(string_view path) {
    if ((path.front() == '"') && (path.back() == '"'))
      return path.substr(1, path.size()-2);
    return path;
  }
}

Driver::Driver(std::ostream& out) : out(out), 
  diagEngine(sourceMgr, out) {}

bool Driver::processFile(string_view path) {
  // Cleanup the file path
  path = cleanupPath(path);

  // Load the file in the source manager.
  FileID file = tryLoadFile(path);
  if(!file) return false; // Stop if it can't be loaded.

	// Create the DiagnosticVerifier if we're in verify mode.
  std::unique_ptr<DiagnosticVerifier> dv;
  if (options.verifyMode) {
    dv = std::make_unique<DiagnosticVerifier>(diagEngine, sourceMgr);
    // Parse the file
    dv->parseFile(file);
    // Enable the verify mode in the DiagnosticEngine
    diagEngine.enableVerifyMode(dv.get());
  }

  ASTContext ctxt(sourceMgr, diagEngine);

  UnitDecl* unit = nullptr;
  {
    // Do lexing
    Lexer lex(sourceMgr, diagEngine, file);
    {
      auto timer = createTimer(*this, "Lexing");
      lex.lex();
    }

    // Dump tokens if needed
    if (options.dumpTokens) {
      auto& toks = lex.getTokens();
      out << toks.size() << " tokens found in '" 
          << sourceMgr.getFileName(file) << "'\n";
      for (Token& tok : toks) {
        out << "    ";
        tok.dump(out, sourceMgr, /*printFileName*/ false);
      }
    }

    // Parse the file if we did not have any error
    if(!diagEngine.hadAnyError()) {
      auto timer = createTimer(*this, "Parsing");
      // TODO: Give an actual name to the unit
      unit = Parser(ctxt, lex).parseUnit(ctxt.getIdentifier("TestUnit"));
    }
  }

  auto canContinue = [&](){
    return (unit != nullptr) && !diagEngine.hadAnyError();
  };

  // Dump alloc if needed
  if (options.dumpASTAllocator) {
    out << "Allocator Dump:\n";
    ctxt.dumpAllocator();
  }
 
  // Do semantic analysis unless the client doesn't want it.
  if(canContinue() && !options.parseOnly) {
    Sema s(ctxt);
    s.checkUnitDecl(unit);
  }

  // Helper
  auto finish = [&]() {
    auto timer = createTimer(*this, "ASTContext::reset()");
    ctxt.reset();
    return !diagEngine.hadAnyError(); 
  };


  // Dump AST if needed, and if the unit isn't null
  if (unit && options.dumpAST) {
    auto timer = createTimer(*this, "ASTDumper");
    out << "AST Dump:\n";
    ASTDumper(sourceMgr, out, 1).print(unit);
  }

  // (Verify mode) Finish Verification
  if (options.verifyMode) {
    assert(dv && "DiagnosticVerifier is null");
    // Return directly after finishing. We won't do BCGen or execute
    // anything in verify mode
    return finish(), dv->finish();
  }

  if(!canContinue())
    return finish();

  if(!needsToGenerateBytecode())
    return finish();

  BCModule theModule;
  BCGen generator(ctxt, theModule);
  generator.genUnit(unit);

  // Dump the bytecode if needed
  if (options.dumpBCGen)
    theModule.dump(out);

  // Run the bytecode if needed
  if (options.run) {
    VM(theModule).call(theModule.getFunction(0));
    return true;
  }

  return true;
}

int Driver::main(int argc, char* argv[]) {
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
      options.verifyMode = true;
    else if (str == "-werr")
      diagEngine.setWarningsAreErrors(true);
    else if (str == "-dump-ast")
      options.dumpAST = true;
    else if(str == "-parse-only")
      options.parseOnly = true;
    else if(str == "-dump-bcgen") 
      options.dumpBCGen = true;
    else if(str == "-dump-tokens") 
      options.dumpTokens = true;
    else if(str == "-run") 
      options.run = true;
    else {
      // TODO: Emit a diagnostic for this.
      out << "Unknown argument \"" << str << "\"\n";
      return false;
    }
  }
 
  return processFile(filepath) ? EXIT_SUCCESS : EXIT_FAILURE;
}

FileID Driver::tryLoadFile(string_view path) {
  auto result = sourceMgr.readFile(path);
  FileID file = result.first;
  if (!file) {
    // TO-DO: Emit a diagnostic instead
    out << "Could not open file \"" << path << "\"\n"
      "\tReason:" << toString(result.second) << '\n';
  }
  return file;
}

bool Driver::needsToGenerateBytecode() const {
  return options.run || options.dumpBCGen;
}
