/*
	matchXXX functions : Parse a NON-TERMINAL.
		Theses function DON'T update the cursor BUT the cursor will be updated if the nonterminal is found, because the parseXXX function called will update it.
	parseXXX functions : Parse a TERMINAL
		Theses functions UPDATE the cursor.

	HOW TO IMPLEMENT A GRAMMAR RULE:
		1 - Check for the terminal or nonterminals of the rule
			Case a : Matched all token 
				Update the cursor, return a valid pointer
			Case b : Matched no token
				Don't update the cursor, return a null pointer
			Case c : matched one or more token, but encountered an unexpected token
				Don't update the cursor, return a null pointer, throw an error (with Error::reportError)

*/
#pragma once
// Lexer
#include "../Lexer/Lexer.h"
// Error reporting
#include "../../Common/Errors/Errors.h"
// AST
#include "../AST/Nodes/ASTExpr.h"
#include "../AST/Nodes/IASTNode.h"
#include "../AST/Nodes/ASTVarDeclStmt.h"
#include "../AST/Nodes/IASTStmt.h"
// Enum
#include "../Util/Enums.h"
#include "../AST/Visitor/Dumper/Dumper.h" // Debug use

#include <tuple>	// std::tuple, std::pair

#define NULL_UNIPTR(x) std::unique_ptr<x>(nullptr)
namespace Moonshot
{
	class Parser
	{
		public:
			Parser(Lexer *l);
			~Parser();

			// parseXXX() = "match" the rule XXX (attempts to find it, if it found it, the method will return a valid pointer (if(ptr) will return true). if not, it will return a std::unique_ptr<(TYPE OF NODE)>(nullptr)
			
			// EXPR
			std::unique_ptr<ASTExpr> parseExpr(const char &priority = 7); // Go from lowest priority to highest !
			std::unique_ptr<ASTExpr> parseTerm();
			std::unique_ptr<ASTExpr> parseValue();
			// Callables
			std::unique_ptr<ASTExpr> parseCallable(); // values/functions calls.

			// STMT
			std::unique_ptr<IASTStmt> parseStmt();
			// Var Declaration Statement
			std::unique_ptr<IASTStmt> parseVarDeclStmt();
				// type spec (for vardecl)
				std::tuple<bool, bool, std::size_t> parseTypeSpec(); // Success flag, isConst, type of variable.
			// Expression statement
			std::unique_ptr<IASTStmt> parseExprStmt();

		private:
			// OneUpNode is a function that ups the node one level.
			// Example: There is a node N, with A B (values) as child. You call oneUpNode like this : oneUpNode(N,parse::PLUS)
			// oneUpNode will return a new node X, with the optype PLUS and N as left child.
			std::unique_ptr<ASTExpr> oneUpNode(std::unique_ptr<ASTExpr> &node, const parse::optype &op = parse::optype::PASS);
			// matchToken -> returns true if the token is matched, and increment pos_, if the token isn't matched return false
			// MATCH BY TYPE OF TOKEN
			std::pair<bool,token> matchValue();				// match a TT_VALUE
			std::pair<bool, std::string> matchID();			// match a ID
			bool matchSign(const lex::signs &s);			// match any signs : ! : * etc.
			bool matchKeyword(const lex::keywords &k);		// Match any keyword

			bool matchEOI();								// Match a EOI, currently a semicolon.
			
			// match a type keyword : int, float, etc.
			std::size_t matchTypeKw();
			// MATCH OPERATORS
			std::pair<bool, parse::optype> matchUnaryOp(); // ! -
			std::pair<bool, parse::optype> matchBinaryOp(const char &priority); // + - * / % ^ ...
			// UTILITY METHODS
			token getToken() const;
			token getToken(const size_t &d) const;
			// Make error message 
			void errorUnexpected();							// generic error message "unexpected token.."
			void errorExpected(const std::string &s);		// generic error message "expected token after.."
			// Member variables
			size_t pos_ = 0;
			Lexer *lex_ = 0;
	};
}
