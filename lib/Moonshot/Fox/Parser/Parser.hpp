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
// Status: Up to date with latest grammar changes, but isn't finished yet. Functions decl/call aren't in, and import/using statements too.
////------------------------------------------------------////

#pragma once

#include "Moonshot/Common/Types/Types.hpp"
#include "Moonshot/Fox/Lexer/Token.hpp"					
#include "Moonshot/Fox/Parser/ParsingResult.hpp"

#include "Moonshot/Fox/AST/ASTDecl.hpp"
#include "Moonshot/Fox/AST/ASTExpr.hpp"
#include "Moonshot/Fox/AST/ASTStmt.hpp"

#include "Moonshot/Fox/AST/Operators.hpp"			

#include <tuple>							// std::tuple, std::pair
#include <memory>							// std::shared_ptr
#include <vector>							// std::vector

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
			ParsingResult<IASTExpr*> parseDeclCall(); 
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
			ParsingResult<ASTCompoundStmt*> parseCompoundStatement(const bool& isMandatory=false); // Compound Statement
			ParsingResult<IASTStmt*> parseStmt(); // General Statement
			ParsingResult<IASTStmt*> parseBody(); // body for control flow

			// STATEMENTS : CONDITION & LOOPS
			ParsingResult<IASTStmt*> parseCondition(); // Parse a  if-else if-else "block
			ParsingResult<IASTStmt*> parseWhileLoop();

			// DECLS-STMTS
			ParsingResult<IASTStmt*> parseVarDeclStmt(); // Var Declaration Statement
			// DECLS
			ParsingResult<ASTFunctionDecl*> parseFunctionDeclaration();
			
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
			
			// MATCH BY TYPE OF TOKEN
			ParsingResult<FoxValue> matchLiteral();			// match a literal
			ParsingResult<std::string> matchID();			// match a ID
			bool matchSign(const Token::sign &s);			// match any signs : ; . ( ) , returns true if success
			bool matchKeyword(const Token::keyword &k);		// Match any keyword, returns true if success

			ParsingResult<std::size_t> matchTypeKw();		// match a type keyword : int, float, etc. and returns its index in the FValue
			
			// MATCH OPERATORS
			bool							matchExponentOp(); //  **
			ParsingResult<binaryOperator>	matchAssignOp(); // = 
			ParsingResult<unaryOperator>	matchUnaryOp(); // ! - +
			ParsingResult<binaryOperator>	matchBinaryOp(const char &priority); // + - * / % 
			
			// UTILITY METHODS
			Token getToken() const;
			Token getToken(const std::size_t &d) const;

			// resync
			// This function will skip every token until the appropriate "resync" token is found.
			// Returns true if resync was successful.
			/*
				Note, this could use improvements, for instance a maximum thresold, or stop when a '}' is found to avoid matching to a semicolon out of the compound statement, etc.
				This is a matter for another time, first I want to finish the interpreter up to v1.0, then i'll do a refactor to give better error messages before moving on to other features (arrays, oop, tuples)
			*/
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
			TokenVector& tokens_;					// reference to the the token's vector.
	};
}