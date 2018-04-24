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

void ASTUnit::addDecl(std::unique_ptr<ASTDecl> decl)
{
	decls_.emplace_back(std::move(decl));
}

const ASTDecl * ASTUnit::getDecl(const std::size_t & idx)
{
	if (idx < decls_.size())
		return decls_[idx].get();
	return nullptr;
}

std::size_t ASTUnit::getDeclCount() const
{
	return decls_.size();
}

ASTUnit::DeclVecIter ASTUnit::decls_beg()
{
	return decls_.begin();
}

ASTUnit::DeclVecIter ASTUnit::decls_end()
{
	return decls_.end();
}

ASTUnit::DeclVecConstIter ASTUnit::decls_beg() const
{
	return decls_.begin();
}

ASTUnit::DeclVecConstIter ASTUnit::decls_end() const
{
	return decls_.end();
}