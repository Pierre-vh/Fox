#include "Utils.h"

template<typename Derived, typename Base, typename Del>
std::unique_ptr<Derived, Del>
dynamic_unique_ptr_cast(std::unique_ptr<Base, Del>&& p)
{
	if (Derived *result = dynamic_cast<Derived *>(p.get())) {
		p.release();
		return std::unique_ptr<Derived, Del>(result, std::move(p.get_deleter()));
	}
	return std::unique_ptr<Derived, Del>(nullptr, p.get_deleter());
}