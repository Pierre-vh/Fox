////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : IdentifierTable.cpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
//			SEE HEADER FILE FOR MORE INFORMATION			
////------------------------------------------------------////

#include "IdentifierTable.hpp"

#include <cassert>

using namespace Moonshot;

StringPtrInMap::StringPtrInMap(ItTy iter) : it_(iter)
{
}

std::string StringPtrInMap::get() const
{
	return it_->first;
}

IdentifierInfo::IdentifierInfo(StringPtrInMap::ItTy iter): mapIter_(iter)
{

}

std::string IdentifierInfo::getStr() const
{
	return mapIter_.get();
}

bool IdentifierInfo::operator<(const IdentifierInfo& id) const
{
	return getStr() < id.getStr();
}

bool IdentifierInfo::operator<(const std::string& idstr) const
{
	return getStr() < idstr;
}

IdentifierInfo & IdentifierTable::getUniqueIDinfo(const std::string & id)
{
	auto it = table_.find(id);
	if (it != table_.end())
	{
		assert(it->second.mapIter_.it_ != table_.end() && "IdentifierInfo iterator was invalid");
		return it->second;
	}
	else
		return insert(id);
}

bool IdentifierTable::exists(const std::string & id) const
{
	// Return false if there is no identifier in the table
	if (table_.size())
	{
		auto it = table_.find(id);
		return (it != table_.end());
	}
	return false;
}

IdentifierTable::IDTableConstIterator IdentifierTable::begin() const
{
	return table_.begin();
}

IdentifierTable::IDTableIterator IdentifierTable::begin()
{
	return table_.begin();
}

IdentifierTable::IDTableConstIterator IdentifierTable::end() const
{
	return table_.end();
}

IdentifierTable::IDTableIterator IdentifierTable::end()
{
	return table_.end();
}

IdentifierInfo & IdentifierTable::insert(const std::string& str)
{
	auto entry = table_.insert(std::make_pair(str, IdentifierInfo(table_.end())));
	auto it = entry.first;

	// Set the IdentifierInfo's iterator to the correct iterator.
	it->second.mapIter_ = it;

	// A little sanity check, just to be sure :)
	assert((it->second.mapIter_.it_ != table_.end()) && "The IdentifierInfo's iterator is invalid!");
	
	// Return the new entry.
	return it->second;
}
