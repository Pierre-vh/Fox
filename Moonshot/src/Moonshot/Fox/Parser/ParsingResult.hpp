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
	enum class ParsingOutcome {
		SUCCESS, 
		NOTFOUND,
		FAILED_WITHOUT_ATTEMPTING_RECOVERY,
		FAILED_AND_DIED, 
		FAILED_BUT_RECOVERED
	};
	/*
		Usage
			* Parsing function finds all the tokens and return a fully formed node:
				use SUCCESS flag
				hasRecovered() returns true 
				wasSuccessful() returns true
			* Parsing function doesn't find the first token and just returns nullptr 
				use NOTFOUND flag
				hasRecovered() returns true 
				wasSuccessful() returns true
			* Parsing function finds the first tokens, but encounters an unexpected token and dies: 
				use FAILED_AND_DIED flag
				hasRecovered() returns true
				wasSuccessful() returns false
			* Parsing function finds the first tokens, but encounters an unexpected token and successfully recovers to the semicolon,parens or curly bracket.
				use FAILED_AND_RECOVERED flag
				hasRecovered() returns true if the node is not null.
				wasSuccessful() returns false
			* Parsing function finds the first tokens, but encounters an unexpected token and successfully recovers to the semicolon,parens or curly bracket.
				use FAILED_WITHOUT_ATTEMPTING_RECOVERY flag
				hasRecovered() returns true if the node is not null.
				wasSuccessful() false false
	*/
	template<typename Ty>
	struct ParsingResultData
	{
		Ty result_;
	};

	template<typename Ty>
	struct ParsingResultData<Ty*>
	{
		std::unique_ptr<Ty> result_;
	};

	template<typename DataTy>
	struct ParsingResult : ParsingResultData<DataTy> {
		public:
			using ParsingResultData<DataTy>::result_;
			using ResultType = decltype(result_);

			ParsingResult(const ParsingOutcome& pc, ResultType res)
			{
				// set data
				if constexpr (std::is_pointer<DataTy>::value)
				{
					if (res)
					{
						result_ = std::move(res);
						isDataAvailable_ = true;
					}
					else
					{
						result_ = nullptr;
						isDataAvailable_ = false;
					}
				}
				else
				{
					isDataAvailable = true;
					result_ = res;
				}
				// set flags
				enumflag_ = pc;
				if (pc == ParsingOutcome::SUCCESS || pc == ParsingOutcome::NOTFOUND)
				{
					successFlag_ = true;
					recovered_ = true;
				}
				else if (pc == ParsingOutcome::FAILED_AND_DIED)
				{
					successFlag_ = false;
					recovered_ = false;
				}
				else if (pc == ParsingOutcome::FAILED_BUT_RECOVERED || pc == ParsingOutcome::FAILED_WITHOUT_ATTEMPTING_RECOVERY)
				{
					successFlag_ = isDataAvailable();
					recovered_ = (pc == ParsingOutcome::FAILED_BUT_RECOVERED);
				}
			}

			ParsingResult(const ParsingOutcome& pc)
			{
				enumflag_ = pc;
				if (pc == ParsingOutcome::SUCCESS || pc == ParsingOutcome::NOTFOUND)
				{
					successFlag_ = true;
					recovered_ = true;
				}
				else if (pc == ParsingOutcome::FAILED_AND_DIED)
				{
					successFlag_ = false;
					recovered_ = false;
				}
				else if (pc == ParsingOutcome::FAILED_BUT_RECOVERED || pc == ParsingOutcome::FAILED_WITHOUT_ATTEMPTING_RECOVERY)
				{
					successFlag_ = false;
					recovered_ = (pc == ParsingOutcome::FAILED_BUT_RECOVERED);
				}
			}

			operator bool() const
			{
				return wasSuccessful() && isDataAvailable();
			}

			ParsingOutcome getFlag() const
			{
				return enumflag_;
			}

			bool wasSuccessful() const
			{
				return successFlag_;
			}

			bool hasRecovered() const
			{
				return recovered_;
			}

			bool isPointer() const
			{
				return std::is_pointer<DataTy>::value;
			}

			bool isDataAvailable() const
			{
				return isDataAvailable_;
			}
		private:
			ParsingOutcome enumflag_;
			bool successFlag_ = false, recovered_ = true, isDataAvailable_ = false;
	};
}