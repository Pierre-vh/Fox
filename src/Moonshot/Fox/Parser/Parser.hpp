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

namespace Moonshot
{
	class Context;
	class Parser
	{
		public:
			Parser(Context& c,ASTContext& astctxt,TokenVector& l);

			// UNIT
			UnitParsingResult	parseUnit();

			// EXPRESSIONS
			ParsingResult<ASTExpr*> parseSuffix(std::unique_ptr<ASTExpr> &base);
			ParsingResult<IASTDeclRef*> parseDeclCall(); 
			ParsingResult<ASTExpr*> parsePrimitiveLiteral();
			ParsingResult<ASTExpr*> parseArrayLiteral();
			ParsingResult<ASTExpr*> parseLiteral();
			ParsingResult<ASTExpr*> parsePrimary();
			ParsingResult<ASTExpr*> parseArrayOrMemberAccess();
			ParsingResult<ASTExpr*> parseExponentExpr();
			ParsingResult<ASTExpr*> parsePrefixExpr(); 
			ParsingResult<ASTExpr*> parseCastExpr();
			ParsingResult<ASTExpr*> parseBinaryExpr(const char &priority = 5);
			ParsingResult<ASTExpr*> parseExpr(); 

			// STATEMENTS
			ParsingResult<ASTStmt*> parseReturnStmt();
			ParsingResult<ASTStmt*> parseExprStmt(); 
			ParsingResult<ASTCompoundStmt*> parseCompoundStatement(const bool& isMandatory=false); 
			ParsingResult<ASTStmt*> parseStmt();
			ParsingResult<ASTStmt*> parseBody();

			// STATEMENTS : CONDITION & LOOPS
			ParsingResult<ASTStmt*> parseCondition();
			ParsingResult<ASTStmt*> parseWhileLoop();

			// DECLS
			ParsingResult<ASTVarDecl*> parseVarDeclStmt();
			ParsingResult<ASTFunctionDecl*> parseFunctionDeclaration();
			ParsingResult<ASTDecl*> parseDecl();

		private:
			// expression helpers
			ParsingResult<ASTExpr*> parseParensExpr(const bool& isMandatory = false);
			ParsingResult<ExprList*> parseExprList();
			ParsingResult<ExprList*> parseParensExprList();
			// Arg decl & decl list
			ParsingResult<ASTArgDecl*> parseArgDecl();
			// Type spec
			ParsingResult<QualType> parseFQTypeSpec();

			// Type keyword
			// Returns a nullptr if no type keyword is found
			const Type* parseBuiltinTypename();
			// returns a pair : first -> the type, null if none was found. second -> false on error.
			std::pair<const Type*,bool> parseType(); 

			// OneUpNode is a function that ups the node one level.
			// Example: There is a node N, with A and B as children. You call oneUpNode like this : oneUpNode(N,PLUS)
			// oneUpNode will return a new node X, with the operation PLUS and N as left child.
			std::unique_ptr<ASTBinaryExpr> oneUpNode(std::unique_ptr<ASTBinaryExpr> node, const binaryOperator &op = binaryOperator::DEFAULT);
			
			ParsingResult<LiteralInfo> matchLiteral();		// match a literal
			IdentifierInfo* matchID();						// match a ID. Returns the IdentifierInfo* if found, nullptr if not.
			bool matchSign(const SignType& s);				// match any signs : ; . ( ) , returns true if success
			bool matchKeyword(const KeywordType& k);		// match any keyword, returns true if success
			bool peekSign(const std::size_t &idx, const SignType &sign) const;
			
			bool matchExponentOp(); //  **
			ParsingResult<binaryOperator> matchAssignOp();						// = 
			ParsingResult<unaryOperator>  matchUnaryOp();						// ! - +
			ParsingResult<binaryOperator> matchBinaryOp(const char &priority);	// + - * / % 
			
			// GetToken
			Token getToken() const;
			Token getToken(const std::size_t &d) const;

			// Get parserState_.pos
			std::size_t getCurrentPosition() const;

			void incrementPosition();
			void decrementPosition();

			// This function will skip every token until the appropriate "resync" token is found. if consumeToken is set to false, the token won't be consumed.
			// Returns true if resync was successful, false if parser died or recovery is not allowed here.
			bool resyncToSign(const SignType &s,const bool& consumeToken = true);
			// Same as resyncToSign, except it works on "let" and "func" keywords
			bool resyncToNextDeclKeyword();
			// Helper for resyncToSign
			bool isClosingDelimiter(const SignType &s) const;			// Returns true if s is a } or ) or ]
			SignType getOppositeDelimiter(const SignType &s);			// Returns [ for ], { for }, ( for )

			// die : Indicates that the parsing is over and the parser has died because of a critical error. 
			void die();

			// Error helpers
			void errorUnexpected();
			void errorExpected(const std::string &s);
			void genericError(const std::string &s); 
			
			struct ParserState
			{
				ParserState();

				std::size_t pos = 0;						// current pos in the Token vector.
				bool isAlive : 1;						// is the parser "alive"?
				bool isRecoveryAllowed : 1;
			} parserState_;

			// This class manages the recovery of the parser using a RAII idiom.
				// The constructor makes a backup of the parser instance's parserState_.isRecoveryAllowed variable, and replaces parserState_.isRecoveryAllowed with the value desired.
				// The constructor restores the parserState_.isRecoveryAllowed variable to it's original value using the backup.
			class RAIIRecoveryManager
			{
				public:
					explicit RAIIRecoveryManager(Parser *parser,const bool& allowsRecovery);
					~RAIIRecoveryManager();
				private:
					Parser *parser_;
					bool recoveryAllowedBackup_ : 1;
			};

			// Creates a RAII object that authorizes recovery within the parser while it's alive.
			RAIIRecoveryManager createRecoveryEnabler();
			// Creates a RAII object that forbids recovery wihin the parser while it's alive.
			RAIIRecoveryManager createRecoveryDisabler();
			

			// Interrogate parser state
			// Returns true if pos >= tokens_.size()
			bool hasReachedEndOfTokenStream() const;

			// Returns true if parserState_.isAlive
			bool isAlive() const;

			// Parser state backup utilities
			ParserState createParserStateBackup() const;
			void restoreParserStateFromBackup(const ParserState& st);

			// Member variables
			ASTContext& astcontext_;
			Context& context_;
			TokenVector& tokens_;	
	};
}