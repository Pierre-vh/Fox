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

// Some includes are only needed when assertions are enabled.
#ifndef NDEBUG
  #include "Fox/AST/Types.hpp"
#endif

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

int Driver::processFile(string_view path) {
  // Cleanup the file path
  path = cleanupPath(path);

  // Load the file in the source manager.
  FileID file = tryLoadFile(path);
  if(!file) return false; // Stop if it can't be loaded.

  bool isVerifyMode = options.verifyMode;

	// Create the DiagnosticVerifier if we're in verify mode.
  std::unique_ptr<DiagnosticVerifier> dv;
  if (isVerifyMode) {
    dv = std::make_unique<DiagnosticVerifier>(diagEngine, sourceMgr);
    // Parse the file
    dv->parseFile(file);
    // Enable the verify mode in the DiagnosticEngine
    diagEngine.enableVerifyMode(dv.get());
  }

  ASTContext ctxt(sourceMgr, diagEngine, file);

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

  /// Helper function to finish the processing of the file.
  /// \p result is what the function should return when not in verify mode.
  auto finish = [&](int result) {
    // (Verify mode) Finish Verification
    if (isVerifyMode) {
      assert(dv && "DiagnosticVerifier is null");
      // In verify mode, override the result with the DV's.
      result = dv->finish() ? EXIT_SUCCESS : EXIT_FAILURE;
    }
    // Reset the ASTContext
    {
      auto timer = createTimer(*this, "ASTContext::reset()");
      ctxt.reset();
    }
    return result;
  };

  // Dump AST if needed, and if the unit isn't null
  if (unit && options.dumpAST) {
    auto timer = createTimer(*this, "ASTDumper");
    out << "AST Dump:\n";
    ASTDumper(sourceMgr, out, 1).print(unit);
  }

  if(!canContinue())
    return finish(diagEngine.hadAnyError());

  // Generate the bytecode if needed
  if(!needsToGenerateBytecode())
    return finish(diagEngine.hadAnyError());

  BCModule theModule(sourceMgr);
  BCGen generator(ctxt, theModule);
  generator.genUnit(unit);

  // Dump the bytecode if needed
  if (options.dumpBCGen)
    theModule.dump(out);

  // Run the bytecode if needed
  if (options.run) {
    int code = run(ctxt, file, theModule);
    if(options.verbose)
      out << "program exited with code " << code << '\n';
    return finish(code);
  }

  // If we reach that point, it's safe to assume that everything
  // went well, or we'd have bailed early.
  return finish(EXIT_SUCCESS);
}

int Driver::main(int argc, char* argv[]) {
  // Must have 2 args, first is executable path, second should
  // be filepath => argc must be >= 2
  if (argc < 2) {
    out << "Not enough args\n";
    return false;
  }

  // Get file path
  string_view filepath = argv[1];
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
    else if(str == "-v" || str == "-verbose")
      options.verbose = true;
    else {
      diagEngine.report(DiagID::unknown_argument, SourceLoc()).addArg(str);
      return false;
    }
  }
 
  return processFile(filepath);
}

/// \returns an error message for a ReadFileResult.
static const char* getMessage(SourceManager::ReadFileResult result) {  
  using RFR = SourceManager::ReadFileResult;
  switch(result) {
    case RFR::Ok:               fox_unreachable("not an error");
    case RFR::NotFound:         return "file not found";
    case RFR::InvalidEncoding:  return "unsupported file encoding";
    default:
      fox_unreachable("unknown ReadFileResult");
  }
}

FileID Driver::tryLoadFile(string_view path) {
  auto result = sourceMgr.readFile(path);
  FileID file = result.first;
  if (!file)
    diagEngine.report(DiagID::couldnt_open_file, SourceLoc())
      .addArg(path).addArg(getMessage(result.second));
  return file;
}

bool Driver::needsToGenerateBytecode() const {
  return options.run || options.dumpBCGen;
}

int Driver::run(ASTContext& ctxt, FileID mainFile, BCModule& theModule) {
  BCFunction* entryPoint = theModule.getEntryPoint();
  // Diagnose if there is no entry point
  if (!entryPoint) {
    diagEngine.report(DiagID::no_entry_point_found, mainFile)
      .addArg(ctxt.getEntryPointIdentifier())
      .addArg(ctxt.getEntryPointType());
    return EXIT_FAILURE;
  }
#ifndef NDEBUG
  // Check that our assumptions about the entry point's type are correct.
  FunctionType* entryType = ctxt.getEntryPointType()->getAs<FunctionType>();
  assert(entryType && entryType->getReturnType()->isIntType() 
    && "Entry Point's type is not () -> int");
#endif
  VM::Register reg = VM(theModule).run(*entryPoint);
  FoxInt rtr = reg.intVal;
  // Clamp it to int's min/max.
  constexpr int iMax = std::numeric_limits<int>::max();
  constexpr int iMin = std::numeric_limits<int>::max();
  if(rtr < 0) 
    return (rtr < iMin) ? iMin : rtr;
  return (rtr > iMax) ? iMax : rtr;
}