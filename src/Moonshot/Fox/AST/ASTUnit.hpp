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
#include "DeclRecorder.hpp"
#include <vector>
#include <memory>

namespace Moonshot
{
	class ASTUnit : public DeclRecorder
	{
		private:
			using DelVecTy = std::vector<std::unique_ptr<ASTDecl>>;
			using DeclVecIter	= DelVecTy::iterator;
			using DeclVecConstIter = DelVecTy::const_iterator;
		public:
			ASTUnit() = default;

			void addDecl(std::unique_ptr<ASTDecl> decl);
			const ASTDecl* getDecl(const std::size_t &idx);
			std::size_t getDeclCount() const;

			DeclVecIter decls_beg();
			DeclVecIter decls_end();

			DeclVecConstIter decls_beg() const;
			DeclVecConstIter decls_end() const;

		private:
			// The decls contained within this unit.
			DelVecTy decls_;
	};
}