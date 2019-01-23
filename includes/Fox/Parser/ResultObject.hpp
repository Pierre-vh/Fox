//----------------------------------------------------------------------------//
// This file is part of the Fox project.        
// See the LICENSE.txt file at the root of the project for license information.            
// File : ResultObject.hpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//
// This file contains the ResultObject class which is used by the Parser's
// parsing methods to return values.
//----------------------------------------------------------------------------//

#pragma once

#include "Fox/Common/LLVM.hpp"
#include "Fox/Common/Errors.hpp"
#include "llvm/ADT/PointerIntPair.h"
#include <type_traits>

namespace fox {
  template<typename DataTy>
  class ResultObject {
    protected:
      using DefaultValue = DataTy;
      using CTorValueTy = const DataTy&;
      using CTorRValueTy = DataTy && ;
    public:
      ResultObject(bool success, const DataTy& res):
        result_(res), hasData_(true), successFlag_(success) {

      }

      ResultObject(bool success, DataTy&& res):
        result_(res), hasData_(true), successFlag_(success) {

      }

      explicit ResultObject(bool success) :
        result_(DefaultValue()), hasData_(false), successFlag_(success) {

      }

      bool wasSuccessful() const {
        return successFlag_;
      }

      bool hasData() const {
        return hasData_;
      }

      DataTy get() const {
        return result_;
      }

      DataTy&& move() {
        return std::move(result_);
      }

    private:
      bool hasData_ : 1;
      bool successFlag_ : 1;
      DataTy result_;
  };

  template<typename DataTy>
  class ResultObject<DataTy*> {
    protected:
      using DefaultValue = std::nullptr_t;
      using CTorValueTy = DataTy*;
      // Disable the Move CTor if we have a pointer
      using CTorRValueTy = std::enable_if<false, void>;
      using ThisTy = ResultObject<DataTy*>;

    public:
      ResultObject(bool success, CTorValueTy ptr):
        data_(ptr, success) {

      }

      ResultObject(bool success):
        data_(nullptr, success) {

      }

      bool wasSuccessful() const {
        return data_.getInt();
      }
      
      bool hasData() const {
        return data_.getPointer();
      }

      DataTy* get() const {
        return data_.getPointer();
      }

      template<typename Ty>
      Ty* castTo() {
        auto* ptr = data_.getPointer();
        assert(ptr && "Can't use this on a null pointer");
        return cast<Ty>(ptr);
      }

      template<typename Ty>
      const Ty* castTo() const {
        return const_cast<ThisTy*>(this)->castTo<Ty>();
      }

      void* getOpaque() const {
        return data_.getPointer();
      }

    private:
      llvm::PointerIntPair<DataTy*, 1> data_;
  };
}