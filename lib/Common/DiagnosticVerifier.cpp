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
  auto range = expectedDiags_.equal_range(diag.getStr());
  for (auto it = range.first; it != range.second; ++it) {
    // Found one, but check if the file & line match.
    std::pair<FileID, LineTy> pair = it->second;
    SourceLoc loc = diag.getRange().getBegin();

    // Check file match
    if (pair.first != loc.getFileID())
      continue;
    // Okay, file matches, now check the line.
    // This is the most expensive operation here so we do it last,
    // when we're sure that the string & file match.
    auto line = srcMgr_.getLineNumber(loc);
    if (line != pair.second)
      continue;

    // Diagnostic was expected, ignore it, remove the entry from the map
    // and return.
    expectedDiags_.erase(it);
    diag.ignore();
    return;
  }
}

void DiagnosticVerifier::addExpectedDiag(FileID file, LineTy line, 
  string_view str) {
  expectedDiags_.insert({str, {file, line}});
}

bool DiagnosticVerifier::handleVerifyInstr(SourceLoc loc, string_view instr) {
	parseVerifyInstr(loc, instr);
  return true;
}

ResultObject<DiagnosticVerifier::VerifyInstr> 
DiagnosticVerifier::parseVerifyInstr(SourceLoc loc, string_view instr) {
	using RtrTy = ResultObject<VerifyInstr>;
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
	std::cout << "Split(" << splitted.first << "," << splitted.second << ")\n";
	
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

	std::cout << "Post-checks(" << fullSuffix << ")(" << str << ")\n";

	return RtrTy(false);
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
