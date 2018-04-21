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
// Note :	I've dug a lot into CLang's parser to try and see how to handle complex cases with elegance,
//			so some stuff you'll see here looks a lot like clang's parser, simplified of course.
// 
// Status: Up to date with latest grammar changes, except import/using rules that aren't implemented yet.
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
//			Remove match methods, and instead use consume methods that work everywhere. There's just no need to split token consumeToken and match functions.
//			just add a consumeAny() to skip a token.
//
//			Remove the ParseRes's functionality of automatically using a unique_ptr when DataTy is a pointer type. This is confusing and makes it impossible to use raw pointers in a parsing result.
//			Instead, create a "UniqueParseRes" class that holds it's data as a unique_ptr.
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
			ParseRes<ASTExpr*>		parseSuffix(std::unique_ptr<ASTExpr> &base);
			ParseRes<ASTDeclRef*>	parseDeclCall(); 
			ParseRes<ASTExpr*> parsePrimitiveLiteral();
			ParseRes<ASTExpr*> parseArrayLiteral();
			ParseRes<ASTExpr*> parseLiteral();
			ParseRes<ASTExpr*> parsePrimary();
			ParseRes<ASTExpr*> parseArrayOrMemberAccess();
			ParseRes<ASTExpr*> parseExponentExpr();
			ParseRes<ASTExpr*> parsePrefixExpr(); 
			ParseRes<ASTExpr*> parseCastExpr();
			ParseRes<ASTExpr*> parseBinaryExpr(const char &priority = 5);
			ParseRes<ASTExpr*> parseExpr(); 

			// STATEMENTS
			ParseRes<ASTStmt*> parseReturnStmt();
			ParseRes<ASTStmt*> parseExprStmt(); 
			ParseRes<ASTCompoundStmt*> parseCompoundStatement(const bool& isMandatory=false); 
			ParseRes<ASTStmt*> parseStmt();
			ParseRes<ASTStmt*> parseBody();
			ParseRes<ASTStmt*> parseCondition();
			ParseRes<ASTStmt*> parseWhileLoop();

			// DECLS
			ParseRes<ASTVarDecl*>		parseVarDeclStmt();
			ParseRes<ASTFunctionDecl*> parseFunctionDeclaration();
			ParseRes<ASTDecl*>			parseDecl();

		private:
			// Parsing helpers
			ParseRes<ASTExpr*>		parseParensExpr(const bool& isMandatory = false);
			ParseRes<ExprList*>	parseExprList();
			ParseRes<ExprList*>	parseParensExprList();

			ParseRes<ASTArgDecl*>	parseArgDecl();

			const Type* parseBuiltinTypename();									// Returns a nullptr if no type keyword is found
			std::pair<const Type*,bool> parseType();							// first -> The Type* (nullptr if not found), second -> False if error
			ParseRes<QualType>		parseFQTypeSpec();

			ParseRes<binaryOperator> parseAssignOp();						// = 
			ParseRes<unaryOperator>  parseUnaryOp();						// ! - +
			ParseRes<binaryOperator> parseBinaryOp(const char &priority);	// + - * / % 
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