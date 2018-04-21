////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : ParsingResult.hpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
// This class defines an object to encapsulate parsing results (ASTNodes pointers).
////------------------------------------------------------////

#pragma once

#include <memory>

namespace Moonshot
{
	// ParseRes provides 2 specialization, one for
	// "normal" objects, and one of pointers. The one for pointer 
	// manages it using a std::unique_ptr.
	template<typename DataTy>
	struct ParseRes {
		public:
			ParseRes(const DataTy& res)
			{
				successFlag_ = true;
				hasData_ = true;
				result = res;
			}

			ParseRes(const bool &wasSuccessful = true)
			{
				hasData_ = false;
				successFlag_ = wasSuccessful;
				result = DataTy();
			}

			operator bool() const
			{
				return isUsable();
			}

			// Returns true if the Parsing function reported a successful parsing, or a failure.
			bool wasSuccessful() const
			{
				return successFlag_;
			}
			
			// Returns true if this ParseRes contains usable data.
			bool isUsable() const
			{
				return successFlag_  && hasData_;
			}

			// The result's data
			DataTy result;
		private:
			bool successFlag_ : 1;
			bool hasData_ : 1;
	};

	// Special overload for pointer types
	template<typename DataTy>
	struct ParseRes<DataTy*> {
		public:
			ParseRes(std::unique_ptr<DataTy> res)
			{
				successFlag_ = true;
				result = std::move(res);
			}

			ParseRes(const bool &wasSuccessful = true)
			{
				successFlag_ = wasSuccessful;
				result = nullptr;
			}

			operator bool() const
			{
				return isUsable();
			}

			// Returns true if the Parsing function reported a successful parsing, or a failure.
			bool wasSuccessful() const
			{
				return successFlag_;
			}

			// Returns true if this ParseRes contains usable data.
			bool isUsable() const
			{
				return successFlag_ && (bool)result;
			}

			// The pointer
			std::unique_ptr<DataTy> result;
		private:
			bool successFlag_ : 1;
	};

	// Forward decl ASTUnit
	class ASTUnit;
	// Parsing Result Specific to Units.
		// Success/Failure is deduced from the value of the unit pointer (nullptr = failure)
	class UnitParsingResult
	{
		public:

			UnitParsingResult(std::unique_ptr<ASTUnit> parsedUnit = nullptr)
			{
				unit = std::move(parsedUnit);
			}

			operator bool() const
			{
				return (bool)(unit);
			}

			// the parsed unit
			std::unique_ptr<ASTUnit> unit;
	};
}