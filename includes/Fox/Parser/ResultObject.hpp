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
  enum class ResultKind : std::uint8_t {
    Success, Error, NotFound
  };

  namespace detail {
    template<typename DataTy>
    struct IsEligibleForPointerIntPairStorage {
      static constexpr bool value = false;
    };

    template<typename DataTy>
    struct IsEligibleForPointerIntPairStorage<DataTy*> {
      static constexpr bool value = alignof(DataTy) >= 2;
    };

    template<typename DataTy, bool canUsePointerIntPair = 
      IsEligibleForPointerIntPairStorage<DataTy>::value>
    class ResultObjectDataStorage {
      DataTy data_ = DataTy();
      ResultKind kind_;
      public:
        using default_value_type = DataTy; 
        using value_type = DataTy;

        ResultObjectDataStorage(const value_type& data, ResultKind kind):
          data_(data), kind_(kind) {}

        ResultObjectDataStorage(value_type&& data, ResultKind kind):
          data_(data), kind_(kind) {}

        value_type data() {
          return data_;
        }

        const value_type data() const {
          return data_;
        }

        value_type&& move() {
          return std::move(data_);
        }

        ResultKind kind() const {
          return kind_;
        }
    };

    template<typename DataTy>
    class ResultObjectDataStorage<DataTy*, true> {
      llvm::PointerIntPair<DataTy*, 2, ResultKind> pair_;
      public:
        using default_value_type = std::nullptr_t; 
        using value_type = DataTy*;

        ResultObjectDataStorage(DataTy* data, ResultKind kind):
          pair_(data, kind) {}

        DataTy* data() {
          return pair_.getPointer();
        }

        const DataTy* data() const {
          return pair_.getPointer();
        }

        ResultKind kind() const {
          return pair_.getInt();
        }
    };
  }

  template<typename DataTy>
  class ResultObject {
    using StorageType = detail::ResultObjectDataStorage<DataTy>;
    protected:
      using DefaultValue = DataTy;
      using CTorValueTy = const DataTy&;
      using CTorRValueTy = DataTy && ;
      static constexpr bool isPointerType = std::is_pointer<DataTy>::value;

    public:
      ResultObject(ResultKind kind, const DataTy& data):
        storage_(data, kind) {}

      template<typename = typename std::enable_if<!isPointerType>::type>
      ResultObject(ResultKind kind, DataTy&& data):
        storage_(data, kind) {}

      explicit ResultObject(ResultKind kind) :
        storage_(StorageType::default_value_type(), kind) {}

      bool wasSuccessful() const {
        return storage_.kind() == ResultKind::Success || storage_.kind() == ResultKind::NotFound;
      }

      ResultKind getResultKind() const {
        return storage_.kind();
      }

      DataTy get() {
        return storage_.data();
      }

      const DataTy get() const {
        return storage_.data();
      }

      template<typename Ty, typename = typename std::enable_if<isPointerType>::type>
      Ty* castTo() {
        DataTy ptr = storage_.data();
        assert(ptr && "Can't use this on a null pointer");
        return cast<Ty>(ptr);
      }

      template<typename Ty>
      const auto castTo() const {
        return const_cast<ResultObject<DataTy>*>(this)->castTo<Ty>();
      }

      template<typename = typename std::enable_if<isPointerType, void*>::type>
      void* getOpaque() const  {
        return (void*)storage_.data();
      }

      template<typename = typename std::enable_if<!isPointerType>::type>
      DataTy&& move() {
        return storage_.move();
      }

    private:
      StorageType storage_;
  };
}