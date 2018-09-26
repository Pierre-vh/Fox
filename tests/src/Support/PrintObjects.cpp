////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : PrintObjects.cpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
//			SEE HEADER FILE FOR MORE INFORMATION			
////------------------------------------------------------////

#include "PrintObjects.hpp"

#include "Fox/Common/Source.hpp"

using namespace fox;

std::ostream& fox::operator<<(std::ostream& os, FileID fid)
{
	os << "FileID(" << fid.getRaw() << ")";
	return os;
}

std::ostream& fox::operator<<(std::ostream& os, SourceLoc loc)
{
	os << "SourceLoc(" << loc.getFileID() << ", " << loc.getIndex() << ")";
	return os;
}

std::ostream& fox::operator<<(std::ostream& os, SourceRange range)
{
	os << "SourceRange(" << range.getBegin() << ", " << range.getEnd() << ")";
	return os;
}

std::ostream& fox::operator<<(std::ostream& os, const CompleteLoc& loc)
{
	os << "CompleteLoc(" << loc.fileName << ", " << loc.line << ", " << loc.column << ")";
	return os;
}
