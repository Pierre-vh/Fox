////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : Symbols.h											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
// The Symbols Table class. A Class encapsulating a hashtable (map) used to
// manipulate the symbols table, and performs various checks on it.
////------------------------------------------------------////

#pragma once

#include "../Types/Types.h"

#include <tuple> // std::pair
#include <map> // std::map

#define LOG_IMPLICIT_CASTS false

// Symbols table manager
namespace Moonshot
{
	class Context;
	class SymbolsTable
	{
		public:
			SymbolsTable(Context& c);
			~SymbolsTable();
			// Retrieve a value (return it). Throws a critical error if the value isn't a basic type.
			FVal retrieveValue(const std::string& varname);
	
			// getVAttr
			var::varattr retrieveVarAttr(const std::string& varname);

			// Declare a new value. Throws an error if v_attr.type != initVal.index()
			bool declareValue(const var::varattr& v_attr,const FVal& initVal = FVal());

			// Sets a new value. Throws an error if the type is different, or the value is const.
			bool setValue(const std::string& varname, const FVal& newVal);

			void dumpSymbolsTable() const;
		private:
			// Context
			Context& context_;
			// Further down the line, to manage contexts, create a vector of theses, and, when searching, search every table.
			std::map<var::varattr, FVal> sym_table_;

			// Getters/setters for the symbols table.
			std::pair<var::varattr, FVal> symtable_getEntry(const std::string& str,bool& successFlag);
			bool symtable_setEntry(const std::string& vname, const FVal& vvalue,const bool& isDecl = false);
			bool symtable_addEntry(const var::varattr& vattr,FVal initval);

			// Helper function : creates a key.
			inline var::varattr createTempKey(const std::string& v_name)
			{
				return var::varattr(v_name);
			}
	};

}
