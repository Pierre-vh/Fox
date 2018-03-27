#include "Flags.hpp"
#include "Moonshot/Common//Exceptions/Exceptions.hpp"
using namespace Moonshot;

// FoxFlags
bool FlagsManager::isSet(const FoxFlag& ff) const
{
	if (existsInMap(fox_flags_, ff))
		return fox_flags_.find(ff)->second;
	else
		throw std::out_of_range("Enum value does not exists in map. This can happen if you add a new enum value without using the .def files!");
}

void FlagsManager::set(const FoxFlag& ff)
{
	if (existsInMap(fox_flags_, ff))
		fox_flags_[ff] = true;
	else
		throw std::out_of_range("Enum value does not exists in map. This can happen if you add a new enum value without using the .def files!");
}

void FlagsManager::unSet(const FoxFlag& ff)
{
	if (existsInMap(fox_flags_, ff))
		fox_flags_[ff] = false;
	else
		throw std::out_of_range("Enum value does not exists in map. This can happen if you add a new enum value without using the .def files!");
}

// CommonFlag
bool FlagsManager::isSet(const CommonFlag& ff) const
{
	if (existsInMap(common_flags_, ff))
		return common_flags_.find(ff)->second;
	else
		throw std::out_of_range("Enum value does not exists in map. This can happen if you add a new enum value without using the .def files!");
}

void FlagsManager::set(const CommonFlag& ff)
{
	if (existsInMap(common_flags_, ff))
		common_flags_[ff] = true;
	else
		throw std::out_of_range("Enum value does not exists in map. This can happen if you add a new enum value without using the .def files!");
}

void FlagsManager::unSet(const CommonFlag& ff)
{
	if (existsInMap(common_flags_, ff))
		common_flags_[ff] = false;
	else
		throw std::out_of_range("Enum value does not exists in map. This can happen if you add a new enum value without using the .def files!");
}