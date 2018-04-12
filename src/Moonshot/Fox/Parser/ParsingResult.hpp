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

	// Note : This is trash, and could use a rework.
	// First, simplify the ParsingOutcomes to just 2 : SUCCESS/FAILED
	// Second remove some useless information, like "hasRecovered", we know if it has recovered or not already with the success flag.
		// success & no data -> not found, isUsable return false
		// success & data -> found something, we don't care how, it just completed successfuly, isUsable returns true
		// failure & (we don't care) -> failed, hasn't recovered, panic!
	// then, make another overload of this class with "isPointer" to true
	// template<typename DataTy,bool isPtr = std::is_pointer<DataTy>::value>
	// and the specialization for <DataTy,true> has extra function like isNull(), etc.
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