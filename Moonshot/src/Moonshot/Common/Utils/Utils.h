#pragma once

#include <memory>

// https://stackoverflow.com/questions/21174593/downcasting-unique-ptrbase-to-unique-ptrderived
template<typename Derived, typename Base, typename Del>
std::unique_ptr<Derived, Del>
dynamic_unique_ptr_cast(std::unique_ptr<Base, Del>&& p);