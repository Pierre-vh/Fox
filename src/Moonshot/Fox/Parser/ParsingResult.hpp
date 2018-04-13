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
	// ParsingResult provides 2 specialization, one for
	// "normal" objects, and one of pointers. The one for pointer 
	// manages it using a std::unique_ptr.
	template<typename DataTy>
	struct ParsingResult {
		public:
			ParsingResult(const DataTy& res)
			{
				successFlag_ = true;
				hasData_ = true;
				data_ = res;
			}

			ParsingResult(const bool &wasSuccessful = true)
			{
				hasData_ = false;
				successFlag_ = wasSuccessful;
				data_ = DataTy();
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
			
			// Returns true if this ParsingResult contains usable data.
			bool isUsable() const
			{
				return successFlag  && hasData_;
			}

			// The result's data
			DataTy result;
		private:
			bool successFlag_ : 1;
			bool hasData_ : 1;
	};

	// Special overload for pointer types
	template<typename DataTy>
	struct ParsingResult<DataTy*> {
	public:
		ParsingResult(std::unique_ptr<DataTy> res)
		{
			successFlag_ = true;
			result = std::move(res);
		}

		ParsingResult(const bool &wasSuccessful = true)
		{
			successFlag_ = wasSuccessful;
			data_ = nullptr;
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

		// Returns true if this ParsingResult contains usable data.
		bool isUsable() const
		{
			return successFlag && (bool)result;
		}

		// The pointer
		std::unique_ptr<DataTy> result;
		private:
			bool successFlag_ : 1;
	};
}