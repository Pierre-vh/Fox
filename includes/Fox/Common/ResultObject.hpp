////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : ResultObject.hpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
// This file contains the ResultObject class, which is a "baseline"
// for classes that needs a special "Result" object that encapsulates
// a value/result + a boolean flag.
////------------------------------------------------------//// 

#pragma once

#include "LLVM.hpp"
#include "Errors.hpp"
#include <type_traits>

namespace fox
{
	template<typename DataTy>
	class ResultObject
	{
		protected:
			using DefaultValue = DataTy;
			using CTorValueTy = const DataTy&;
			using CTorRValueTy = DataTy && ;
		public:
			ResultObject(bool success, const DataTy& res):
				result_(res), hasData_(true), successFlag_(success)
			{

			}

			ResultObject(bool success, DataTy&& res):
				result_(res), hasData_(true), successFlag_(success)
			{

			}

			ResultObject(bool success) :
				result_(DefaultValue()), hasData_(false), successFlag_(success)
			{

			}

			explicit operator bool() const
			{
				return isUsable();
			}

			bool wasSuccessful() const
			{
				return successFlag_;
			}

			bool hasData() const
			{
				return hasData_;
			}

			bool isUsable() const
			{
				return successFlag_ && hasData_;
			}

			const DataTy get() const
			{
				return result_;
			}

			DataTy get()
			{
				return result_;
			}

			DataTy& getRef()
			{
				return result_;
			}

		private:
			bool hasData_ : 1;
			bool successFlag_ : 1;
			DataTy result_;
	};

	template<typename DataTy>
	class ResultObject<DataTy*>
	{
		protected:
			using DefaultValue = std::nullptr_t;
			using CTorValueTy = DataTy*;
			using CTorRValueTy = std::enable_if<false, void>;  // Disable the Move CTor if we have a pointer

		public:
			ResultObject(bool success, CTorValueTy val):
				ptr_(val), successFlag_(success)
			{

			}

			ResultObject(bool success) :
				ptr_(nullptr), successFlag_(success)
			{

			}

			explicit operator bool() const
			{
				return isUsable();
			}

			bool wasSuccessful() const
			{
				return successFlag_;
			}
			
			bool hasData() const
			{
				return ptr_;
			}

			bool isUsable() const
			{
				return successFlag_ && ptr_;
			}

			const DataTy* get() const
			{
				return ptr_;
			}

			DataTy* get()
			{
				return ptr_;
			}

			template<typename Ty>
			Ty* getAs()
			{
				assert(ptr_ && "Can't use this on a null pointer");
				Ty* cast = dyn_cast<Ty>(ptr_);
				assert(cast && "Incorrect type!");
				return cast;
			}

			template<typename Ty>
			const Ty* getAs() const
			{
				assert(ptr_ && "Can't use this on a null pointer");
				Ty* cast = dyn_cast<Ty>(ptr_);
				assert(cast && "Incorrect type!");
				return cast;
			}

			void* getOpaque()
			{
				return ptr_;
			}

			const void* getOpaque() const
			{
				return ptr_;
			}

		private:
			bool successFlag_ : 1;
			DataTy* ptr_ = nullptr;
	};
}