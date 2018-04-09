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

	template<typename Ty>
	struct ParsingResultDataType
	{
		typedef Ty type;
	};

	template<typename Ty>
	struct ParsingResultDataType<Ty*>
	{
		typedef std::unique_ptr<Ty> type;
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

	template<typename DataTy>
	struct ParsingResult {
		public:
			using ResultType = typename ParsingResultDataType<DataTy>::type;

			ParsingResult(const ParsingOutcome& pc, ResultType res)
			{
				// set data
				if constexpr (std::is_pointer<DataTy>::value)
				{
					if (res)
					{
						result_ = std::move(res);
						data_ok_ = true;
					}
					else
					{
						result_ = nullptr;
						data_ok_ = false;
					}
				}
				else
				{
					data_ok_ = true;
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
					successFlag_ = false;
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
				return data_ok_;
			}

			ResultType result_;
		private:
			ParsingOutcome enumflag_;
			bool successFlag_ = false, recovered_ = true, data_ok_ = false;
	};
}