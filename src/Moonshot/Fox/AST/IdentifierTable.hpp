///------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : IdentifierTable.hpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
// This file contains the IdentifierTable, which is a class
// that manages a "table" of identifiers. Every lexed user-defined 
// identifier ends up here.
//
// The main goal of this class is 2 fold.
// 1 - Reduce memory usage a bit, every identifier is stored once, and then only pointers to it are stored.
// 2 - Enable a centralized identifier table, which allows to attach specific information to an identifier.
// 
// This class offers lookup functions using Strings, and can return a reference or a pointer to a map entry.
// 
// Note: This structure is heavily inspired by CLang. Well, a lot here is inspired by this incredible compiler.
// Of course, I adapt and simplify some stuff, but the core ideas come from CLang
// 
// Also, note that 0 lines of source code were copy-pasted from the source code of CLang. I'm only attempting to reproduce
// what I understand that they're doing to increase the quality of this compiler and learn more about how CLang works.
////------------------------------------------------------////

#include <map>
#include <string>

namespace Moonshot
{
	class IdentifierInfo;

	// Wrapper around a const_iterator of a map entry, used to safely access the .first element (the string)
	// This is used to avoid storing a raw pointer or iterator.
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
	// That's the advantage of this class, I can add information to an identifier without breaking everything!
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

	// A class that maps strings to IdentifierInfo.
	// This contains every (user-defined) identifier currently in use.
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