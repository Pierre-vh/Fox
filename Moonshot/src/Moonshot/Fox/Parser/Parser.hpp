////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : Parser.hpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
// This file implements the recursive descent parser.		
// The parser is implemented as a set of functions, each	
// function represents a rule in the parser.				
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
////------------------------------------------------------////

/*
Note :
	matchXXX functions : Parse a NON-TERMINAL.
		Theses function DON'T update the cursor BUT the cursor will be updated if the nonterminal is found, because the parseXXX function called will update it.
	parseXXX functions : Parse a TERMINAL
		Theses functions UPDATE the cursor.

	HOW TO IMPLEMENT A GRAMMAR RULE:
		1 - Nonterminal rules
			Case a : Matched all Token 
				Return a valid pointer to the node. Cursor is updated through match function
			Case b : Matched no Token
				 return a null pointer, cursor isn't updated
			Case c : matched one or more Token, but encountered an unexpected Token
				Don't update the cursor, return a null pointer, throw an error (with the context)
		2 - Terminal rules
			If the current Token is the requested Token, update the cursor.

*/
#pragma once

// Tokens
#include "Moonshot/Fox/Lexer/Token.hpp"					
// include ParsingResult class
#include "Moonshot/Fox/Parser/ParsingResult.hpp"
// Include interfaces so the users of this class can manipulate (also reduces the number of fwddecl required)
#include "Moonshot/Fox/AST/Nodes/IASTExpr.hpp"
#include "Moonshot/Fox/AST/Nodes/IASTStmt.hpp"
#include "Moonshot/Fox/AST/Nodes/ForwardDeclarations.hpp"
#include "Moonshot/Fox/Common/Operators.hpp"			

#include <tuple>							// std::tuple, std::pair
#include <memory>							// std::shared_ptr
#include <stack>							// std::stack

#define DEFAULT__shouldPrintSuggestions true

namespace Moonshot
{
	class Context;
	class Parser
	{
		public:
			Parser(Context& c,TokenVector& l);
			~Parser();

			// EXPRESSIONS
			ParsingResult<IASTExpr> parseCallable(); // values/functions calls.
			ParsingResult<IASTExpr> parseValue();
			ParsingResult<IASTExpr> parseExponentExpr();
			ParsingResult<IASTExpr> parsePrefixExpr(); // unary prefix expressions
			ParsingResult<IASTExpr> parseCastExpr();
			ParsingResult<IASTExpr> parseBinaryExpr(const char &priority = 5);
			ParsingResult<IASTExpr> parseExpr(); 
				// PARENS EXPR
			ParsingResult<IASTExpr> parseParensExpr(bool isMandatory = false);

			// STATEMENTS
			ParsingResult<IASTStmt> parseStmt(); // General Statement
			ParsingResult<IASTStmt> parseVarDeclStmt(); // Var Declaration Statement
			ParsingResult<IASTStmt> parseExprStmt(); // Expression statement

			// STATEMENTS : COMPOUND STATEMENT
			ParsingResult<IASTStmt> parseCompoundStatement(); // Compound Statement

			// STATEMENTS : IF,ELSE IF,ELSE
			ParsingResult<IASTStmt> parseCondition(); // Parse a  if-else if-else "block
			
			// STATEMENTS : WHILE LOOP
			ParsingResult<IASTStmt> parseWhileLoop();

			// FUNCTION DECLARATION
			ParsingResult<ASTFunctionDeclaration> parseFunctionDeclaration();
			
		private:
			// Private parse functions

			// type spec (for vardecl).
			std::tuple<bool, bool, std::size_t> parseTypeSpec(); // Tuple values: Success flag, isConst, type of variable.


			// OneUpNode is a function that ups the node one level.
			// Example: There is a node N, with A B (values) as child. You call oneUpNode like this : oneUpNode(N,PLUS)
			// oneUpNode will return a new node X, with the operation PLUS and N as left child.
			std::unique_ptr<ASTBinaryExpr> oneUpNode(std::unique_ptr<ASTBinaryExpr> node, const binaryOperator &op = binaryOperator::PASS);
			
			// matchToken -> returns true if the Token is matched, and increment pos_, if the Token isn't matched return false
			
			// MATCH BY TYPE OF TOKEN
			std::pair<bool,Token> matchLiteral();			// match a literal
			std::pair<bool, std::string> matchID();			// match a ID
			bool matchSign(const Token::sign &s);					// match any signs : ; . ( ) 
			bool matchKeyword(const Token::keyword &k);			// Match any keyword

			std::size_t matchTypeKw();						// match a type keyword : int, float, etc.
			
			// MATCH OPERATORS
			bool							matchExponentOp(); //  **
			std::pair<bool, binaryOperator>	matchAssignOp(); // = 
			std::pair<bool, unaryOperator>	matchUnaryOp(); // ! - +
			std::pair<bool, binaryOperator>	matchBinaryOp(const char &priority); // + - * / % 
			
			// UTILITY METHODS
			Token getToken() const;
			Token getToken(const std::size_t &d) const;

			// resync
			// This function will skip every token until the appropriate "resync" token is found.
			// Returns true if resync was successful.
			bool resyncToDelimiter(const Token::sign &s);

			// die : sets the pos to tokens_.size() and sets isAlive to false. Indicates that the parsing is over and the parser has died because of a critical error.
			void die();

			// Make error message :
			void errorUnexpected();	// generic error message "unexpected Token..". 
			void errorExpected(const std::string &s, const std::vector<std::string>& sugg = {});		// generic error message "expected Token after.."
			void genericError(const std::string &s); // just a normal, generic error

			bool shouldPrintSuggestions_; // unused for now
			
			struct ParserState
			{
				std::size_t pos = 0;						// current pos in the Token vector.
				bool isAlive = true;						// is the parser "alive"?
			} state_;

			ParserState createParserStateBackup() const;
			void restoreParserStateFromBackup(const ParserState& st);

			// Member variables
			Context& context_;
			TokenVector& tokens_;					// reference to the lexer to access our tokens 
	};
}