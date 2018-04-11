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

/*
	QUICK NOTE: State of the parser error recovery system as of April 2018
		At top level (function/variable declarations at file level), the parser only tries to recover to the next declaration it can find if something went wrong.
		The parsing function themselves are not allowed to attempt recovery. Upon error, they return the data if possible with a flag "FAILED_WITHOUT_ATTEMPTING_RECOVERY"

		At "local" level (inside a function declaration and inside the function declaration's inner scopes), Compound statements and statements will attempt to recover to the next FREE ';', ')' or '}'
		they can find. If they can't find one, the parsing ends. Note : FREE = for }, ) and ], the recovery function ignore matches if it found an opening {, ( or [  earlier. If you read the code of 
		resyncToSign at line ~148 you will understand what I mean.

		There's still a lot of work to do to get really good error messages, but currently the parser tries to give you decent error messages, and they should help you
		find the error in the most common error cases. Fox's syntax isn't hard, so there should be really no complex case whatsoever.
		If someone reads this and wants to contribute, you can try to find a better way of recovering errors that would produce less error cascades and would suit fox well.
		If you find such a thing, don't hesitate to make a PR (if you implemented it), but try to contact me at pierre.vanhoutryve@gmail.com first to discuss the matter a bit !

*/

#pragma once

#include "Moonshot/Common/Types/Types.hpp"
#include "Moonshot/Fox/Lexer/Token.hpp"					
#include "Moonshot/Fox/Parser/ParsingResult.hpp"

#include "Moonshot/Fox/AST/ASTDecl.hpp"
#include "Moonshot/Fox/AST/ASTExpr.hpp"
#include "Moonshot/Fox/AST/ASTStmt.hpp"
#include "Moonshot/Fox/AST/ASTUnit.hpp"

#include "Moonshot/Fox/AST/Operators.hpp"			

namespace Moonshot
{
	class Context;
	class Parser
	{
		public:
			Parser(Context& c,TokenVector& l);

			// UNIT
			ParsingResult<ASTUnit*>	parseUnit();

			// EXPRESSIONS
			ParsingResult<IASTDeclRef*> parseArrayAccess(std::unique_ptr<IASTDeclRef> base);
			ParsingResult<IASTDeclRef*> parseDeclCall(); 
			ParsingResult<IASTExpr*> parseLiteral();
			ParsingResult<IASTExpr*> parsePrimary();
			ParsingResult<IASTExpr*> parseMemberAccess();
			ParsingResult<IASTExpr*> parseExponentExpr();
			ParsingResult<IASTExpr*> parsePrefixExpr(); 
			ParsingResult<IASTExpr*> parseCastExpr();
			ParsingResult<IASTExpr*> parseBinaryExpr(const char &priority = 5);
			ParsingResult<IASTExpr*> parseExpr(); 

			// STATEMENTS
			ParsingResult<IASTStmt*> parseReturnStmt();
			ParsingResult<IASTStmt*> parseExprStmt(); // Expression statement
			ParsingResult<ASTCompoundStmt*> parseTopLevelCompoundStatement(const bool& isMandatory = false); // Top level compound statement : always mandatory and doesn't try to recover on error, since error handling is done by parseUnit
			ParsingResult<ASTCompoundStmt*> parseCompoundStatement(const bool& isMandatory=false); // Compound Statement, might be mandatory, and tries to recover to next } on error.
			ParsingResult<IASTStmt*> parseStmt(); // General Statement
			ParsingResult<IASTStmt*> parseBody(); // body for control flow

			// STATEMENTS : CONDITION & LOOPS
			ParsingResult<IASTStmt*> parseCondition(); // Parse a  if-else if-else "block
			ParsingResult<IASTStmt*> parseWhileLoop();

			// DECLS
			ParsingResult<ASTVarDecl*> parseTopLevelVarDeclStmt();	// Parses a var declaration, but doesn't attempt to recover when an error is met
			ParsingResult<ASTVarDecl*> parseVarDeclStmt();			// Same as above, but tries to recover.
			ParsingResult<ASTFunctionDecl*> parseFunctionDeclaration();
			ParsingResult<IASTDecl*> parseTopLevelDecl();

		private:
			// expression helpers
			ParsingResult<IASTExpr*> parseParensExpr(const bool& isMandatory = false, const bool& isExprMandatory = false);
			ParsingResult<ExprList*> parseExprList();
			ParsingResult<ExprList*> parseParensExprList();
			// arg decl for functions
			ParsingResult<FoxFunctionArg> parseArgDecl();
			ParsingResult<std::vector<FoxFunctionArg>> parseArgDeclList();
			// type spec for vardecl
			ParsingResult<FoxType> parseTypeSpec();

			// OneUpNode is a function that ups the node one level.
			// Example: There is a node N, with A and B as children. You call oneUpNode like this : oneUpNode(N,PLUS)
			// oneUpNode will return a new node X, with the operation PLUS and N as left child.
			std::unique_ptr<ASTBinaryExpr> oneUpNode(std::unique_ptr<ASTBinaryExpr> node, const binaryOperator &op = binaryOperator::DEFAULT);
			
			// matchToken -> returns true if the Token is matched, and increment pos_, if the Token isn't matched return false
			// Peek != Match, Peek tries to match a token, if it does, it returns true and DOES NOT increment the position. Match does the same but increments if found.
			// Match
			ParsingResult<LiteralInfo> matchLiteral();			// match a literal
			ParsingResult<std::string> matchID();			// match a ID
			bool matchSign(const SignType& s);				// match any signs : ; . ( ) , returns true if success
			bool matchKeyword(const KeywordType& k);		// Match any keyword, returns true if success

			// Peek : peek at index
			bool peekSign(const std::size_t &idx, const SignType &sign) const;

			ParsingResult<std::size_t> matchTypeKw();		// match a type keyword : int, float, etc. and returns its index in the FValue
			
			// MATCH OPERATORS
			bool							matchExponentOp(); //  **
			ParsingResult<binaryOperator>	matchAssignOp(); // = 
			ParsingResult<unaryOperator>	matchUnaryOp(); // ! - +
			ParsingResult<binaryOperator>	matchBinaryOp(const char &priority); // + - * / % 
			
			// UTILITY METHODS
			// GetToken
			Token getToken() const;
			Token getToken(const std::size_t &d) const;

			// Get state_.pos
			std::size_t getCurrentPosition() const;
			// Get state_.pos++
			std::size_t getNextPosition() const;
			// Increment state_.pos
			void incrementPosition();
			// Decrement state_.pos
			void decrementPosition();

			/*
				Note, this could use improvements, for instance a maximum thresold, or stop when a '}' is found to avoid matching to a semicolon out of the compound statement, etc.
				This is a matter for another time, first I want to finish the interpreter up to v1.0, then i'll do a refactor to give better error messages before moving on to other features (arrays, oop, tuples)
			*/
			// This function will skip every token until the appropriate "resync" token is found.
			// Returns true if resync was successful.
			bool resyncToSign(const SignType &s);
			bool isClosingDelimiter(const SignType &s) const;			// Returns true if s is a } or ) or ]
			SignType getOppositeDelimiter(const SignType &s);			// Returns [ for ], { for }, ( for )
			// Same as resyncToSign, except it works on "let" and "func" keywords
			bool resyncToNextDeclKeyword();

			// die : Indicates that the parsing is over and the parser has died because of a critical error. 
			void die();

			// Typical parser error message functions:
			// "Unexpected token x"
			void errorUnexpected();
			// "Expected x after y"
			void errorExpected(const std::string &s);
			// Any error
			void genericError(const std::string &s); 
			
			struct ParserState
			{
				std::size_t pos = 0;						// current pos in the Token vector.
				bool isAlive = true;						// is the parser "alive"?
			} state_;

			// Interrogate parser state
			// Returns true if pos >= tokens_.size()
			bool hasReachedEndOfTokenStream() const;

			// Returns isAlive
			bool isAlive() const;

			ParserState createParserStateBackup() const;
			void restoreParserStateFromBackup(const ParserState& st);

			// Member variables
			Context& context_;
			TokenVector& tokens_;					// reference to the the token's vector.
	};
}