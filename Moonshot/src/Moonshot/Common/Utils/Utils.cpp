#include "Utils.h"

using namespace Moonshot;

template<typename Derived, typename Base, typename Del>
std::unique_ptr<Derived, Del>
util::dynamic_unique_ptr_cast(std::unique_ptr<Base, Del>&& p)
{
	if (Derived *result = dynamic_cast<Derived *>(p.get())) {
		p.release();
		return std::unique_ptr<Derived, Del>(result, std::move(p.get_deleter()));
	}
	return std::unique_ptr<Derived, Del>(nullptr, p.get_deleter());
}

std::string util::filepath_MoonshotProj(const std::string & s)
{
	return ".\\..\\..\\..\\..\\Moonshot\\" + s;
}