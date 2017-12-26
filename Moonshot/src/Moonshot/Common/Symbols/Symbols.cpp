#include "Symbols.h"

using namespace Moonshot;

Symbols::Symbols()
{
}


Symbols::~Symbols()
{
}

FVal Symbols::retrieveValue(const std::string & varname)
{
	bool successFlag;
	auto res = symtable_getEntry(varname,successFlag);
	if (successFlag)
		return res.second;
	else
		E_ERROR("Variable " + varname + " does not exists.");
	return FVal();
}

bool Symbols::declareValue(const var::varattr & v_attr, const FVal & initVal)
{
	if (!symtable_addEntry(v_attr, initVal))
	{
		E_ERROR("Variable " + v_attr.name + " is already declared.");
		return false;
	}
	return true;
}

bool Symbols::setValue(const std::string & varname, const FVal & newVal)
{
	if (!symtable_setEntry(varname, newVal))
	{
		E_ERROR("Tried to assign a value to a undeclared variable.");
		return false;
	}
	return true;
}

std::pair<var::varattr, FVal> Symbols::symtable_getEntry(const std::string & str, bool& successFlag)
{
	auto it = sym_table_.find(
		createTempKey(str)
	);
	if (it != sym_table_.end())
	{
		successFlag = true;
		return { it->first, it->second };
	}
	successFlag = false;
	return std::pair<var::varattr, FVal>();
}

bool Symbols::symtable_setEntry(const std::string & vname, const FVal & vvalue)
{
	auto it = sym_table_.find(
		createTempKey(vname)
	);
	if (it != sym_table_.end())
	{
		it->second = vvalue; // update the value
		return true;
	}
	else
		return false; // No value found ? return false.
}

bool Symbols::symtable_addEntry(const var::varattr & vattr, const FVal & initval)
{
	auto ret = sym_table_.insert({ vattr,initval });
	return ret.second; // ret.second is a "flag" if the operation was successful.
}

