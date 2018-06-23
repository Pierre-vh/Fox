///------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : Drivers.cpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
////------------------------------------------------------////

#include "Utils.hpp"

using namespace Moonshot;
using namespace Moonshot::Tests;

ParserPreparator::ParserPreparator(const std::string& filepath) 
{
	loadedFileID_ = ctxt_.sourceManager.loadFromFile(Tests::convertRelativeTestResPathToAbsolute(filepath));

	// If file couldn't be loaded, give us the reason
	if (!loadedFileID_)
	{
		success_ = false;
		errorReason_ = "Couldn't load file \"" + filepath + "\" in memory.";
		return;
	}

	lexer_ = std::make_unique<Lexer>(ctxt_, ast_);
	lexer_->lexFile(loadedFileID_);

	if (!ctxt_.isSafe())
	{
		success_ = false;
		errorReason_ = "Lexing Error";
		return;
	}

	parser_ = std::make_unique<Parser>(ctxt_, ast_, lexer_->getTokenVector(), &dr_);
	success_ = true;
}

bool ParserPreparator::hasSetupCompletedSuccessfuly() const
{
	return success_;
}

Context& ParserPreparator::getContext()
{
	return ctxt_;
}

ASTContext& ParserPreparator::getASTContext()
{
	return ast_;
}

DeclRecorder& ParserPreparator::getDeclRecorder()
{
	return dr_;
}

SourceManager& ParserPreparator::getSourceManager()
{
	return ctxt_.sourceManager;
}

Parser& ParserPreparator::getParser()
{
	assert(parser_ && "Parser doesn't exist");
	return *parser_;
}

Lexer& ParserPreparator::getLexer()
{
	assert(lexer_ && "lexer doesn't exist");
	return *lexer_;
}

std::string ParserPreparator::getLastErrorMsg() const
{
	return errorReason_;
}

FileID ParserPreparator::getFileID() const
{
	return loadedFileID_;
}
