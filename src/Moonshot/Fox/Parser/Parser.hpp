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

#include "Moonshot/Fox/Lexer/Token.hpp"					
#include "Moonshot/Fox/Parser/ParsingResult.hpp"

#include "Moonshot/Fox/AST/ASTContext.hpp"
#include "Moonshot/Fox/AST/Types.hpp"
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
			Parser(Context& c,ASTContext* astctxt,TokenVector& l);

			// UNIT
			ParsingResult<ASTUnit*>	parseUnit();

			// EXPRESSIONS
			ParsingResult<IASTDeclRef*> parseArrayAccess(std::unique_ptr<IASTDeclRef> &base);
			ParsingResult<IASTDeclRef*> parseDeclCall(); 
			ParsingResult<ASTExpr*> parseLiteral();
			ParsingResult<ASTExpr*> parsePrimary();
			ParsingResult<ASTExpr*> parseMemberAccess();
			ParsingResult<ASTExpr*> parseExponentExpr();
			ParsingResult<ASTExpr*> parsePrefixExpr(); 
			ParsingResult<ASTExpr*> parseCastExpr();
			ParsingResult<ASTExpr*> parseBinaryExpr(const char &priority = 5);
			ParsingResult<ASTExpr*> parseExpr(); 

			// STATEMENTS
			ParsingResult<ASTStmt*> parseReturnStmt();
			ParsingResult<ASTStmt*> parseExprStmt(); // Expression statement
			ParsingResult<ASTCompoundStmt*> parseCompoundStatement(const bool& isMandatory=false, const bool& recoverOnError = false); 
			ParsingResult<ASTStmt*> parseStmt(); // General Statement
			ParsingResult<ASTStmt*> parseBody(); // body for control flow

			// STATEMENTS : CONDITION & LOOPS
			ParsingResult<ASTStmt*> parseCondition(); // Parse a  if-else if-else "block
			ParsingResult<ASTStmt*> parseWhileLoop();

			// DECLS
			ParsingResult<ASTVarDecl*> parseVarDeclStmt(const bool& recoverToSemiOnError = true);
			ParsingResult<ASTFunctionDecl*> parseFunctionDeclaration();
			ParsingResult<ASTDecl*> parseDecl();

		private:
			// expression helpers
			ParsingResult<ASTExpr*> parseParensExpr(const bool& isMandatory = false);
			ParsingResult<ExprList*> parseExprList();
			ParsingResult<ExprList*> parseParensExprList();
			// Arg decl & decl list
			ParsingResult<FunctionArg> parseArgDecl();
			ParsingResult<std::vector<FunctionArg>> parseArgDeclList();
			// Type spec
			ParsingResult<QualType> parseFQTypeSpec();
			// Type keyword
			// Note : Returns a nullptr if no type keyword is found
			TypePtr parseTypeKw();

			// OneUpNode is a function that ups the node one level.
			// Example: There is a node N, with A and B as children. You call oneUpNode like this : oneUpNode(N,PLUS)
			// oneUpNode will return a new node X, with the operation PLUS and N as left child.
			std::unique_ptr<ASTBinaryExpr> oneUpNode(std::unique_ptr<ASTBinaryExpr> node, const binaryOperator &op = binaryOperator::DEFAULT);
			
			// matchToken -> returns true if the Token is matched, and increment pos_, if the Token isn't matched return false
			// Peek != Match, Peek tries to match a token, if it does, it returns true and DOES NOT increment the position. Match does the same but increments if found.
			// Match
			ParsingResult<LiteralInfo> matchLiteral();		// match a literal
			IdentifierInfo* matchID();						// match a ID. Returns the IdentifierInfo* if found, nullptr if not.
			bool matchSign(const SignType& s);				// match any signs : ; . ( ) , returns true if success
			bool matchKeyword(const KeywordType& k);		// match any keyword, returns true if success
			bool peekSign(const std::size_t &idx, const SignType &sign) const;
			
			bool							matchExponentOp(); //  **
			ParsingResult<binaryOperator>	matchAssignOp(); // = 
			ParsingResult<unaryOperator>	matchUnaryOp(); // ! - +
			ParsingResult<binaryOperator>	matchBinaryOp(const char &priority); // + - * / % 
			
			// GetToken
			Token getToken() const;
			Token getToken(const std::size_t &d) const;

			// Get state_.pos
			std::size_t getCurrentPosition() const;
			// Get state_.pos++
			std::size_t getNextPosition() const; 

			void incrementPosition();
			void decrementPosition();

			// This function will skip every token until the appropriate "resync" token is found. if consumeToken is set to false, the token won't be consumed.
			// Returns true if resync was successful.
			bool resyncToSign(const SignType &s,const bool& consumeToken = true);
			// Same as resyncToSign, except it works on "let" and "func" keywords
			bool resyncToNextDeclKeyword();
			// Helper for resyncToSign
			bool isClosingDelimiter(const SignType &s) const;			// Returns true if s is a } or ) or ]
			SignType getOppositeDelimiter(const SignType &s);			// Returns [ for ], { for }, ( for )

			// die : Indicates that the parsing is over and the parser has died because of a critical error. 
			void die();

			// Typical parser error message helpers
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
			bool isAlive() const;

			// Parser state backup
			ParserState createParserStateBackup() const;
			void restoreParserStateFromBackup(const ParserState& st);

			// Member variables
			ASTContext* astCtxt_ = nullptr;
			Context& context_;
			TokenVector& tokens_;					// reference to the the token's vector.
	};
}