////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : Parser.hpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
// This file implements the recursive descent parser.		
// The parser is implemented as a set of functions, each	
// "parseXXX" method represents a rule in the grammar.				
//															
// The grammar can be found in	/doc/																		
//
// Status: Up to date with latest grammar changes, but isn't finished yet.
//
//		Potential Areas of improvement
//			Recovery Efficiency
//				> Tweak it by running different test situations and adding special recovery cases wherever needed.
//				> Find flaws in the current system and fix them!
//			Speed
//				> Not for now. I'm going to make it work, make it right, then (maybe) make it fast. 
// 
//		Next modifications planned
//			Add better error recovey with common cases support in if/while parsing & function declaration
//
//			Remove the ParsingResult's functionality of automatically using a unique_ptr when DataTy is a pointer type. This is confusing and makes it impossible to use raw pointers in a parsing result.
//			Instead:
//				Split ParsingResult in UniqueParsingResult and ParsingResult.
//					UniqueParsingResult would have helper functions like "isa" to check if it's unique ptr is dynamic_cast-able to the desired type, and 
//					a getAs which would move the ptr and return it, casted to the desired type.
//
//					Create usings in the Parser for exprs,stmts and decls.
//						using ExprParsingResult = UniqueParsingResult<ASTExpr>
//						using ExprParsingError	= UniqueParsingResult<ASTExpr>(false)
//					TODO:Maybe find better names? Think about it a bit.
//
//			When SourceLoc system is added, match functions should return a SourceLoc instead, and a Invalid sourceloc if it doesn't match anything.
//			SourceLoc will need to overload operator bool(), which will check it's validity.
//				Also, Migrate every diag to the new diag system for the lexer, token & parser classes when SourceLoc is added, and delete the old context diag system.
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
#include <functional>

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
			ParsingResult<ASTDeclRef*> parseDeclCall(); 
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
			// Parsing helpers
			ParsingResult<ASTExpr*>		parseParensExpr(const bool& isMandatory = false);
			ParsingResult<ExprList*>	parseExprList();
			ParsingResult<ExprList*>	parseParensExprList();

			ParsingResult<ASTArgDecl*>	parseArgDecl();

			const Type* parseBuiltinTypename();									// Returns a nullptr if no type keyword is found
			std::pair<const Type*,bool> parseType();							// first -> The Type* (nullptr if not found), second -> False if error
			ParsingResult<QualType>		parseFQTypeSpec();

			ParsingResult<binaryOperator> parseAssignOp();						// = 
			ParsingResult<unaryOperator>  parseUnaryOp();						// ! - +
			ParsingResult<binaryOperator> parseBinaryOp(const char &priority);	// + - * / % 
			bool parseExponentOp();												//  **

			/*
				Match methods :
					Match methods are designed to match a single keyword/sign. If they find it, they consume it and return true, if they don't, they return false;
			*/

			// Match an Identifier, returns nullptr if not found.
			IdentifierInfo* matchID();					
			// Matches any sign but brackets.
			bool matchSign(const SignType& s);		
			// Matches a bracket and keeps the bracket count up to date.
			bool matchBracket(const SignType& s);	
			// Helper for matchSign & matchBracket
			bool isBracket(const SignType& s) const;
			// Matches a single keyword.
			bool matchKeyword(const KeywordType& k);		


			Token getToken() const;
			Token getToken(const std::size_t &d) const;

			void consumeToken();						// Increments parserState_.pos
			void setPosition(const std::size_t &pos);	// Sets parserState_.pos
			void revertConsume();						// Decrement parserState_.pos

			
			// "Panic" methods to resync to a specific sign, or a sign in a set of signs
			bool resyncToSign(const SignType& sign, const bool& stopAtSemi, const bool& shouldConsumeToken);
			bool resyncToSign(const std::vector<SignType>& signs, const bool& stopAtSemi, const bool& shouldConsumeToken);
			// Methods to resync to the next declaration.
			bool resyncToNextDecl();

			// Indicates that the parsing is over.
			void die();

			void errorUnexpected();
			void errorExpected(const std::string &s);
			void genericError(const std::string &s); 

			// error member variables
			std::size_t lastUnexpectedTokenPosition_;
			
			struct ParserState
			{
				ParserState();

				std::size_t pos = 0;						// current pos in the Token vector.
				bool isAlive : 1;							// is the parser "alive"?
				bool isRecoveryAllowed : 1;
			
				uint8_t curlyBracketsCount  = 0;
				uint8_t roundBracketsCount  = 0;
				uint8_t squareBracketsCount	= 0;
			} parserState_;

			// Interrogate parserState_
			bool hasReachedEndOfTokenStream() const;
			bool isAlive() const;
			std::size_t getCurrentPosition() const;

			// Backup parserState_
			ParserState createParserStateBackup() const;
			void restoreParserStateFromBackup(const ParserState& st);

			// This class manages the recovery of the parser
				// The constructor makes a backup of the parser instance's parserState_.isRecoveryAllowed variable, and replaces parserState_.isRecoveryAllowed with the value desired.
				// The constructor restores the parserState_.isRecoveryAllowed variable to it's original value using the backup.
			class RAIIRecoveryManager
			{
				public:
					explicit RAIIRecoveryManager(Parser &parser,const bool& allowsRecovery);
					~RAIIRecoveryManager();
				private:
					Parser &parser_;
					bool recoveryAllowedBackup_ : 1;
			};

			RAIIRecoveryManager createRecoveryEnabler();
			RAIIRecoveryManager createRecoveryDisabler();


			ASTContext& astcontext_;
			Context& context_;
			TokenVector& tokens_;	

			// constants
			static constexpr uint8_t kMaxBraceDepth = (std::numeric_limits<uint8_t>::max)();
	};
}