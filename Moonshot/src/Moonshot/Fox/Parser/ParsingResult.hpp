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
					successFlag_ = isNodeUsable();
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

			bool isNodeUsable() const
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