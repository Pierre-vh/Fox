///------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : Identifiers.hpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
// This file contains the IdentifierTable, which is a class
// that manages a map of string to identifierinfo objects.
//
// Possible improvements :
//		In general, the IdentifierInfo that holds an interator to the string is a bit weird.
//		Either just store the string directly in the IdentifierInfo, or find a different solution.
//		The thing is the different solutions would require the use of const_cast or another trickery
//		(like mutable members)
//
////------------------------------------------------------////

#pragma once 

#include <map>
#include <string>

namespace fox
{
	class IdentifierInfo;

	// Wrapper around a const_iterator of a map entry, used to safely access the .first element (the string)
	class StringPtrInMap
	{
		public:
			typedef std::map<std::string, IdentifierInfo>::const_iterator ItTy;

			StringPtrInMap(ItTy iter);

			const std::string& get() const;

			ItTy it_;
	};

	// A Class holding informations related to a identifier.
	// Currently, it only stores the iterator to the std::string.
	class IdentifierInfo
	{
		public:
			IdentifierInfo(StringPtrInMap::ItTy iter);

			// Returns the string naming this identifier
			const std::string& getStr() const;

			// Comparison operators for use with STL containers.
			bool operator<(const IdentifierInfo& id) const;
			bool operator<(const std::string& idstr) const;

			// Other comparison operators
			bool operator==(const IdentifierInfo& id) const;
			bool operator==(const std::string& str) const;

			bool operator!=(const IdentifierInfo& id) const;
			bool operator!=(const std::string& str) const;
		private:
			friend class IdentifierTable;

			StringPtrInMap mapIter_;
	};

	// A class that maps strings to IdentifierInfo.
	// This contains every (user-defined) identifier currently in use.
	class IdentifierTable
	{
		private:
			using IDTableIterator = std::map<std::string, IdentifierInfo>::iterator;
			using IDTableConstIterator = std::map<std::string, IdentifierInfo>::const_iterator;
		public:
			IdentifierTable();

			// Returns the identifierinfo of the string "id" if it exists. 
			// If it does not exists, it creates a new entry into the table and returns it.
			IdentifierInfo* getUniqueIdentifierInfo(const std::string& id);
			IdentifierInfo* getInvalidID();

			// Returns true if the identifier exists in the map, false otherwise.
			bool exists(const std::string &id) const;

			// Iterators
			IDTableConstIterator begin() const;
			IDTableIterator begin();

			IDTableConstIterator end() const;
			IDTableIterator end();
		private:
			IdentifierInfo* invalidID = nullptr;

			// Deleted methods
			IdentifierTable(const IdentifierTable&) = delete;
			IdentifierTable& operator=(const IdentifierTable&) = delete;

			// Member variables
			std::map<std::string,IdentifierInfo> table_;
	};
}