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
		SUCCESS, FAILURE
	};

	enum class ParserStatus {
		RECOVERED, DEAD
	};
	template<typename PtrTy>
	struct ParsingResult {
	public:
		ParsingResult(const ParsingOutcome& pc, const ParserStatus& ps, std::unique_ptr<PtrTy>& node) {
			if (pc == ParsingOutcome::SUCCESS)
				isvalid_ = true;
			else if (pc == ParsingOutcome::FAILURE)
				isvalid_ = false;

			if (ps == ParserStatus::RECOVERED)
				isparseralive_ = true;
			else if (ps == ParserStatus::DEAD)
				isparseralive_ = false;

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
			return isparseralive_;
		}

		bool isNodeUsable() const
		{
			if (node_)
				return true;
			return false;
		}

		std::unique_ptr<PtrTy> node_ = nullptr;


	private:
		bool isvalid_;
		bool isparseralive_;
	};
}