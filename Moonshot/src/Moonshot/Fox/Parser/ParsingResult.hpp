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
		SUCCESS, FAILED_AND_DIED, FAILED_BUT_RECOVERED
	};

	template<typename PtrTy>
	struct ParsingResult {
		public:
			ParsingResult(const ParsingOutcome& pc, std::unique_ptr<PtrTy>& node) {
				if (pc == ParsingOutcome::SUCCESS)
				{
					successFlag_ = true;
					recovered_ = true;
				}
				else if (pc == ParsingOutcome::FAILED_AND_DIED)
				{
					successFlag_ = false;
					recovered_ = false;
				}
				else if (pc == ParsingOutcome::FAILED_BUT_RECOVERED)
				{
					if (node)						// if the node is usable and the parser has recovered, that means the parsing function returned a valid, but probably incomplete result.
						successFlag_ = true;		// this result can still be used in the ast. (even if the ast won't be used, this might reduce the number of errors later)
					else
						successFlag_ = true;

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