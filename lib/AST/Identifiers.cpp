//----------------------------------------------------------------------------//
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : Identifiers.cpp											
// Author : Pierre van Houtryve								
//----------------------------------------------------------------------------//

#include "Fox/AST/Identifiers.hpp"
#include <cassert>

// Normally this identifier shouldn't be possible
// in the language because <> are both illegal in an identifier
#define INVALID_ID_STR "<invalid>"

using namespace fox;

StringPtrInMap::StringPtrInMap(ItTy iter) : it_(iter)
{
}

const std::string& StringPtrInMap::get() const
{
	return (it_->first);
}

Identifier::Identifier(const StringPtrInMap::ItTy& iter): mapIter_(iter)
{

}

const std::string& Identifier::getStr() const
{
	return mapIter_.get();
}

bool Identifier::operator<(const Identifier& id) const
{
	return getStr() < id.getStr();
}

bool Identifier::operator<(const std::string& idstr) const
{
	return getStr() < idstr;
}

bool Identifier::operator==(const Identifier& id) const
{
	return getStr() == id.getStr();
}

bool Identifier::operator==(const std::string& str) const
{
	return getStr() == str;
}

bool Identifier::operator!=(const Identifier& id) const
{
	return !(*this == id);
}

bool Identifier::operator!=(const std::string& str) const
{
	return !(*this == str);
}

Identifier* IdentifierTable::getUniqueIdentifierInfo(const std::string& id)
{
	auto it = table_.lower_bound(id);
	if (it != table_.end() && !(table_.key_comp()(id, it->first)))
	{
		// Identifier already exists in table_, return ->second after some checks.

		assert(it->second.mapIter_.it_ != table_.end() && "Identifier iterator was invalid");
		assert(it->second.mapIter_.it_ == it && "String iterator in ->second is incorrect");
		return &(it->second);
	}
	else
	{
		// Key does not exists, insert.
		auto newIt = table_.insert(it, std::make_pair(id, Identifier(table_.end())));

		assert(newIt != table_.end() && "Fresh iterator was equal to .end() ?");

		// /!\ Important : Set iterator
		newIt->second.mapIter_.it_ = newIt;

		return &(newIt->second);
	}
}

Identifier* IdentifierTable::getInvalidID()
{
	if (!invalidID_)
		invalidID_ = getUniqueIdentifierInfo(INVALID_ID_STR);
	assert(invalidID_ && "invalidID_ cannot be null!");
	return invalidID_;
}

bool IdentifierTable::exists(const std::string& id) const
{
	// Return false if there is no identifier in the table
	if (table_.size())
	{
		auto it = table_.find(id);
		return (it != table_.end());
	}
	return false;
}

IdentifierTable::IDTableConstIteratorType IdentifierTable::begin() const
{
	return table_.begin();
}

IdentifierTable::IDTableIteratorType IdentifierTable::begin()
{
	return table_.begin();
}

IdentifierTable::IDTableConstIteratorType IdentifierTable::end() const
{
	return table_.end();
}

IdentifierTable::IDTableIteratorType IdentifierTable::end()
{
	return table_.end();
}