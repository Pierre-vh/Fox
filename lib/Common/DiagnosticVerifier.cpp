//----------------------------------------------------------------------------//
// This file is a part of The Moonshot Project.        
// See LICENSE.txt for license info.            
// File : DiagnosticVerifier.cpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//

#include "Fox/Common/DiagnosticVerifier.hpp"
#include "Fox/Common/DiagnosticEngine.hpp"
#include "Fox/Common/Diagnostic.hpp"
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
    return SourceLoc(loc.getFileID(), loc.getIndex() + off);
  }

  // Removes characters such as \r, \n from the end of the string.
  void removeNewline(string_view& str) {
    std::size_t end = str.size();
    for (auto it = str.rbegin(); it != str.rend(); ++it) {
      char c = (*it);
      if ((c == '\r') || (c == '\n'))
        --end;
      else break;
    }

    // See if trimming is necessary, if it is, do it.
    if(end != str.size())
      str = str.substr(0, end);
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

    removeNewline(rtr);
    return rtr;
  }

	// Trims a string, removing spaces, tabs and others to the left and 
	// right of the string.
	// str = The string that'll be trimmed
	void trim(string_view &str) {
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

struct DiagnosticVerifier::ParsedInstr {
	ParsedInstr() = default;

	ParsedInstr(string_view suffix, string_view arg, string_view str) :
		suffix(suffix), arg(arg), str(str) {}

	string_view suffix;
	string_view arg;
	string_view str;
};

struct DiagnosticVerifier::ExpectedDiag {
	ExpectedDiag(DiagSeverity sev, string_view str, FileID file, LineTy line):
		severity(sev), str(str), file(file), line(line) {}

	DiagSeverity severity;
	string_view str;
	FileID file;
	LineTy line;

	// For Multiset
	bool operator<(const ExpectedDiag& other) {
		return std::tie(severity, str, file, line) 
			< std::tie(other.severity, other.str, other.file, other.line);
	}
};

DiagnosticVerifier::DiagnosticVerifier(DiagnosticEngine& engine, 
																			 SourceManager& srcMgr): 
	diags_(engine), srcMgr_(srcMgr) {
  
}

bool DiagnosticVerifier::parseFile(FileID fid) {
  // Fetch the content of the file
  string_view fStr = srcMgr_.getSourceStr(fid);
  bool rtr = true;
  {
    std::size_t last = 0, idx = 0;
    do {
      idx = fStr.find(vPrefix, last);
      if (idx == string_view::npos)
        break;
      last = idx + 1;
      auto instr = getRestOfLine(idx, fStr);
      std::cout << "Full instr found(" << instr << ")\n";
      rtr |= handleVerifyInstr(SourceLoc(fid, idx), instr);
    } while (true);
  }
  return false;
}

void DiagnosticVerifier::consume(Diagnostic& diag) {
  // Check if there is an entry for this string in our map

	// FIXME: This isn't ideal, might have poor performance.
	// Construct an ExpectedDiag to search the map
	SourceLoc diagLoc = diag.getRange().getBegin();
	ExpectedDiag ed(diag.getSeverity(),
									diag.getStr(),
									diagLoc.getFileID(), 
									srcMgr_.getLineNumber(diagLoc));
  auto it = expectedDiags_.find(ed);
  if(it != expectedDiags_.end()) {
    // We expected this diag, erase the entry from the map and ignore
		// the diag.
    expectedDiags_.erase(it);
    diag.ignore();
  }
}

bool DiagnosticVerifier::handleVerifyInstr(SourceLoc loc, string_view instr) {
	auto parsingResult = parseVerifyInstr(loc, instr);

	// Parsing failed? We can't do much more!
	if (!parsingResult.wasSuccessful()) return false;
	auto parsedInstr = parsingResult.get();
	
  return true;
}

ResultObject<DiagnosticVerifier::ParsedInstr>
DiagnosticVerifier::parseVerifyInstr(SourceLoc loc, string_view instr) {
	using RtrTy = ResultObject<ParsedInstr>;
	std::size_t fullInstrSize = instr.size();
	// Remove the prefix
	instr = instr.substr(vPrefixSize, instr.size() - vPrefixSize);
	// Find the ':'
	auto colonPos = instr.find(':');
	if (colonPos == string_view::npos) {
		diagnoseMissingColon(offsetSourceLoc(loc, fullInstrSize));
		return RtrTy(false);
	}
	
	// With that, we can split the instr in 2, the base and the string.
	// The base is the suffix, maybe with some arguments, and the string
	// is the actual expected diagnostic string.
	auto splitted = split(instr, colonPos);
	
	string_view fullSuffix = splitted.first;
	string_view str = splitted.second;

	// Check if we have a prefix. If we don't, that's an error.
	if (!fullSuffix.size()) {
		diagnoseMissingSuffix(loc);
		return RtrTy(false);
	}

	// Trim the string
	trim(str);

	// Check if we have a diag str, If we don't, that's an error.
	if (!str.size()) {
		// Because we removed the prefix earlier, we'll
		// add the vPrefixSize to colonPos to calculate it's real position
		auto realColonPos = colonPos + vPrefixSize;
		// We increment realColonPos because we want the diagnostic to be
		// just after the colon
		diagnoseMissingStr(offsetSourceLoc(loc, realColonPos+1));
		return RtrTy(false);
	}

	string_view suffix;
	string_view arg;
	// Now, check if we have arguments in the suffix
	auto atLoc = fullSuffix.find(vArgSep);
	if (atLoc != string_view::npos) {
		// We found it, split the string in 2.
		std::tie(suffix, arg) = split(fullSuffix, atLoc);
	} else {
		// If we don't have one, that means our suffix doesn't have any arg.
		suffix = fullSuffix;
	}
	std::cout << "Done, returning:(" 
		<< suffix << ")(" << arg << ")(" << str << ")\n";
	return RtrTy(true, ParsedInstr(suffix, arg, str));
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

bool 
DiagnosticVerifier::parseSuffix(string_view suffix, ExpectedDiag& expected) {
	// Parse using vWarnPrefix and the likes.
	// Put the result inside expected.severity
}

bool 
DiagnosticVerifier::parseArg(const ParsedInstr & instr, ExpectedDiag& expected) {
	// First char must be + or -, second must be digit.
	// Put the result inside expected.offset
}
