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
				result = res;
			}

			ParsingResult(const bool &wasSuccessful = true)
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
			
			// Returns true if this ParsingResult contains usable data.
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

			// Returns true if this ParsingResult contains usable data.
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
		// Unit Parsing result is trivial, because it attempts
		// to return the Unit even on failure. 
		// Success/Failure is deduced from the value of the unit pointer (nullptr = failure)
	class UnitParsingResult
	{
		public:

			UnitParsingResult(std::unique_ptr<ASTUnit> parsedUnit = nullptr);
			operator bool() const;

			// the parsed unit
			std::unique_ptr<ASTUnit> unit;
	};

	// Resync results
	class ResyncResult
	{
		public:
			// First parameter is true/false for if recovery succeeded, second is for if it has recovered on the requested token.
			ResyncResult(const bool& succeeded, const bool& onRequestedToken = false);

			operator bool();
			bool hasRecovered() const;
			bool hasRecoveredOnRequestedToken() const;
		private:
			bool resynced_ : 1;
			bool resyncedOnRequestedToken_ : 1;
	};
}