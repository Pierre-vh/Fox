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
//
//			(*) /!\ There's a bug that makes the recovery unable to match the requested token if it's after a bracket. e.g. if the parser tries to recover to the semi at the end of this : "let x : int[] = [];" it just won't work.
//			review how the resync function works and fix this. I'll probably need to have different "skipToken" (the actual consumeAny) and consumeAny will consume by dispatching to the appropriate function.
//			also, my current for loop is a bit weird, I should double check that.
//		
//			Add better error recovey with common cases support in if/while parsing & function declaration
//
//			Rethink the ParserState system : it works kinda well, but it's a bit verbose to access, isn't it?
//
//			(*) Review the code that manipulates iterator to check that they verify boudaries correctly, and that iterators aren't mishandled anywhere.
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

#include "Moonshot/Fox/Basic/Utils.hpp"

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

			// Consumes an Identifier, Returns nullptr if the Identifier was not found.
			IdentifierInfo* consumeIdentifier();

			// Consumes any sign but brackets. Returns false if the bracket was not found.
			bool consumeSign(const SignType& s);

			// Consumes a bracket and keeps the bracket count up to date. Returns false if the bracket was not found.
			bool consumeBracket(const SignType& s);

			// Consumes a keyword. Returns false if the keyword was not found.
			bool consumeKeyword(const KeywordType& k);

			// Skips 1 token (increments the iterator)
			void consumeAny();		
		
			// Revert the last consume operation (decrements the iterator)
			void revertConsume();	

			// Helper for consumeSign & consumeBracket
			bool isBracket(const SignType& s) const;

			Token getCurtok() const;
			Token getPreviousToken() const;
			
			/*-------------- Error Recovery --------------*/
			bool resyncToSign(const SignType& sign, const bool& stopAtSemi, const bool& shouldConsumeToken);
			bool resyncToSign(const std::vector<SignType>& signs, const bool& stopAtSemi, const bool& shouldConsumeToken);
			bool resyncToNextDecl();

			/*-------------- Error Reporting --------------*/
			void errorUnexpected();
			void errorExpected(const std::string &s);
			void genericError(const std::string &s); 

			// Returns (it == state_.lastUnexpectedTokenIt)
			bool isLastUnexpectedToken(TokenIteratorTy it) const;
			// Sets state_.lastUnexpectedTokenIt
			void markAsLastUnexpectedToken(TokenIteratorTy it);

			/*-------------- Parser State --------------*/
			struct ParserState
			{
				ParserState();

				// The current token
				TokenIteratorTy tokenIterator;

				// The last token that was the target of the "unexpected token" error.
				// This is saved to avoid printing the "unexpected token x" multiple time for the same token.
				TokenIteratorTy lastUnexpectedTokenIt;

				bool isAlive : 1;				// This is set to false when the parser dies (gives up)
				bool isRecoveryAllowed : 1;
			
				// Brackets counters
				uint8_t curlyBracketsCount  = 0;
				uint8_t roundBracketsCount  = 0;
				uint8_t squareBracketsCount	= 0;
			} state_;

			// Interrogate state_
			bool isDone() const;
			bool isAlive() const;
			void die();

			// Backup state_
			ParserState createParserStateBackup() const;
			void restoreParserStateFromBackup(const ParserState& st);


			/*-------------- RAIIRecoveryManager --------------*/
				// This class manages the recovery of the parser
				// The constructor makes a backup of the parser instance's state_.isRecoveryAllowed variable, and replaces state_.isRecoveryAllowed with the value desired.
				// The constructor restores the state_.isRecoveryAllowed variable to it's original value using the backup.
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

			/*-------------- Constants --------------*/
			static constexpr uint8_t kMaxBraceDepth = (std::numeric_limits<uint8_t>::max)();
	};
}