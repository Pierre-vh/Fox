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
// Terminology :
//			Parens always mean Round Brackets only.
//			Brackets always mean Round/Curly/Square Bracket (Every kind of bracket)
// 
// Status: Up to date with latest grammar changes, except import/using rules that aren't implemented yet.
//
//		Potential Areas of improvement
//			Recovery Efficiency
//				Tweak it by running different test situations and adding special recovery cases wherever needed.
//			Speed
//
//
//		Parser "to-do" list. Important stuff is marked with (*)
//			Add better error recovey with common cases support in if/while parsing & function declaration
//
////------------------------------------------------------////

#pragma once

#include "Fox/Lexer/Token.hpp"					
#include "Fox/AST/Type.hpp"
#include "Fox/AST/Decl.hpp"
#include "Fox/AST/Expr.hpp"
#include "Fox/AST/Stmt.hpp"
#include "Fox/Common/DiagnosticEngine.hpp"
#include "Fox/Common/Utils.hpp"
#include "Fox/Common/LLVM.hpp"

namespace fox
{
	class ASTContext;
	class IdentifierTable;
	class SourceManager;
	class DeclContext;
	class Parser
	{
		public:
			/*-------------- Forward Declarations --------------*/
			template<typename DataTy>
			class Result;

			template<typename DataTy>
			class UniqueResult;

			/*-------------- Usings --------------*/

			// Bunch of usings & helper functions for parsing functions. Theses are public
			// so external classes can use them.
			using ExprResult = UniqueResult<Expr>;
			using ExprListResult = UniqueResult<ExprList>;
			using DeclResult = UniqueResult<Decl>;
			using StmtResult = UniqueResult<Stmt>;
			using UnitResult = UniqueResult<UnitDecl>;
		private:
			using TokenIteratorTy = TokenVector::iterator;
		public:
			// Note : the parser now takes an optional DeclContext* argument,
			// This will be used as the base DeclContext.
			Parser(DiagnosticEngine& diags,SourceManager &sm,ASTContext& astctxt,TokenVector& l,DeclContext* dr = nullptr);

			/*-------------- Parsing Methods --------------*/
			// UNIT
			// Generally, this will be the entry point of the parsing process.
			// fid = The FileID of the file where the unit is contained.
			// unitName = the name of the unit that we're parsing. Usually, the name of the file.
			// isMainUnit = true if the unit that we'll be parsing should be considered as the main unit
			// Note: This function returns an observing pointer (null in case of errors). It doesn't give
			// an unique_ptr like the others, because it gives ownership to the ASTContext, not you.
			UnitDecl* parseUnit(const FileID& fid,IdentifierInfo* unitName,const bool& isMainUnit);

			// EXPRESSIONS
			ExprListResult parseExprList();
			ExprListResult parseParensExprList(SourceLoc* LParenLoc = nullptr, SourceLoc *RParenLoc = nullptr);
			ExprResult parseParensExpr(bool isMandatory,SourceLoc* leftPLoc = nullptr, SourceLoc* rightPLoc = nullptr);
			ExprResult parseSuffix(std::unique_ptr<Expr> &base);
			ExprResult parseDeclRef();
			ExprResult parsePrimitiveLiteral();
			ExprResult parseArrayLiteral();
			ExprResult parseLiteral();
			ExprResult parsePrimary();
			ExprResult parseSuffixExpr();
			ExprResult parseExponentExpr();
			ExprResult parsePrefixExpr();
			ExprResult parseCastExpr();
			ExprResult parseBinaryExpr(std::uint8_t precedence = 5);
			ExprResult parseExpr(); 

			// STATEMENTS
			StmtResult parseReturnStmt();
			StmtResult parseExprStmt();
			StmtResult parseCompoundStatement(bool isMandatory=false);
			StmtResult parseStmt();
			StmtResult parseBody();
			StmtResult parseCondition();
			StmtResult parseWhileLoop();

			// DECLS
			DeclResult parseParamDecl();
			DeclResult parseVarDecl();
			DeclResult parseFunctionDecl();
			DeclResult parseDecl();

			// Getters
			ASTContext& getASTContext();
			SourceManager& getSourceManager();
			DiagnosticEngine& getDiagnosticEngine();
		private:
			/*-------------- Parser Setup --------------*/
			void setupParser();

			/*-------------- "Basic" Parse Methods --------------*/
			// Returns a nullptr if no type keyword is found
			Result<Type*> parseBuiltinTypename();	

			// first -> The Type* (nullptr if not found), second -> False if error
			Result<Type*> parseType();

			// Parses a QualType 
			Result<QualType> parseQualType();

			Result<BinaryOperator> parseAssignOp();						// = 
			Result<UnaryOperator>  parseUnaryOp();						// ! - +
			Result<BinaryOperator> parseBinaryOp(std::uint8_t priority);	// + - * / % 
			SourceRange parseExponentOp();											//  **

			/*-------------- Token Consuming --------------*/
			/*	
				Consume methods all return a result that evaluates to true if the "consume" operation finished successfully 
				(found the requested token), false otherwise

				Note: SourceLocs and SourceRanges can be both evaluated in a condition to check their validity (operator bool is implemented on both)
			*/

			// Consumes an Identifier
			// The Result will have the SourceRange of the identifier if one was found.
			Result<IdentifierInfo*> consumeIdentifier();

			// Consumes any sign but brackets.
			SourceLoc consumeSign(SignType s);

			// Consumes a bracket and keeps the bracket count up to date. Returns an invalid SourceLoc if the bracket was not found.
			// Note : In the US, a Bracket is a [], however, here the bracket noun is used in the strict sense, where Round B. = (), Square B. = [] and Curly B. = {}
			SourceLoc consumeBracket(SignType s);

			// Consumes a keyword. Returns an invalid SourceRange if not found.
			SourceRange consumeKeyword(KeywordType k);

			// Peek a Keyword or a Sign. Returns true if the next token is of the requested kind.
			// Does not update any counter.
			bool peekNext(SignType s);
			bool peekNext(KeywordType s);

			// Dispatch to the appriate consume method. Won't return any loc information.
			// Used to skip a token, updating any necessary counters.
			void consumeAny();

			// Reverts the last consume operation, updates counters if needed.
			void revertConsume();

			// Increments the iterator if possible. Used to skip a token without updating any counters.
			void increaseTokenIter();

			// Decrements the iterator if possible. Used to revert a consume operation. Won't change updated counters.
			// Only use in cases where a counter wasn't updated by the last consume operation. Else, use a Parser State Backup.
			void decreaseTokenIter();	

			// Helper for consumeSign & consumeBracket
			// Brackets are one of the following : '(' ')' '[' ']' '{' '}'
			bool isBracket(SignType s) const;

			Token getCurtok() const;
			Token getPreviousToken() const;
			
			/*-------------- Error Recovery --------------*/
				// Last Parameter is an optional pointer to a SourceRange. If the recovery was successful, the SourceRange of the token found
				// will be saved there.
			bool resyncToSign(SignType sign, bool stopAtSemi, bool shouldConsumeToken);
			bool resyncToSign(const std::vector<SignType>& signs, bool stopAtSemi, bool shouldConsumeToken);
			bool resyncToNextDecl();

			/*-------------- Error Reporting --------------*/
			// Reports an error of the "unexpected" family.
			// The SourceLoc of the error is right past the end of the previous token.
			Diagnostic reportErrorExpected(DiagID diag);

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
				std::uint8_t curlyBracketsCount  = 0;
				std::uint8_t roundBracketsCount  = 0;
				std::uint8_t squareBracketsCount = 0;

				// Current Decl Recorder
				DeclContext *declContext = nullptr;
			} state_;

			// Interrogate state_
				// isDone returns false if( (state_.tokenIterator == tokens_.end) or !isAlive())
			bool isDone() const;
				// Returns state_.isAlive
			bool isAlive() const;
				// Kills Parsing (stops it)
			void die();

			// Register a declaration in state_.declContext, asserting that it's not null.
			void recordDecl(NamedDecl *nameddecl);

			// Creates a state_ backup
			ParserState createParserStateBackup() const;
			// Restores state_ from a backup.
			void restoreParserStateFromBackup(const ParserState& st);

			/*-------------- RAIIDeclContext --------------*/
			// This class sets the current DeclContext at construction, and restores the last
			// one at destruction.
			// If the DeclContext that was here before isn't null, it's marked as being the parent of the DeclContext passed as argument to the constructor.
			// It assists in registering Decl in the appropriate DeclContext.
			class RAIIDeclContext
			{
				public:
					RAIIDeclContext(Parser &p,DeclContext *dr);
					~RAIIDeclContext();
				private:
					Parser& parser_;
					DeclContext* declCtxt_ = nullptr;
			};

			/*-------------- Member Variables --------------*/
			ASTContext& astContext_;
			IdentifierTable& identifiers_;
			DiagnosticEngine& diags_;
			SourceManager& srcMgr_;
			TokenVector& tokens_;
			
			/*-------------- Constants --------------*/
			static constexpr uint8_t maxBraceDepth_ = (std::numeric_limits<std::uint8_t>::max)();

		public:
			/*-------------- Result Classes --------------*/
			// Class for encapsulating a parsing function's result.
			// It also stores a SourceRange to store a Position/Range if needed.
			template<typename DataTy>
			class Result
			{
				public:
					Result(DataTy res, const SourceRange& range = SourceRange()) : result_(res), hasData_(true), successFlag_(true), range_(range)
					{

					}

					Result(bool wasSuccessful = true) : hasData_(false), successFlag_(wasSuccessful)
					{

					}

					explicit operator bool() const
					{
						return isUsable();
					}

					bool wasSuccessful() const
					{
						return successFlag_;
					}

					bool isUsable() const
					{
						return successFlag_ && hasData_;
					}

					DataTy get() const
					{
						return result_;
					}

					// Helper methods 
					static Result<DataTy> Error()
					{
						return Result<DataTy>(false);
					}

					static Result<DataTy> NotFound()
					{
						return Result<DataTy>(true);
					}

					SourceRange getSourceRange() const
					{
						return range_;
					}

				private:
					SourceRange range_;
					bool hasData_ : 1;
					bool successFlag_ : 1;
					DataTy result_;
			};

			// Result class that holds it's information as a unique_ptr.
			template<typename DataTy>
			class UniqueResult
			{
				public:
					UniqueResult(std::unique_ptr<DataTy> res) : result_(std::move(res)), successFlag_(true)
					{

					}

					UniqueResult(bool wasSuccessful = true) : result_(nullptr), successFlag_(wasSuccessful)
					{

					}

					explicit operator bool() const
					{
						return isUsable();
					}

					bool wasSuccessful() const
					{
						return successFlag_;
					}

					bool isUsable() const
					{
						return successFlag_ && result_;
					}

					// Helper methods 
					static UniqueResult<DataTy> Error()
					{
						return UniqueResult<DataTy>(false);
					}

					static UniqueResult<DataTy> NotFound()
					{
						return UniqueResult<DataTy>(true);
					}

					DataTy* getObserverPtr()
					{
						return result_.get();
					}

					// Moves the content of the ParsingResult as a derived class.
					// Asserts that result_ isn't null, check with is() or isUsable() before using if you're unsure.
					// Note that the type is asserted to be castable to the desired type.
					template<typename Derived>
					std::unique_ptr<Derived> moveAs()
					{
						assert(result_ && "Result was null, or has already been moved!");
						Derived *ptr = dyn_cast<Derived>(result_.get());
						assert(ptr && "Can't cast to desired type");
						result_.release();
						return std::unique_ptr<Derived>(ptr);
					}

					// Classic move. Asserts that result_ isn't null, check with is() or isUsable() before using if you're unsure.
					std::unique_ptr<DataTy> move()
					{
						assert(result_ && "Result was null, or has already been moved!");
						return std::move(result_);
					}

					// Checks that Derived can be cast to the correct type.
					template<typename Derived>
					bool is() const
					{
						return isa<Derived>(result_.get());
					}
				private:
					bool successFlag_ : 1;
					std::unique_ptr<DataTy> result_;
			};
	};
}