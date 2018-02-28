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
			* Parsing function finds the first tokens, but encounters an unexpected token and dies without recovering: 
				use FAILED_AND_DIED flag
				hasRecovered() returns true
				wasSuccessful() returns false
			* Parsing function finds the first tokens, but encounters an unexpected token and successfully recovers to the semicolon,parens or curly bracket.
				use FAILED_AND_RECOVERED flag
				hasRecovered() returns true
				wasSuccessful() returns false
	*/
	template<typename PtrTy>
	struct ParsingResult {
		public:
			ParsingResult(const ParsingOutcome& pc, std::unique_ptr<PtrTy> node = nullptr)
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
					successFlag_ = isNodeNull();
					recovered_ = true;
				}

				if (node)
					node_ = std::move(node);
				else
					node_ = nullptr;
			}

			operator bool() const
			{
				return wasSuccessful() && node_;
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

			bool isNodeNull() const
			{
				if (node_)
					return true;
				return false;
			}

			// The node returned by the parsing function
			std::unique_ptr<PtrTy> node_ = nullptr;
		private:
			ParsingOutcome enumflag_;
			bool successFlag_ = false,recovered_ = true;
	};
}