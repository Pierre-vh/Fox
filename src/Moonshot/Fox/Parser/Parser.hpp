////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : Parser.hpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
// This file implements the recursive descent parser.		
// The parser is implemented as a set of functions, each	
// function represents a rule in the grammar.				
// Some extra functions, for instance matchXXX	are used to help in the parsing process.					
//															
// The grammar used can be found in	/doc/grammar_(major).(minor).txt							
//															
// The lexer is the second step of the interpretation process:
// Lexer -> [PARSER] -> ...									
//															
// INPUT													
// It uses the data gathered and identified by the lexer to build a representation of the source file (AST.)		
//															
// OUTPUT													
// The Abstract Syntax Tree, AST for short.				
//
// Status: Up to date with latest grammar changes, but isn't finished yet.
////------------------------------------------------------////

#pragma once

#include "Moonshot/Fox/Lexer/Token.hpp"					
#include "Moonshot/Fox/Parser/ParsingResult.hpp"

#include "Moonshot/Fox/AST/ASTContext.hpp"
#include "Moonshot/Fox/AST/Types.hpp"
#include "Moonshot/Fox/AST/ASTDecl.hpp"
#include "Moonshot/Fox/AST/ASTExpr.hpp"
#include "Moonshot/Fox/AST/ASTStmt.hpp"
#include "Moonshot/Fox/AST/ASTUnit.hpp"

#include "Moonshot/Fox/AST/Operators.hpp"			

#include <tuple>

namespace Moonshot
{
	class Context;
	class Parser
	{
		public:
			Parser(Context& c,ASTContext& astctxt,TokenVector& l);

			// UNIT
			UnitParsingResult parseUnit();

			// EXPRESSIONS
			ParsingResult<ASTExpr*>		parseSuffix(std::unique_ptr<ASTExpr> &base);
			ParsingResult<IASTDeclRef*> parseDeclCall(); 
			ParsingResult<ASTExpr*> parsePrimitiveLiteral();
			ParsingResult<ASTExpr*> parseArrayLiteral();
			ParsingResult<ASTExpr*> parseLiteral();
			ParsingResult<ASTExpr*> parsePrimary();
			ParsingResult<ASTExpr*> parseArrayOrMemberAccess();
			ParsingResult<ASTExpr*> parseExponentExpr();
			ParsingResult<ASTExpr*> parsePrefixExpr(); 
			ParsingResult<ASTExpr*> parseCastExpr();
			ParsingResult<ASTExpr*> parseBinaryExpr(const char &priority = 5);
			ParsingResult<ASTExpr*> parseExpr(); 

			// STATEMENTS
			ParsingResult<ASTStmt*> parseReturnStmt();
			ParsingResult<ASTStmt*> parseExprStmt(); 
			ParsingResult<ASTCompoundStmt*> parseCompoundStatement(const bool& isMandatory=false); 
			ParsingResult<ASTStmt*> parseStmt();
			ParsingResult<ASTStmt*> parseBody();
			ParsingResult<ASTStmt*> parseCondition();
			ParsingResult<ASTStmt*> parseWhileLoop();

			// DECLS
			ParsingResult<ASTVarDecl*>		parseVarDeclStmt();
			ParsingResult<ASTFunctionDecl*> parseFunctionDeclaration();
			ParsingResult<ASTDecl*>			parseDecl();

		private:
			ParsingResult<ASTExpr*>		parseParensExpr(const bool& isMandatory = false);
			ParsingResult<ExprList*>	parseExprList();
			ParsingResult<ExprList*>	parseParensExprList();
			ParsingResult<ASTArgDecl*>	parseArgDecl();
			ParsingResult<QualType>		parseFQTypeSpec();

			// Returns a nullptr if no type keyword is found
			const Type* parseBuiltinTypename();
			// first -> The Type* (nullptr if not found), second -> False if error
			std::pair<const Type*,bool> parseType(); 
			
			IdentifierInfo* matchID();					// Returns nullptr if not found
			bool matchSign(const SignType& s);				
			bool matchKeyword(const KeywordType& k);		

			ParsingResult<binaryOperator> matchAssignOp();						// = 
			ParsingResult<unaryOperator>  matchUnaryOp();						// ! - +
			ParsingResult<binaryOperator> matchBinaryOp(const char &priority);	// + - * / % 
			bool matchExponentOp(); //  **

			Token getToken() const;
			Token getToken(const std::size_t &d) const;

			// Increments parserState_.pos
			void consumeToken();
			// Sets parserState_.pos
			void setPosition(const std::size_t &pos);
			// Decrement parserState_.pos
			void revertConsume();

			// 3 Types of resync functions and all 3 won't work if parser isn't allowed to recover.
				// Skips every token until the sign s,a semicolon, a free }, eof or a token marking the beginning of a statement is found.
			ResyncResult resyncToSignInStatement(const SignType &s,const bool& shouldConsumeToken = true);
				//Skips every token until the sign s, a free }, "func" or eof is found
			ResyncResult resyncToSignInFunction(const SignType &s, const bool& shouldConsumeToken = true);
			ResyncResult resyncToNextDeclKeyword();
			// helpers for resync funcs
			bool isBeginningOfStatementKeyword(const KeywordType& kw);	// Returns true if kw is on of "let", "if", "else", "while", "return" or "func"
			bool isClosingDelimiter(const SignType &s) const;			// Returns true if s is a } or ) or ]
			SignType getOppositeDelimiter(const SignType &s);			// Returns [ for ], { for }, ( for )

			// die : Indicates that the parsing is over.
			void die();

			// Error helpers
			void errorUnexpected();
			void errorExpected(const std::string &s);
			void genericError(const std::string &s); 
			
			struct ParserState
			{
				ParserState();

				std::size_t pos = 0;						// current pos in the Token vector.
				bool isAlive : 1;							// is the parser "alive"?
				bool isRecoveryAllowed : 1;
			} parserState_;

			// Interrogate parserState_
			bool hasReachedEndOfTokenStream() const;
			bool isAlive() const;
			std::size_t getCurrentPosition() const;

			// Backup parserState_
			ParserState createParserStateBackup() const;
			void restoreParserStateFromBackup(const ParserState& st);

			// This class manages the recovery of the parser using a RAII idiom.
				// The constructor makes a backup of the parser instance's parserState_.isRecoveryAllowed variable, and replaces parserState_.isRecoveryAllowed with the value desired.
				// The constructor restores the parserState_.isRecoveryAllowed variable to it's original value using the backup.
			class RAIIRecoveryManager
			{
				public:
					explicit RAIIRecoveryManager(Parser *parser,const bool& allowsRecovery);
					~RAIIRecoveryManager();
				private:
					Parser *parser_;
					bool recoveryAllowedBackup_ : 1;
			};

			RAIIRecoveryManager createRecoveryEnabler();
			RAIIRecoveryManager createRecoveryDisabler();

			ASTContext& astcontext_;
			Context& context_;
			TokenVector& tokens_;	
	};
}