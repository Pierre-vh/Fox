///------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : Utils.hpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
// This file declares various utilities (like Drivers) for
// tests located in this directory.
////------------------------------------------------------////

#include "Support/TestUtils.hpp"
#include "Moonshot/Fox/Parser/Parser.hpp"
#include "Moonshot/Fox/Lexer/Lexer.hpp"
#include "Moonshot/Fox/AST/Decl.hpp"
#include "Moonshot/Fox/Common/Context.hpp"
#include "Moonshot/Fox/Common/SourceManager.hpp"

#include <memory>

namespace Moonshot
{
	// ParserPreparator
	// This prepares a Parser instance for use.
	// This is to be used with only one file.
	class ParserPreparator
	{
		public:
			ParserPreparator(const std::string& filepath);

			bool hasSetupCompletedSuccessfuly() const;

			Context& getContext();
			ASTContext& getASTContext();
			DeclRecorder& getDeclRecorder();
			SourceManager& getSourceManager();

			Parser& getParser();
			Lexer& getLexer();

			FileID getFileID() const;

			std::string getLastErrorMsg() const;
		private:
			bool success_;
			std::string fullFilePath_;
			std::string errorReason_;

			FileID loadedFileID_;
			Context ctxt_;
			ASTContext ast_;
			DeclRecorder dr_;
			std::unique_ptr<Lexer> lexer_;
			std::unique_ptr<Parser> parser_;
	};
}