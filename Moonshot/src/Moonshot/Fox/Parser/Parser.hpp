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
// Include interfaces so the users of this class can manipulate 
#include "Moonshot/Fox/AST/Nodes/IASTExpr.hpp"
#include "Moonshot/Fox/AST/Nodes/IASTStmt.hpp"
#include "Moonshot/Fox/AST/Nodes/ForwardDeclarations.hpp"
#include "Moonshot/Fox/Util/Enums.hpp"			

#include <tuple>							// std::tuple, std::pair
#include <memory>							// std::shared_ptr
#include <stack>							// std::stack

#define DEFAULT__maxExpectedErrorsCount 2
#define DEFAULT__shouldPrintSuggestions true

namespace Moonshot
{
	/*
	// put this in its own file! (.hpp, it needs to be header only because of template)
	template<typename T>
	struct ParsingResult
	{
		public:
			parsingResult() = default;
			parsingResult(T& value); // sets wasSuccessful to true. will probably have problems with value when it's a unique ptr
									// use std::is_copy_construcitble with a constrexpr if/else + std::move in case it's not cpy constr?
			bool wasSuccesful(); // return successful_ 
			bool isResultValid();  // return result_ ? true : false
		private:
			T result_;
			bool successful_ = false;
	};

	template<typename TYPE>
	using parsingResult_uptr = parsingResult<std::unique_ptr<TYPE>>;
	*/
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
			std::unique_ptr<IASTStmt> parseExprStmt(); // Expression statement

			// STATEMENTS : COMPOUND STATEMENT
			std::unique_ptr<IASTStmt> parseCompoundStatement(); // Compound Statement

			// STATEMENTS : IF,ELSE IF,ELSE
			std::unique_ptr<IASTStmt> parseCondition(); // Parse a  if-else if-else "block
			
			// STATEMENTS : WHILE LOOP
			std::unique_ptr<IASTStmt> parseWhileLoop();
		private:
			// Private parse functions

			// type spec (for vardecl).
			std::tuple<bool, bool, std::size_t> parseTypeSpec(); // Tuple values: Success flag, isConst, type of variable.
			
			// ParseCondition helper functions
			ConditionalStatement parseCond_if();	 // Parses a classic if statement.
			ConditionalStatement parseCond_elseIf(); // Parses a else if
			std::unique_ptr<IASTStmt> parseCond_else(); // parse a else


			// OneUpNode is a function that ups the node one level.
			// Example: There is a node N, with A B (values) as child. You call oneUpNode like this : oneUpNode(N,PLUS)
			// oneUpNode will return a new node X, with the operation PLUS and N as left child.
			std::unique_ptr<ASTBinaryExpr> oneUpNode(std::unique_ptr<ASTBinaryExpr> &node, const binaryOperation &op = binaryOperation::PASS);
			
			// matchToken -> returns true if the Token is matched, and increment pos_, if the Token isn't matched return false
			
			// MATCH BY TYPE OF TOKEN
			std::pair<bool,Token> matchLiteral();			// match a literal
			std::pair<bool, std::string> matchID();			// match a ID
			bool matchSign(const sign &s);					// match any signs : ! : * etc.
			bool matchKeyword(const keyword &k);			// Match any keyword

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
			
			unsigned int maxExpectedErrorCount_;
			unsigned int currentExpectedErrorsCount_ = 0; 	// Current "expected" error count, used to avoid "expected (x)" spam by the interpreter.
			bool shouldPrintSuggestions_; // unused for now

			struct ParserState
			{
				std::size_t pos = 0;						// current pos in the Token vector.
			} state_;

			ParserState createParserStateBackup() const;
			void restoreParserStateFromBackup(const ParserState& st);

			// Member variables
			Context& context_;
			TokenVector& tokens_;					// reference to the lexer to access our tokens 
	};
}