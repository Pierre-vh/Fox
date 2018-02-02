////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : Symbols.cpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
//			SEE HEADER FILE FOR MORE INFORMATION			
////------------------------------------------------------////

#include "Symbols.h"

using namespace Moonshot;

SymbolsTable::SymbolsTable(Context& c) : context_(c)
{
}


SymbolsTable::~SymbolsTable()
{
}

FVal SymbolsTable::retrieveValue(const std::string & varname)
{
	bool successFlag;
	auto res = symtable_getEntry(varname,successFlag);
	if (successFlag)
		return res.second;
	return FVal();
}

var::varattr SymbolsTable::retrieveVarAttr(const std::string & varname)
{
	bool successFlag;
	auto res = symtable_getEntry(varname, successFlag);
	if (successFlag)
		return res.first;
	return var::varattr();
}

bool SymbolsTable::declareValue(const var::varattr & v_attr, const FVal & initVal)
{
	if (std::holds_alternative<std::monostate>(initVal))
		return symtable_addEntry(v_attr,fv_util::getSampleFValForIndex(v_attr.type_)); // Init with a default value.
	return symtable_addEntry(v_attr, initVal);
}

bool SymbolsTable::setValue(const std::string & varname, const FVal & newVal)
{
	return symtable_setEntry(varname, newVal);
}

void SymbolsTable::dumpSymbolsTable() const
{
	std::stringstream out;
	out << "Dumping symbols table...\n";
	for (auto& elem : sym_table_)
	{
		out << "NAME: " << elem.first.name_ << " TYPE: " << fv_util::indexToTypeName(elem.first.type_) << " ---> VALUE: " << fv_util::dumpFVal(elem.second) << std::endl;
	}
	context_.logMessage(out.str());
	out.clear();
}

std::pair<var::varattr, FVal> SymbolsTable::symtable_getEntry(const std::string & str, bool& successFlag)
{
	auto it = sym_table_.find(
		createTempKey(str)
	);
	if (it != sym_table_.end())
	{
		
		successFlag = true;
		return { it->first, it->second };
	}
	context_.reportError("Undeclared variable " + str);
	successFlag = false;
	return std::pair<var::varattr, FVal>();
}

bool SymbolsTable::symtable_setEntry(const std::string & vname,const FVal& vvalue, const bool& isDecl)
{
	auto it = sym_table_.find(
		createTempKey(vname)
	);
	if (it != sym_table_.end())
	{
		if (it->first.type_ != vvalue.index()) //  trying to modify the type ? not on my watch.
		{
			// Implicit cast
			if (LOG_IMPLICIT_CASTS)
			{
				std::stringstream out;
				out << "Implicit cast : Attempted to store a " << fv_util::indexToTypeName(vvalue.index()) << " into the variable ";
				out << vname << " (of type " << fv_util::indexToTypeName(it->first.type_) << ")\n";
				out << "Attempting cast to the desired type...";
				context_.logMessage(out.str());
			}
			auto castVal = castTo(context_,it->first.type_, vvalue);
			if(context_.isSafe())	// Cast went well
				return symtable_setEntry(vname,castVal,isDecl); // Proceed
			return false; // Bad cast : abort
		}
		// Error cases
		if (it->first.isConst && !isDecl) // if the variable is const, and we're not in a declaration
			context_.reportError("Can't assign a value to const variable \"" + vname + "\". Const variables must be initialized at declaration and can't be changed later.");
		// No error ? proceed.
		else
		{
			it->second = vvalue; // update the value
			return true;
		}
	}
	else
		context_.reportError("Unknown variable " + vname + "!");
	return false; // No value found ? return false.
}

bool SymbolsTable::symtable_addEntry(const var::varattr & vattr,FVal initval)
{
	auto ret = sym_table_.insert({ vattr,FVal() });
	if (ret.second)
		return symtable_setEntry(vattr.name_, initval,true); 	// Attempt to assign the initial value
	else 
		context_.reportError("Variable " + vattr.name_ + " is already declared.");
	return ret.second; // ret.second is a "flag" if the operation was successful.
}

