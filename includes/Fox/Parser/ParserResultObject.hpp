//----------------------------------------------------------------------------//
// This file is part of the Fox project.        
// See the LICENSE.txt file at the root of the project for license information.            
// File : ParserResultObject.hpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//
// This file contains the ParserResultObject class which is used by the Parser's
// parsing methods to return values.
//----------------------------------------------------------------------------//

#pragma once

#include "Fox/Common/LLVM.hpp"
#include "Fox/Common/Errors.hpp"
#include "llvm/ADT/PointerIntPair.h"
#include <type_traits>

namespace fox {
  enum class ParserResultKind : std::uint8_t {
    Success, Error, NotFound
    // There's still room for one more ParserResultKind. If more is 
    // added, update bitsForPRK below.
  };

  namespace detail {
    constexpr unsigned bitsForPRK = 2;

    template<typename DataTy>
    struct IsEligibleForPointerIntPairStorage {
      static constexpr bool value = false;
    };

    template<typename DataTy>
    struct IsEligibleForPointerIntPairStorage<DataTy*> {
      static constexpr bool value = alignof(DataTy) >= bitsForPRK;
    };

    template<typename DataTy, bool canUsePointerIntPair = 
      IsEligibleForPointerIntPairStorage<DataTy>::value>
    class ParserResultObjectDataStorage {
      DataTy data_ = DataTy();
      ParserResultKind kind_;
      public:
        using default_value_type = DataTy; 
        using value_type = DataTy;

        ParserResultObjectDataStorage(const value_type& data, 
                                      ParserResultKind kind):
          data_(data), kind_(kind) {}

        ParserResultObjectDataStorage(value_type&& data, 
                                      ParserResultKind kind):
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

        ParserResultKind kind() const {
          return kind_;
        }
    };

    template<typename DataTy>
    class ParserResultObjectDataStorage<DataTy*, true> {
      llvm::PointerIntPair<DataTy*, bitsForPRK, ParserResultKind> pair_;
      public:
        using default_value_type = std::nullptr_t; 
        using value_type = DataTy*;

        ParserResultObjectDataStorage(DataTy* data, ParserResultKind kind):
          pair_(data, kind) {}

        DataTy* data() {
          return pair_.getPointer();
        }

        const DataTy* data() const {
          return pair_.getPointer();
        }

        ParserResultKind kind() const {
          return pair_.getInt();
        }
    };
  }

  template<typename DataTy>
  class ParserResultObject {
    using StorageType = detail::ParserResultObjectDataStorage<DataTy>;
    protected:
      using DefaultValue = DataTy;
      using CTorValueTy = const DataTy&;
      using CTorRValueTy = DataTy && ;
      static constexpr bool isPointerType = std::is_pointer<DataTy>::value;

    public:
      ParserResultObject(ParserResultKind kind, const DataTy& data):
        storage_(data, kind) {}

      template<typename = typename std::enable_if<!isPointerType>::type>
      ParserResultObject(ParserResultKind kind, DataTy&& data):
        storage_(data, kind) {}

      explicit ParserResultObject(ParserResultKind kind) :
        storage_(StorageType::default_value_type(), kind) {}

      bool isSuccess() const {
        return getResultKind() == ParserResultKind::Success;
      }

      bool isNotFound() const {
        return getResultKind() == ParserResultKind::NotFound;
      }

      bool isError() const {
        return getResultKind() == ParserResultKind::Error;
      }

      ParserResultKind getResultKind() const {
        return storage_.kind();
      }

      DataTy get() {
        return storage_.data();
      }

      const DataTy get() const {
        return storage_.data();
      }

      template<typename Ty, typename = typename 
               std::enable_if<isPointerType>::type>
      Ty* castTo() {
        DataTy ptr = storage_.data();
        assert(ptr && "Can't use this on a null pointer");
        return cast<Ty>(ptr);
      }

      template<typename Ty>
      const auto castTo() const {
        return const_cast<ParserResultObject<DataTy>*>(this)->castTo<Ty>();
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