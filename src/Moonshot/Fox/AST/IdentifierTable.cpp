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
	// Effective STL, Item 24 by Scott Meyers : https://stackoverflow.com/a/101980
	auto it = table_.lower_bound(id);
	if (it != table_.end() && !(table_.key_comp()(id,it->first)))
	{
		// Identifier already exists in table_, return ->second after some checks.

		assert(it->second.mapIter_.it_ != table_.end() && "IdentifierInfo iterator was invalid");
		assert(it->second.mapIter_.it_ == it && "Iterator was not correct!");
		return it->second;
	}
	else
	{
		// Key does not exists, insert.

		auto newIt = table_.insert(it, std::make_pair(id, IdentifierInfo(table_.end())));
		// Important : Set iterator
		newIt->second.mapIter_.it_ = newIt;

		return newIt->second;
	}
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
