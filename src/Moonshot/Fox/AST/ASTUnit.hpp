////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : ASTUnit.hpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
// This file declares the ASTUnit class.
// The ASTUnit represents one source file.
////------------------------------------------------------////
#pragma once

#include "ASTDecl.hpp"

#include <vector>
#include <memory>

namespace Moonshot
{
	class ASTUnit
	{
		private:
			using decl_iter = std::vector<std::unique_ptr<ASTDecl>>::iterator;
		public:
			ASTUnit() = default;

			// TODO : Replace that with a DeclContext when it's done.
			void addDecl(std::unique_ptr<ASTDecl> decl);
			const ASTDecl* getDecl(const std::size_t &idx);
			std::size_t getDeclCount() const;

			decl_iter decls_beg();
			decl_iter decls_end();
		private:
			// The decls contained within this unit.
			std::vector<std::unique_ptr<ASTDecl>> decls_;
	};
}