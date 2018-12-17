//----------------------------------------------------------------------------//
// This file is a part of The Moonshot Project.        
// See LICENSE.txt for license info.            
// File : DiagnosticVerifier.cpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//

#include "Fox/Common/DiagnosticVerifier.hpp"
#include "Fox/Common/DiagnosticEngine.hpp"
#include "Fox/Common/ResultObject.hpp"
#include <tuple>
#include <cctype>

using namespace fox;

//----------------------------------------------------------------------------//
//  DiagnosticVerifier's file parsing implementation/helpers
//----------------------------------------------------------------------------//

namespace {
  // Prefix used for verification
  constexpr char vPrefix[] = "expect-";
  constexpr std::size_t vPrefixSize = sizeof(vPrefix)-1;

	// The separator used to add arguments after the suffix.
	constexpr char vArgSep = '@';

	// Suffix used by each severity
	constexpr char vErrorSuffix[] = "error";
	constexpr char vWarnSuffix[] = "warning";
	constexpr char vNoteSuffix[] = "note";
	constexpr char vFatalSuffix[] = "fatal";

  // Offsets a sourceloc by X chars. This doesn't check if the SourceLoc
  // is valid, it justs adds the offset to the index.
  SourceLoc offsetSourceLoc(SourceLoc loc, std::size_t off) {
    return SourceLoc(loc.getFile(), loc.getIndex() + off);
  }

  // Trims a string, removing spaces, tabs and others to the left and 
  // right of the string.
  // str = The string that'll be trimmed
  void trim(string_view &str) {
    if (!str.size()) return;

    std::size_t beg = 0;
    std::size_t end = str.size() - 1;
    // Trim to the left
    while (std::isspace(str[beg]) && beg != end) ++beg;

    // If the string fully consists of spaces beg will be equal to end, then
    // instead of searching again, just set it to "" as the trimmed string
    // is considered empty.
    if (beg == end) {
      str = "";
      return;
    }

    // Trim to the right
    while (std::isspace(str[end])) --end;
    str = str.substr(beg, end - beg + 1);
  }

  // Returns the whole string between pos and the end of the line/eof
  string_view getRestOfLine(std::size_t pos, string_view str) {
    auto end = str.find('\n', pos);
    string_view rtr;
    // If we found a \n, all good.
    if(end != string_view::npos)
      rtr = str.substr(pos, end-pos);
    // If we didn't, take everything until the end of the file.
    else
      rtr = str.substr(pos, str.size()-pos);

    trim(rtr);
    return rtr;
  }

	// Splits a string view around a character. 
	// e.g. split("foo:bar", 3) returns {"foo", "bar"
	std::pair<string_view, string_view> split(string_view str, std::size_t pos) {
		string_view first = str.substr(0, pos);
		string_view second = str.substr(pos + 1, str.size() - pos - 1);
		return { first, second };
	}

} // anonymous namespace

//----------------------------------------------------------------------------//
//  DiagnosticVerifier's methods implementation
//----------------------------------------------------------------------------//

DiagnosticVerifier::DiagnosticVerifier(
  DiagnosticEngine& engine, SourceManager& srcMgr):
	diags_(engine), srcMgr_(srcMgr) {}

bool DiagnosticVerifier::parseFile(FileID fid) {
  // Fetch the content of the file
  string_view fStr = srcMgr_.getSourceStr(fid);
  bool rtr = true;
  {
    std::size_t last = 0, idx = 0;
    while (last<fStr.size()) {
      idx = fStr.find(vPrefix, last);
      if (idx == string_view::npos)
        break;
      last = idx + 1;
      auto instr = getRestOfLine(idx, fStr);
      //std::cout << "Verify Instr found(" << instr << ")\n";
      rtr &= handleVerifyInstr(SourceLoc(fid, idx), instr);
    }
  }
  return rtr;
}

DiagnosticVerifier::DiagsSetTy&
DiagnosticVerifier::getExpectedDiags() {
  return expectedDiags_;
}

bool DiagnosticVerifier::reportUnemittedDiags() {
  if (expectedDiags_.size()) {
    // First, emit an error of the form "X expected diags weren't emitted"
    diags_.report(DiagID::diagverif_errorExpectedDiagsNotEmitted)
      .addArg(expectedDiags_.size());
    // Then, emit a note for each diag
    for (auto& diag : expectedDiags_) {
      diags_.report(DiagID::diagverif_diagNotEmitted, diag.file)
        .addArg(diag.diagStr)
        .addArg(toString(diag.severity))
        .addArg(diag.line);
    }
    return true;
  }
  return false;
}

bool DiagnosticVerifier::verify(Diagnostic& diag) {
  {
    DiagID id = diag.getID();
    // Always emit our own diagnostics.
    if((id >= DiagID::First_DiagnosticVerifier) 
      && (id <= DiagID::Last_DiagnosticVerifier))
      return true;
  }

	// Construct an ExpectedDiag to search the map
	SourceLoc diagLoc = diag.getRange().getBegin();
  // Save the string in a local variable, because if we don't and we try
  // to call diag.getStr() in the ExpectedDiag ctor, the call to diag.getStr() 
  // will generate a std::string temporary object. This temporary will be 
  // converted to string_view and then die, creating a corrupted string_view
  // inside the ExpectedDiag.
  std::string diagStr = diag.getStr();
	ExpectedDiag ed(diag.getSeverity(),
                  diagStr,
									diagLoc.getFile(), 
									srcMgr_.getLineNumber(diagLoc));

  auto it = expectedDiags_.find(ed);
  if(it != expectedDiags_.end()) {
    // We expected this diag, erase the entry from the map and ignore
		// the diag.
    expectedDiags_.erase(it);
    return false;
  }
  return true;
}

bool DiagnosticVerifier::handleVerifyInstr(SourceLoc loc, string_view instr) {
	auto parsingResult = parseVerifyInstr(loc, instr);

	// Parsing failed? We can't do much more!
	if (!parsingResult.wasSuccessful()) return false;
	auto diag = parsingResult.get();
	

  // Offset stuff
	expectedDiags_.insert(diag);
  return true;
}

ResultObject<DiagnosticVerifier::ExpectedDiag>
DiagnosticVerifier::parseVerifyInstr(SourceLoc loc, string_view instr) {
	using RtrTy = ResultObject<ExpectedDiag>;
  assert(loc && "invalid loc");
  assert(instr.size() && "empty instr");
  // The values we'll collect
  DiagSeverity severity;
  string_view diagStr;
  FileID file = loc.getFile();
  LineTy line = srcMgr_.getLineNumber(loc);

	std::size_t fullInstrSize = instr.size();
	// Remove the prefix
	instr = instr.substr(vPrefixSize, instr.size() - vPrefixSize);
	// Find the ':'
  std::size_t colonPos = instr.find(':');
  if (colonPos == string_view::npos) {
    diagnoseMissingColon(offsetSourceLoc(loc, fullInstrSize));
    return RtrTy(false);
  }

	// With that, we can split the instr in 2, the base and the diagStr.
	// The base is the suffix, maybe with some arguments, and the diagStr
	// is the actual expected diagnostic string.
  string_view base;
	std::tie(base, diagStr) = split(instr, colonPos);
	
	// Check if we have a suffix. If we don't, that's an error.
	if (!base.size()) {
		diagnoseMissingSuffix(loc);
		return RtrTy(false);
	}

	// Trim the diagStr to remove end of line characters and
  // others.
	trim(diagStr);

	// Check if we have a diag str, If we don't, that's an error.
	if (!diagStr.size()) {
		// We increment colonPos because we want the diagnostic to be
		// just after the colon
		diagnoseMissingStr(offsetSourceLoc(loc, vPrefixSize+colonPos+1));
		return RtrTy(false);
	}

  // Suffix parsing : 2 cases
  //    Simple suffix: just "error" or "warn"
  //    Suffix with offset: "error@+1" "warn@-9"
  // -> We can dispatch based on the presence of the vArgSep or not
  {
    string_view sevStr;
    auto sepLoc = base.find(vArgSep);

    // It has arguments
    if (sepLoc != string_view::npos) {
      std::int8_t offset = 0;
      string_view offsetStr;
      // Split the string to get the offset string and the severity string.
      std::tie(sevStr, offsetStr) = split(base, sepLoc);

      // The range of the offset string is calculated based on the sepLoc+1.
      // We also add the vPrefixSize because we removed it earlier
      SourceLoc beg = offsetSourceLoc(loc, vPrefixSize+sepLoc+1);

      // The range's size is the offsetStr's size-1
      SourceRange argRange(beg, offsetStr.size()-1);

      // Parse the offset
      if (!parseOffset(argRange, offsetStr, offset))
        return RtrTy(false);

      // Apply it.
      line += offset;
    }
    else // It's a simple suffix, the suffix is equal to the base.
      sevStr = base;
 
    // Now parse the severity string
    if (!parseSeverity(sevStr, severity))
      return RtrTy(false);
  }

	//std::cout << "Done, returning:(" 
	//	<< severity << ")(" << line << ")(" << diagStr << ")\n";
	return RtrTy(true, ExpectedDiag(severity, diagStr, file, line));
}

void DiagnosticVerifier::diagnoseZeroOffset(SourceLoc offsetDigitLoc) {
  diags_.report(DiagID::diagverif_offsetIsZero, offsetDigitLoc);
}

void DiagnosticVerifier::diagnoseMissingStr(SourceLoc loc) {
	diags_.report(DiagID::diagverif_expectedstr, loc);
}

void DiagnosticVerifier::diagnoseMissingColon(SourceLoc loc) {
	diags_.report(DiagID::diagverif_expectedcolon, loc);
}

void DiagnosticVerifier::diagnoseMissingSuffix(SourceLoc instrBeg) {
  diags_.report(DiagID::diagverif_expectedsuffix,
    offsetSourceLoc(instrBeg, vPrefixSize))
      .addArg(vPrefix)
      .setExtraRange(SourceRange(instrBeg, vPrefixSize - 1));
}

void DiagnosticVerifier::diagnoseIllFormedOffset(SourceRange range) {
	diags_.report(DiagID::diagverif_illFormedOffset, range);
}

bool 
DiagnosticVerifier::parseSeverity(string_view suffix, DiagSeverity& sev) {
	if (suffix == vFatalSuffix)
		sev = DiagSeverity::FATAL;
	else if (suffix == vErrorSuffix)
		sev = DiagSeverity::ERROR;
	else if (suffix == vWarnSuffix)
		sev = DiagSeverity::WARNING;
	else if (suffix == vNoteSuffix)
		sev = DiagSeverity::NOTE;
	else return false;
	return true;
}

bool 
DiagnosticVerifier::parseOffset(SourceRange strRange, string_view str,
																std::int8_t& offset) {
	if (str.size() != 2) {
		diagnoseIllFormedOffset(strRange);
		return false;
	}
	
	// Get the digit
	std::int8_t digit = 0;
	if (std::isdigit(str[1]))
		digit = str[1] - '0';
	else {
		diagnoseIllFormedOffset(strRange);
		return false;
	}

  // Digit must be between 1 and 9, so it can't be 0.
  if (digit == 0) {
    // The loc of the offset is the end of the range
    diagnoseZeroOffset(strRange.getEnd());
    return false;
  }

	// Act on the sign
	char sign = str[0];
	if (sign == '+') {
		offset = digit;
		return true;
	} 
  else if (sign == '-') {
		offset = -digit;
		return true;
	} 
  else {
		diagnoseIllFormedOffset(strRange);
		return false;
	}
}
