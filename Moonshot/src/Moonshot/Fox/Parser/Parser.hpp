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
#include "../Lexer/Token.hpp"					
// AST Nodes
#include "Moonshot/Fox/AST/Nodes/ASTExpr.hpp"
#include "Moonshot/Fox/AST/Nodes/ASTVarDeclStmt.hpp"
#include "Moonshot/Fox/AST/Nodes/ASTCompStmt.hpp"
#include "Moonshot/Fox/AST/Nodes/ASTCondition.hpp"
#include "Moonshot/Fox/AST/Nodes/ASTWhileLoop.hpp"
#include "Moonshot/Fox/AST/Nodes/IASTStmt.hpp"
#include "Moonshot/Fox/Util/Enums.hpp"			

#include <tuple>							// std::tuple, std::pair
#include <memory>							// std::shared_ptr
#include <stack>							// std::stack

#define DEFAULT__maxExpectedErrorsCount 2
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
			std::unique_ptr<IASTExpr> parseCallable(); // values/functions calls.
			std::unique_ptr<IASTExpr> parseValue();
			std::unique_ptr<IASTExpr> parseExponentExpr();
			std::unique_ptr<IASTExpr> parsePrefixExpr(); // unary prefix expressions
			std::unique_ptr<IASTExpr> parseCastExpr();
			std::unique_ptr<IASTExpr> parseBinaryExpr(const char &priority = 5);
			std::unique_ptr<IASTExpr> parseExpr(); // Go from lowest priority to highest !

			// STATEMENTS
			std::unique_ptr<IASTStmt> parseStmt(); // General Statement
			std::unique_ptr<IASTStmt> parseVarDeclStmt(); // Var Declaration Statement
			std::tuple<bool, bool, std::size_t> parseTypeSpec(); // type spec (for vardecl). Tuple values: Success flag, isConst, type of variable.
			std::unique_ptr<IASTStmt> parseExprStmt(); // Expression statement

			// STATEMENTS : COMPOUND STATEMENT
			std::unique_ptr<IASTStmt> parseCompoundStatement(); // Compound Statement

			// STATEMENTS : IF,ELSE IF,ELSE
			std::unique_ptr<IASTStmt> parseCondition(); // Parse a  if-else if-else "block
			
			// STATEMENTS : WHILE LOOP
			std::unique_ptr<IASTStmt> parseWhileLoop();
		private:
			// Private parse functions
			// ParseCondition helper functions
			ASTCondition::CondBlock parseCond_if();
			ASTCondition::CondBlock parseCond_else_if();
			// OneUpNode is a function that ups the node one level.
			// Example: There is a node N, with A B (values) as child. You call oneUpNode like this : oneUpNode(N,PLUS)
			// oneUpNode will return a new node X, with the operation PLUS and N as left child.
			std::unique_ptr<ASTBinaryExpr> oneUpNode(std::unique_ptr<ASTBinaryExpr> &node, const binaryOperation &op = binaryOperation::PASS);
			
			// matchToken -> returns true if the Token is matched, and increment pos_, if the Token isn't matched return false
			
			// MATCH BY TYPE OF TOKEN
			std::pair<bool,Token> matchLiteral();				// match a TT_LITERAL
			std::pair<bool, std::string> matchID();			// match a ID
			bool matchSign(const sign &s);			// match any signs : ! : * etc.
			bool matchKeyword(const keyword &k);		// Match any keyword

			bool matchEOI();								// Match a EOI, currently a semicolon.
			std::size_t matchTypeKw();						// match a type keyword : int, float, etc.
			
			// MATCH OPERATORS
			bool								matchExponentOp(); // only **
			std::pair<bool, binaryOperation>	matchAssignOp(); // only = for now.
			std::pair<bool, unaryOperation>		matchUnaryOp(); // ! -
			std::pair<bool, binaryOperation>	matchBinaryOp(const char &priority); // + - * / % ** ...
			
			// UTILITY METHODS
			Token getToken() const;
			Token getToken(const size_t &d) const;

			// Make error message :
			// 2 Types of error messages in the parser : unexpected Token and Expected a Token.
			void errorUnexpected();							// generic error message "unexpected Token.."
			void errorExpected(const std::string &s, const std::vector<std::string>& sugg = {});		// generic error message "expected Token after.."
			
			int currentExpectedErrorsCount = 0;
			int maxExpectedErrorCount = 0;
			bool shouldPrintSuggestions;
			// Member variables
			size_t pos_ = 0;								// current pos in the Token vector.
			Context& context_;
			TokenVector& tokens_;					// reference to the lexer to access our tokens 
	};
}