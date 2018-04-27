////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : TestDiagCons.hpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
// A test diagnostic consumer for the tests
////------------------------------------------------------////

#pragma once

#include "Moonshot/Fox/Common/DiagnosticConsumers.hpp"
#include "Moonshot/Fox/Common/Diagnostic.hpp"

#include <string>

namespace Moonshot
{
	class StrDiagConsumer : public DiagnosticConsumer
	{
		public:
			inline virtual void consume(const Diagnostic& diag) override
			{
				str_ = diag.getDiagStr();
			}
			inline std::string getStr() const
			{
				return str_;
			}
		private:
			std::string str_;
	};
}