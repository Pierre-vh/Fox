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
//		Parser "to-do" list. Important stuff is marked with (*)
//			Add better error recovey with common cases support in if/while parsing & function declaration
//
//			(*) Review a bit the ParserState system, because right now accessing anything in it is pretty verbose. Maybe drop the trailing _ and just use "state" as the variable name?
//
//			(*) Review the code that manipulates iterator to check that they verify boudaries correctly. Also, add a end_ and begin_ member variable with tokens_.begin() and tokens_.end(), and
//			make a generic iteratorIncrement and iteratorDecrement function in Utils.hpp, that decrements the iterator while checking that it doesn't drop below begin/above end.
//			
//			(*) Remove the ParseRes's functionality of automatically using a unique_ptr when DataTy is a pointer type. This is confusing and makes it impossible to use raw pointers in a parsing result.
//			Instead, create a "UniqueParseRes" class that holds it's data as a unique_ptr. However, typing "UniqueParseRes<ASTExpr>" each time is really long. Maybe cut it down using "usings" or typedefs?
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
		private:
			using TokenIteratorTy = TokenVector::iterator;
		public:
			Parser(Context& c,ASTContext& astctxt,TokenVector& l);

			/*-------------- Parsing Methods --------------*/
			// UNIT
			UnitParsingResult parseUnit();

			// EXPRESSIONS
			ParseRes<ExprList*>	parseExprList();
			ParseRes<ExprList*>	parseParensExprList();
			ParseRes<ASTExpr*>	parseParensExpr(const bool& isMandatory = false);
			ParseRes<ASTExpr*>	parseSuffix(std::unique_ptr<ASTExpr> &base);
			ParseRes<ASTDeclRef*> parseDeclCall(); 
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
			ParseRes<ASTArgDecl*> parseArgDecl();
			ParseRes<ASTVarDecl*> parseVarDeclStmt();
			ParseRes<ASTFunctionDecl*> parseFunctionDeclaration();
			ParseRes<ASTDecl*> parseDecl();

		private:
			/*-------------- Parser Setup --------------*/
			void setupParser();

			/*-------------- "Basic" Parse Methods --------------*/
			// Returns a nullptr if no type keyword is found
			const Type* parseBuiltinTypename();	

			// first -> The Type* (nullptr if not found), second -> False if error
			std::pair<const Type*,bool> parseType();

			// Parses a QualType (Full Type Spec)
			ParseRes<QualType>	parseFQTypeSpec();

			ParseRes<binaryOperator> parseAssignOp();						// = 
			ParseRes<unaryOperator>  parseUnaryOp();						// ! - +
			ParseRes<binaryOperator> parseBinaryOp(const char &priority);	// + - * / % 
			bool parseExponentOp();											//  **

			/*-------------- Token Consuming --------------*/
			/*	
				Consume methods all return a boolean if the "consume" operation finished successfully 
				(found the requested token), false otherwise
			*/

			// Match an Identifier, (returns nullptr if not found)	
			IdentifierInfo* consumeIdentifier();

			// Matches any sign but brackets.
			bool consumeSign(const SignType& s);

			// Matches a bracket and keeps the bracket count up to date.
			bool consumeBracket(const SignType& s);

			// Matches a keyword.
			bool consumeKeyword(const KeywordType& k);

			// increases the iterator by n, effectively "skipping" token
			void consumeAny(char n = 1);		
		
			// decreases the iterator by n
			void revertConsume(char n = 1);	

			// Helper for consumeSign & consumeBracket
			bool isBracket(const SignType& s) const;

			Token& getCurtok();
			
			/*-------------- Error Recovery --------------*/
			bool resyncToSign(const SignType& sign, const bool& stopAtSemi, const bool& shouldConsumeToken);
			bool resyncToSign(const std::vector<SignType>& signs, const bool& stopAtSemi, const bool& shouldConsumeToken);
			bool resyncToNextDecl();

			/*-------------- Error Reporting --------------*/
			void errorUnexpected();
			void errorExpected(const std::string &s);
			void genericError(const std::string &s); 

			// This variable keeps track of the latest token that was the target of "errorUnexpected" to 
			// avoid printing multiple "unexpected" errors for the same token.
			TokenIteratorTy lastUnexpectedTokenIt_;

			/*-------------- Parser State --------------*/
			struct ParserState
			{
				ParserState();

				TokenIteratorTy tokenIterator;
				bool isAlive : 1;
				bool isRecoveryAllowed : 1;
			
				uint8_t curlyBracketsCount  = 0;
				uint8_t roundBracketsCount  = 0;
				uint8_t squareBracketsCount	= 0;
			} parserState_;

			// Interrogate parserState_
			bool isDone() const;
			bool isAlive() const;
			void die();

			// Backup parserState_
			ParserState createParserStateBackup() const;
			void restoreParserStateFromBackup(const ParserState& st);


			/*-------------- RAIIRecoveryManager --------------*/
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

			/*-------------- Member Variables --------------*/
			ASTContext& astcontext_;
			Context& context_;
			TokenVector& tokens_;
			Token nullTok_;			// null tok is an empty token used by getToken() to return an empty/null token reference.

			/*-------------- Constants --------------*/
			static constexpr uint8_t kMaxBraceDepth = (std::numeric_limits<uint8_t>::max)();
	};
}