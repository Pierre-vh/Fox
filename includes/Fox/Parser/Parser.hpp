//----------------------------------------------------------------------------//
// This file is part of the Fox project.        
// See the LICENSE.txt file at the root of the project for license information.            
// File : Parser.hpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//
// This file implements the recursive descent parser for Fox.
//                              
// The grammar can be found in  /doc/ 
//
// Terminology note:
//      Parens always mean Round Brackets only.
//      Brackets always mean Round/Curly/Square Bracket (Every kind of bracket)
//----------------------------------------------------------------------------//

#pragma once

#include "Fox/Lexer/Token.hpp"          
#include "Fox/AST/Type.hpp"
#include "Fox/AST/Decl.hpp"
#include "Fox/AST/Expr.hpp"
#include "Fox/AST/Stmt.hpp"
#include "ParserResultObject.hpp"

namespace fox {
  class ASTContext;
  class IdentifierTable;
  class SourceManager;
  class DeclContext;
  class DiagnosticEngine;
  class Diagnostic;
  class ASTNode;
  enum class DiagID : std::uint16_t;
  class Parser {
      //----------------------------------------------------------------------//
      // Forward Declarations
      //----------------------------------------------------------------------//
    public:
      template<typename DataTy>
      class Result;

      //----------------------------------------------------------------------//
      // Type Aliases
      //----------------------------------------------------------------------//
    private:
      // The type of the Token iterator
      using TokenIteratorTy = TokenVector::iterator;

    public:
      //----------------------------------------------------------------------//
      // Public Parser Interface
      //----------------------------------------------------------------------//

      // Constructor for the Parser. 
      // If you plan to use the parser by calling parseDecl/parseFuncDecl/
      // parseVarDecl directly, you MUST pass a UnitDecl to the constructor.
      Parser(ASTContext& astctxt, TokenVector& l, 
        UnitDecl* parentUnit = nullptr);

			// Parse a complete Unit
      UnitDecl* parseUnit(FileID fid, Identifier unitName);

      // Parse a single variable declaration
      Result<Decl*> parseVarDecl();

      // Parse a single function declaration
      Result<Decl*> parseFuncDecl();

      // Parse a single function or variable declaration
      Result<Decl*> parseDecl();

      //----------------------------------------------------------------------//
      // References to other important Fox classes
      //----------------------------------------------------------------------//

      // The ASTContext, used to allocate every node in the AST.
      ASTContext& ctxt;

      // The DiagnosticEngine, used to emit diagnostics.
      // This is a shortcut to ctxt.diagEngine
      DiagnosticEngine& diags;

      // The SourceManager, use to retrieve source information
      // This is a shortcut to ctxt.sourceMgr
      SourceManager& srcMgr;

      // The vector of tokens being considered by the parser
      TokenVector& tokens;

    private:
      //----------------------------------------------------------------------//
      // Private parser methods
      //----------------------------------------------------------------------//

      //---------------------------------//
      // Expression parsing helpers
      //---------------------------------//

      // Parses a list of expressions
      Result<ExprVector> parseExprList();

      // Parse a list of expression between parentheses
      Result<ExprVector> 
			parseParensExprList(SourceLoc *RParenLoc = nullptr);

      // Parse an expression between parentheses
      Result<Expr*> parseParensExpr();

      Result<Expr*> parseSuffix(Expr* base);
      Result<Expr*> parseDeclRef();
      Result<Expr*> parsePrimitiveLiteral();
      Result<Expr*> parseArrayLiteral();
      Result<Expr*> parseLiteral();
      Result<Expr*> parsePrimary();
      Result<Expr*> parseSuffixExpr();
      Result<Expr*> parseExponentExpr();
      Result<Expr*> parsePrefixExpr();
      Result<Expr*> parseCastExpr();
      Result<Expr*> parseBinaryExpr(std::uint8_t precedence = 5);
      Result<Expr*> parseExpr(); 

      //---------------------------------//
      // Statement parsing helpers
      //---------------------------------//

      Result<Stmt*> parseReturnStmt();
      Result<ASTNode> parseExprStmt();
      Result<Stmt*> parseCompoundStatement();
      Result<ASTNode> parseStmt();
      Result<Stmt*> parseCondition();
      Result<Stmt*> parseWhileLoop();

      //---------------------------------//
      // Declaration parsing helpers
      //---------------------------------//

      // Parses a parameter declaration.
      Result<Decl*> parseParamDecl();

      //---------------------------------//
      // Type parsing helpers
      //---------------------------------//

      // Parses a builtin type name
      Result<Type> parseBuiltinTypename();

      // Parses a complete type e.g. [[float]]
      Result<Type> parseType();

      //---------------------------------//
      // Operators parsing helpers
      //---------------------------------//

      // Parses any assignement operator
      Result<BinaryExpr::OpKind> parseAssignOp();

      // Parses any unary operator
      Result<UnaryExpr::OpKind> parseUnaryOp();
      
      // Parses any binary operator
      Result<BinaryExpr::OpKind> parseBinaryOp(std::uint8_t priority);
      SourceRange parseExponentOp();

      //---------------------------------//
      // Current Decl Parent (curParent_) helpers
      //---------------------------------//

      // Should be called whenever a decl is done parsing and about 
      // to be returned.
      void actOnDecl(Decl* decl);

      // Returns true if state_.curParent.is<FuncDecl*>();
      bool isParsingFuncDecl() const;
      // Returns true if state_.curParent is nullptr OR a 
      // DeclContext
      bool isDeclParentADeclCtxtOrNull() const;

      // Asserts that the current decl parent is a DeclContext
      // or nullptr, then returns state_.curParent().dyn_cast<DeclContext*>()
      DeclContext* getDeclParentAsDeclCtxt() const;

      //---------------------------------//
      // Token consumption/manipulation helpers
      //---------------------------------//

      /*  
        Note: Token consuming methods
          Consume methods all return a result that evaluates to true 
				  if the "consume" operation finished successfully 
          (found the requested token), false otherwise

          Note: SourceLocs and SourceRanges can be both evaluated in 
				  a condition to check their validity 
          (operator bool is implemented on both)
      */

      // Consumes an Identifier
      // The Result object will contain the SourceRange of the identifier
			// on a success
      Result<Identifier> consumeIdentifier();

      // Consumes any sign but brackets.
      SourceLoc consumeSign(SignType s);

      // Consumes a bracket and keeps the bracket count up to date.
      // Returns an invalid SourceLoc if the bracket was not found.
      // Note : In the US, a Bracket is a [], however, here the bracket noun 
      // is used in the strict sense, where 
      // Round B. = (), Square B. = [] and Curly B. = {}
      SourceLoc consumeBracket(SignType s);

      // Consumes a keyword. Returns an invalid SourceRange if not found.
      SourceRange consumeKeyword(KeywordType k);

      // Dispatch to the appriate consume method. Won't return any loc information.
      // Used to skip a token, updating any necessary counters.
      void consumeAny();

      // Reverts the last consume operation, updates counters if needed.
      void revertConsume();

      // Increments the iterator if possible. Used to skip a token 
      // without updating any counters.
      void next();

      // Decrements the iterator if possible. Used to revert a consume operation. 
			// Won't change updated counters.
      // Only use in cases where a counter wasn't updated by the
      // last consume operation. 
			// Else (or when in doubt), use revertConsume
      void undo();  

      // Helper for consumeSign & consumeBracket
      // Brackets are one of the following : '(' ')' '[' ']' '{' '}'
      bool isBracket(SignType s) const;

      // Returns the current token (*tokenIterator_)
      Token getCurtok() const;
      
      // Returns the previous token (*(--tokenIterator))
      Token getPreviousToken() const;
      
      //---------------------------------//
      // Resynchronization helpers
      //---------------------------------//

      bool resyncToSign(SignType sign, bool stopAtSemi, 
        bool shouldConsumeToken);
      bool resyncToSign(const SmallVector<SignType, 4>& signs, bool stopAtSemi,
				bool shouldConsumeToken);
      bool resyncToNextDecl();

      //---------------------------------//
      // Error reporting
      //---------------------------------//

      // Reports an error of the "unexpected" family.
      // The SourceLoc of the error is right past the end of the undo token.
      Diagnostic reportErrorExpected(DiagID diag);

      //---------------------------------//
      // Parser "state" variables & methods
      //
      // The variables are part of what I call the "Parser State".
      //---------------------------------//

      // Iterator to the current token being considered
      // by the parser.
      TokenIteratorTy tokenIterator_;

      // isAlive
		  //  This is set to false when the parser dies (=gives up)
      bool isAlive_ : 1;
      
      // Brackets counters
      std::uint8_t curlyBracketsCount_  = 0;
      std::uint8_t roundBracketsCount_  = 0;
      std::uint8_t squareBracketsCount_ = 0;

      // The current Declaration parent, which is either a 
      // DeclContext or a FuncDecl.
      Decl::Parent curParent_;

      Decl::Parent getDeclParent() const {
        return curParent_;
      }

      bool isDone() const;
      bool isAlive() const;

      // Stops the parsing
      void die();

      //----------------------------------------------------------------------//
      // Private parser objects
      //----------------------------------------------------------------------//

      //---------------------------------//
      // RAIIDeclParent
      //
      // This class sets the current DeclParent at construction, 
			// and restores the last one at destruction.
      // If the undo parent wasn't null and the new parent passed
      // to the constructor is a DeclContext, set the parent of the
      // DC passed to the constructor to the last one active.
      //---------------------------------//
      class RAIIDeclParent {
        public:
          RAIIDeclParent(Parser *p, Decl::Parent parent);
          // Restores the origina DeclContext early, instead of waiting
          // for the destruction of this object.
          void restore();
          ~RAIIDeclParent();
        private:
          Parser* parser_;
          Decl::Parent lastParent_;
      };
      
      //----------------------------------------------------------------------//
      // Parser constants
      //----------------------------------------------------------------------//
      
      static constexpr uint8_t 
			maxBraceDepth_ = (std::numeric_limits<std::uint8_t>::max)();

    public:
      //----------------------------------------------------------------------//
      // Result class
      //----------------------------------------------------------------------//

      // Class for encapsulating a parsing function's result.
      // It also stores a SourceRange to store a Position/Range if needed.
      template<typename DataTy>
      class Result : public ParserResultObject<DataTy> {
        using Inherited = ParserResultObject<DataTy>;
        public:
          Result() : Inherited(ResultKind::Error) {}

          explicit Result(typename Inherited::CTorValueTy val, 
						SourceRange range = SourceRange()):
            Inherited(ResultKind::Success, val), range_(range) {

          }

          explicit Result(typename Inherited::CTorRValueTy val, 
						SourceRange range = SourceRange()):
            Inherited(ResultKind::Success, val), range_(range) {

          }

          explicit operator bool() const {
            return Inherited::getResultKind() == ResultKind::Success;
          }

          using Inherited::ParserResultObject;

          SourceRange getRange() const {
            return range_;
          }

          static Result<DataTy> Error() {
            return Result<DataTy>(ResultKind::Error);
          }

          static Result<DataTy> NotFound() {
            return Result<DataTy>(ResultKind::NotFound);
          }

          // Extra function for Result<Type>, which creates a TypeLoc from
          // a Type stored in the ResultObject and it's range.
          template<typename Foo = DataTy>
          auto createTypeLoc() const -> typename 
            std::enable_if<std::is_same<Type, Foo>::value, TypeLoc>::type {
            return TypeLoc(Inherited::get(), range_);
          }

        private:
          SourceRange range_;
      };
  };
}
