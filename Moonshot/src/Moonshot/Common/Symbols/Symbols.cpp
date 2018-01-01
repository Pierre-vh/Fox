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

SymbolsTable::SymbolsTable()
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
	if (!symtable_addEntry(v_attr, initVal))
	{
		E_ERROR("Variable " + v_attr.name + " is already declared.");
		return false;
	}
	return true;
}

bool SymbolsTable::setValue(const std::string & varname, const FVal & newVal)
{
	if (!symtable_setEntry(varname, newVal))
	{
		E_ERROR("Tried to assign a value to a undeclared variable.");
		return false;
	}
	return true;
}

void SymbolsTable::dumpSymbolsTable() const
{
	for (auto& elem : sym_table_)
	{
		std::cout << "NAME: " << elem.first.name << " TYPE: " << fv_util::indexToTypeName(elem.first.type) << " ---> VALUE: " << fv_util::dumpFVal(elem.second) << std::endl;
	}
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
	E_ERROR("Undeclared variable " + str);
	successFlag = false;
	return std::pair<var::varattr, FVal>();
}

bool SymbolsTable::symtable_setEntry(const std::string & vname, const FVal & vvalue)
{
	auto it = sym_table_.find(
		createTempKey(vname)
	);
	if (it != sym_table_.end())
	{
		if (it->first.isConst)
			E_ERROR("Can't assign a value to const variable \"" + vname + "\". Const variables must be initialized at declaration and can't be changed later.");
		else
		{
			if (vvalue.index() == fv_util::fval_vattr) // if we try to assign a var attr to a variable, it's because we try to copy values.
				it->second = retrieveValue(std::get<var::varattr>(vvalue).name);
			else
				it->second = vvalue; // update the value
			return true;
		}
	}
	return false; // No value found ? return false.
}

bool SymbolsTable::symtable_addEntry(const var::varattr & vattr,FVal initval)
{
	if (initval.index() == fv_util::fval_vattr) // if we try to assign a var attr to a variable, it's because we try to copy values.
		initval = retrieveValue(std::get<var::varattr>(initval).name);

	auto ret = sym_table_.insert({ vattr,initval });
	return ret.second; // ret.second is a "flag" if the operation was successful.
}

