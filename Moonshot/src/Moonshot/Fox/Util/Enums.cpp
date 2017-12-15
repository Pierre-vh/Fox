#include "Enums.h"

using namespace Moonshot;

std::string Moonshot::getFromDict(const std::map<parse::optype, std::string>& m, const parse::optype& op)
{
	auto i = m.find(op);
	if (i != m.end())
		return i->second;
	return "";
}
