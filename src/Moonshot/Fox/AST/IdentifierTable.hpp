///------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : IdentifierTable.hpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
// This file contains the IdentifierTable, which is a class
// that manages a "table" of identifiers. Every lexed identifier
// ends up here.
// This class offers lookup function using strings.
//
// Note: This structure is heavily inspired by CLang/Swift.
////------------------------------------------------------////

#include <map>
#include <string>

namespace Moonshot
{
	class IdentifierInfo;

	// Wrapper around a const_iterator of a map entry, used to safely access the .first element (the string)
	// Based on a SO answer:  https://stackoverflow.com/a/516041
	class StringPtrInMap
	{
		public:
			typedef std::map<std::string, IdentifierInfo>::const_iterator ItTy;

			StringPtrInMap(ItTy iter);

			const std::string& get() const;
			ItTy it_;

	};

	// A Class holding informations related to a identifier.
	// Currently, it only lets us access the std::string. In the future, this might hold additional informations.
	class IdentifierInfo
	{
		public:
			IdentifierInfo(StringPtrInMap::ItTy iter);
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

	// a map containing every identifier currently in use.
	class IdentifierTable
	{
		private:
			using IDTableIterator = std::map<std::string, IdentifierInfo>::iterator;
			using IDTableConstIterator = std::map<std::string, IdentifierInfo>::const_iterator;
		public:
			IdentifierTable() = default;

			// Returns the identifierinfo of the string "id" if it exists. 
			// If it does not exists, it creates a new entry into the table and returns it.
			IdentifierInfo& getUniqueIDinfo(const std::string& id);

			// Works the same as getUniqueIDinfo, but returns a pointer.
			// The pointer is guaranteed to be non null.
			IdentifierInfo* getUniqueIDInfoPtr(const std::string& id);

			// Returns true if the identifier exists in the map, false otherwise.
			bool exists(const std::string &id) const;

			// Iterators
			IDTableConstIterator begin() const;
			IDTableIterator begin();

			IDTableConstIterator end() const;
			IDTableIterator end();
		private:
			// Deleted methods
			IdentifierTable(const IdentifierTable&) = delete;
			IdentifierTable& operator=(const IdentifierTable&) = delete;

			// Member variables
			std::map<std::string,IdentifierInfo> table_;
	};
}