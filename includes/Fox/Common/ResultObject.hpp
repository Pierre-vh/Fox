//----------------------------------------------------------------------------//
// This file is a part of The Moonshot Project.        
// See the LICENSE.txt file at the root of the project for license information.            
// File : ResultObject.hpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//
// This file contains the ResultObject class, which is a "baseline"
// for classes that needs a special "Result" object that encapsulates
// a value/result + a boolean flag.
//
// The boolean is meant to contain a "success" flag 
// (true = operation was successful, false if not)
// and is accessible through wasSuccessful
//----------------------------------------------------------------------------//

#pragma once

#include "LLVM.hpp"
#include "llvm/ADT/PointerIntPair.h"
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

      explicit ResultObject(bool success) :
        result_(DefaultValue()), hasData_(false), successFlag_(success)
      {

      }

      bool wasSuccessful() const
      {
        return successFlag_;
      }

      bool hasData() const
      {
        return hasData_;
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
      ResultObject(bool success, CTorValueTy ptr):
        data_(ptr, success)
      {

      }

      ResultObject(bool success):
        data_(nullptr, success)
      {

      }

      bool wasSuccessful() const
      {
        return data_.getInt();
      }
      
      bool hasData() const
      {
        return data_.getPointer();
      }

      const DataTy* get() const
      {
        return data_.getPointer();
      }

      DataTy* get()
      {
        return data_.getPointer();
      }

      template<typename Ty>
      Ty* getAs()
      {
        auto* ptr = data_.getPointer();
        assert(ptr && "Can't use this on a null pointer");
        Ty* cast = dyn_cast<Ty>(ptr);
        assert(cast && "Incorrect type!");
        return cast;
      }

      template<typename Ty>
      const Ty* getAs() const
      {
        auto* ptr = data_.getPointer();
        assert(ptr && "Can't use this on a null pointer");
        Ty* cast = dyn_cast<Ty>(ptr);
        assert(cast && "Incorrect type!");
        return cast;
      }

      void* getOpaque()
      {
        return data_.getPointer();
      }

      const void* getOpaque() const
      {
        return data_.getPointer();
      }

    private:
      llvm::PointerIntPair<DataTy*, 1> data_;
  };
}