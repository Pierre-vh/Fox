////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : Symbols.cpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
//			SEE HEADER FILE FOR MORE INFORMATION			
////------------------------------------------------------////

#include "DataMap.hpp"

#include "Moonshot/Common/Types/TypesUtils.hpp"
#include "Moonshot/Common/Types/FoxValueUtils.hpp"
#include "Moonshot/Common/Types/TypeCast.hpp"
#include "Moonshot/Common/Context/Context.hpp"

#include <sstream> // std::stringstream

using namespace Moonshot;

DataMap::DataMap(Context& c) : context_(c)
{

}


DataMap::~DataMap()
{
}

FoxValue DataMap::retrieveValue(const std::string & varname)
{
	bool successFlag;
	auto res = map_getEntry(varname,successFlag);
	if (successFlag)
		return res.second;
	return FoxValue();
}

FoxVariableAttr DataMap::retrieveVarAttr(const std::string & varname)
{
	bool successFlag;
	auto res = map_getEntry(varname, successFlag);
	if (successFlag)
		return res.first;
	return FoxVariableAttr();
}

bool DataMap::declareValue(const FoxVariableAttr & v_attr, const FoxValue & initVal)
{
	if (std::holds_alternative<VoidType>(initVal))
		return map_getEntry(v_attr,FValUtils::getSampleFValForIndex(v_attr.type_.getTypeIndex())); // Init with a default value.
	return map_getEntry(v_attr, initVal);
}

bool DataMap::setValue(const std::string & varname, const FoxValue & newVal)
{
	return map_setEntry(varname, newVal);
}

void DataMap::dump() const
{
	std::stringstream out;
	out << "Dumping symbols table...\n";
	for (auto& elem : sym_table_)
	{
		out << "NAME: " << elem.first.name_ << " TYPE: " << elem.first.type_.getTypeName() << " ---> VALUE: " << FValUtils::dumpFVal(elem.second) << std::endl;
	}
	context_.logMessage(out.str());
	out.clear();
}

std::pair<FoxVariableAttr, FoxValue> DataMap::map_getEntry(const std::string & str, bool& successFlag)
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
	return std::pair<FoxVariableAttr, FoxValue>();
}

bool DataMap::map_setEntry(const std::string & vname,const FoxValue& vvalue, const bool& isDecl)
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
				out << "Implicit cast : Attempted to store a FoxValue of type" << FValUtils::getFValTypeName(vvalue) << " into the variable ";
				out << vname << " (of type " << it->first.type_.getTypeName() << ")\n";
				out << "Attempting cast to the desired type...";
				context_.logMessage(out.str());
			}
			auto castVal = CastUtilities::performImplicitCast(context_,it->first.type_, vvalue);
			if(context_.isSafe())	// Cast went well
				return map_setEntry(vname,castVal,isDecl); // Proceed
			return false; // Bad cast : abort
		}
		// Error cases
		if (it->first.type_.isConst() && !isDecl) // if the variable is const, and we're not in a declaration
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

bool DataMap::map_getEntry(const FoxVariableAttr & vattr,FoxValue initval)
{
	auto ret = sym_table_.insert({ vattr,FoxValue() });
	if (ret.second)
		return map_setEntry(vattr.name_, initval,true); 	// Attempt to assign the initial value
	else 
		context_.reportError("Variable " + vattr.name_ + " is already declared.");
	return ret.second; // ret.second is a "flag" if the operation was successful.
}

FoxVariableAttr DataMap::createTempKey(const std::string& v_name)
{
	return FoxVariableAttr(v_name);
}