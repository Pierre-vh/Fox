////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : CompoundStatements.hpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
// Test class for compound statements.
// Uses test files located in test/compstmt/
////------------------------------------------------------////

#pragma once

#include "../ITest.hpp"

namespace Moonshot
{
	class CompoundStatements : public ITest
	{
		public:
			CompoundStatements();
			~CompoundStatements();

			virtual std::string getTestName() const override;
			virtual bool runTest(Context & context) override;

		private:
			bool parseFile_compoundStatement(Context& context, const std::string& str) const;
	};
}

