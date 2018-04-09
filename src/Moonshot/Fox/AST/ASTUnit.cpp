////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : ASTUnit.cpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
//			SEE HEADER FILE FOR MORE INFORMATION			
////------------------------------------------------------////

#include "ASTUnit.hpp"

using namespace Moonshot;

void ASTUnit::addDecl(std::unique_ptr<IASTDecl> decl)
{
	decls_.emplace_back(std::move(decl));
}

const IASTDecl * ASTUnit::getDecl(const std::size_t & idx)
{
	if (idx < decls_.size())
		return decls_[idx].get();
	return nullptr;
}

std::size_t ASTUnit::getDeclCount() const
{
	return decls_.size();
}

ASTUnit::decl_iter ASTUnit::decls_beg()
{
	return decls_.begin();
}

ASTUnit::decl_iter ASTUnit::decls_end()
{
	return decls_.end();
}
