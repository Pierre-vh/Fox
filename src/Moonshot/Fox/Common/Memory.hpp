////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : Memory.hpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
// This file contains various classes & wrappers that are used to manage, or help
// manage memory.
// This file is header only, since most, if not every class here
// will be templated.
////------------------------------------------------------////
#pragma once

#include <memory>
#include <vector>

namespace Moonshot
{
	// Typedef for UniquePtrVectors, to make them a bit less verbose.
	template <typename Ty>
	using UniquePtrVector = std::vector<std::unique_ptr<Ty>>;
	
	// Iterator wrapper for iterators that point to a unique_ptr that won't expose the unique_ptr, but 
	// use .get() to only expose the raw pointer.
	// Based on an article https://jonasdevlieghere.com/containers-of-unique-pointers/
	template <typename BaseIterator>
	class DereferenceIterator : public BaseIterator
	{
		public:	
			using value_type = typename BaseIterator::value_type::element_type;
			using pointer = value_type * ;
			using reference = value_type & ;

			DereferenceIterator(const BaseIterator &baseIt) : BaseIterator(baseIt)
			{

			}

			// Operator * returns the pointer
			pointer operator*() const { return this->BaseIterator::operator*().get(); }
			// Operator -> lets you access the members directly. It's equivalent to (*it)->
			pointer operator->() const { return this->BaseIterator::operator*().get(); }
			reference operator[](size_t n) const {
				return *(this->BaseIterator::operator[](n));
			}
	};
}