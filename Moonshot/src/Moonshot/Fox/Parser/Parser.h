////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : Parser.h											
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
			Case a : Matched all token 
				Return a valid pointer to the node. Cursor is updated through match function
			Case b : Matched no token
				 return a null pointer, cursor isn't updated
			Case c : matched one or more token, but encountered an unexpected token
				Don't update the cursor, return a null pointer, throw an error (with the context)
		2 - Terminal rules
			If the current token is the requested token, update the cursor.

*/
#pragma once
// Context and Exceptions
#include "../../Common/Context/Context.h"
#include "../../Common/Exceptions/Exceptions.h"
// Lexer
#include "../Lexer/Lexer.h"					
// AST Nodes
#include "../AST/Nodes/ASTExpr.h"
#include "../AST/Nodes/IASTNode.h"
#include "../AST/Nodes/ASTVarDeclStmt.h"
#include "../AST/Nodes/ASTCompStmt.h"
#include "../AST/Nodes/ASTCondition.h"
#include "../AST/Nodes/IASTStmt.h"
#include "../Util/Enums.h"					// Enum
#include "../AST/Visitor/Dumper/Dumper.h"	// Dumper Visitor, for Debug use
#include <tuple>							// std::tuple, std::pair
#include <memory>							// std::shared_ptr

namespace Moonshot
{
	class Parser
	{
		public:
			Parser(Context& c,Lexer& l);
			~Parser();

			// parseXXX() = "match" the rule XXX (attempts to find it, if it found it, the method will return a valid pointer (if(ptr) will return true). if not, it will return a std::unique_ptr<(TYPE OF NODE)>(nullptr)
			
			// EXPRESSIONS
			std::unique_ptr<ASTExpr> parseExpr(const char &priority = 7); // Go from lowest priority to highest !
			std::unique_ptr<ASTExpr> parseTerm();
			std::unique_ptr<ASTExpr> parseValue();
			std::unique_ptr<ASTExpr> parseCallable(); // values/functions calls.

			// STATEMENTS
			std::unique_ptr<IASTStmt> parseStmt(); // General Statement
			std::unique_ptr<IASTStmt> parseVarDeclStmt(); // Var Declaration Statement
			std::tuple<bool, bool, std::size_t> parseTypeSpec(); // type spec (for vardecl). Tuple values: Success flag, isConst, type of variable.
			std::unique_ptr<IASTStmt> parseExprStmt(); // Expression statement
			// STATEMENTS : COMPOUND STATEMENT
			std::unique_ptr<ASTCompStmt> parseCompoundStatement(); // Compound Statement
			// STATEMENTS : IF,ELSE IF,ELSE
			std::unique_ptr<ASTCondition> parseCondition(); // Parse a  if-else if-else "block"
		private:
			// Private parse functions
			ASTCondition::CondBlock parseCond_if();
			ASTCondition::CondBlock parseCond_else_if();
			// OneUpNode is a function that ups the node one level.
			// Example: There is a node N, with A B (values) as child. You call oneUpNode like this : oneUpNode(N,PLUS)
			// oneUpNode will return a new node X, with the operation PLUS and N as left child.
			std::unique_ptr<ASTExpr> oneUpNode(std::unique_ptr<ASTExpr> &node, const operation &op = operation::PASS);
			
			// matchToken -> returns true if the token is matched, and increment pos_, if the token isn't matched return false
			
			// MATCH BY TYPE OF TOKEN
			std::pair<bool,token> matchValue();				// match a TT_LITERAL
			std::pair<bool, std::string> matchID();			// match a ID
			bool matchSign(const signType &s);			// match any signs : ! : * etc.
			bool matchKeyword(const keywordType &k);		// Match any keyword

			bool matchEOI();								// Match a EOI, currently a semicolon.
			std::size_t matchTypeKw();						// match a type keyword : int, float, etc.
			
			// MATCH OPERATORS
			std::pair<bool, operation> matchUnaryOp(); // ! -
			std::pair<bool, operation> matchBinaryOp(const char &priority); // + - * / % ^ ...
			
			// UTILITY METHODS
			token getToken() const;
			token getToken(const size_t &d) const;

			// Make error message :
			// 2 Types of error messages in the parser : unexpected token and Expected a token.
			void errorUnexpected();							// generic error message "unexpected token.."
			void errorExpected(const std::string &s);		// generic error message "expected token after.."
			

			// Member variables
			size_t pos_ = 0;								// current pos in the token vector.
			Context& context_;
			Lexer& lex_;					// reference to the lexer to access our tokens 
	};
}