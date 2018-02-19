////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : Symbols.hpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
// a DataMap (map of strings-fval) with various operations to safely operate on it.s
////------------------------------------------------------////

#pragma once

#include "Moonshot/Common/Types/Types.hpp"

#include <tuple> // std::pair
#include <map> // std::map

#define LOG_IMPLICIT_CASTS false

// Symbols table manager
namespace Moonshot
{
	class Context;
	class DataMap
	{
		public:
			DataMap(Context& c);
			~DataMap();
			// Retrieve a value (return it). Throws a critical error if the value isn't a basic type.
			FVal retrieveValue(const std::string& varname);
	
			// getVAttr
			var::varattr retrieveVarAttr(const std::string& varname);

			// Declare a new value. Throws an error if v_attr.type != initVal.index()
			bool declareValue(const var::varattr& v_attr,const FVal& initVal = FVal());

			// Sets a new value. Throws an error if the type is different, or the value is const.
			bool setValue(const std::string& varname, const FVal& newVal);

			void dump() const;
		private:
			// Context
			Context& context_;
			// Further down the line, to manage contexts, create a vector of theses, and, when searching, search every table.
			std::map<var::varattr, FVal> sym_table_;

			// Getters/setters for the symbols table.
			std::pair<var::varattr, FVal> map_getEntry(const std::string& str,bool& successFlag);
			bool map_setEntry(const std::string& vname, const FVal& vvalue,const bool& isDecl = false);
			bool map_getEntry(const var::varattr& vattr,FVal initval);

			// Helper function : creates a key.
			var::varattr createTempKey(const std::string& v_name);
	};

}
