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
	/*
		Usage
			* Parsing function finds all the tokens and return a fully formed node:
			* Parsing function doesn't find the first token and just returns nullptr 
				use SUCCESS flag
				hasRecovered() returns true (but it's meaningless, you usually won't try to check it if the parsing was Successful)
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
			enum class Outcome {
				SUCCESS, FAILED_AND_DIED, FAILED_BUT_RECOVERED
			};

			ParsingResult(const Outcome& pc, std::unique_ptr<PtrTy> node) {
				if (pc == Outcome::SUCCESS)
				{
					successFlag_ = true;
					recovered_ = true;
				}
				else if (pc == Outcome::FAILED_AND_DIED)
				{
					successFlag_ = false;
					recovered_ = false;
				}
				else if (pc == Outcome::FAILED_BUT_RECOVERED)
				{
					successFlag_ = isNodeNull();
					recovered_ = true;
				}

				if (node)
					node_ = std::move(node);
				else
					node_ = nullptr;
			}

			operator bool() const {
				return wasSuccessful();
			}

			bool wasSuccessful() const
			{
				return isvalid_;
			}

			bool hasRecovered() const {
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
			bool successFlag_ = false,recovered_ = true;
	};
}