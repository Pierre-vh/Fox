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
#include "Fox/Common/LLVM.hpp"
#include "llvm/ADT/Optional.h"
#include <tuple>

namespace fox {
  class ASTContext;
  class SourceManager;
  class DeclContext;
  class DiagnosticEngine;
  class Diagnostic;
  class ASTNode;
  enum class DiagID : std::uint16_t;
  class Parser {
    public:
      enum class ResultKind : std::uint8_t {
        Success, Error, NotFound
        // There's still room for one more ParserResultKind. If more is 
        // added, update bitsForPRK in the detail namespace below
        // the Parser class.
      };

      template<typename DataTy>
      class Result;

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
      Result<Expr*> parseBinaryExpr(unsigned precedence = 5);
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
      //
      // Note that this method will not register the ParamDecl in any
      // DeclContext. It'll simply set the DeclContext to nullptr.
      Result<Decl*> parseParamDecl();

      //---------------------------------//
      // Type parsing helpers
      //---------------------------------//

      // Parses a builtin type name
      Result<TypeLoc> parseBuiltinTypename();

      // Parses a complete type e.g. [[float]]
      Result<TypeLoc> parseType();

      //---------------------------------//
      // Operators parsing helpers
      //---------------------------------//

      // Theses methods return a Result object that
      // doesn't contain the SourceRange. For now, the workaround I use is 
      // to ask for a SourceRange& as param, and I place the SourceRange 
      // there on success. This workaround will go away with the lexer rework,
      // because operators won't be parsed anymore, they'll be handled
      // by the lexer directly.

      // Parses any assignment operator. The SourceRange of the operator
      // is placed in "range" if the parsing finishes successfully.
      Result<BinaryExpr::OpKind> parseAssignOp(SourceRange& range);

      // Parses any unary operator. The SourceRange of the operator
      // is placed in "range" if the parsing finishes successfully.
      Result<UnaryExpr::OpKind> parseUnaryOp(SourceRange& range);
      
      // Parses any binary operator. The SourceRange of the operator
      // is placed in "range" if the parsing finishes successfully.
      Result<BinaryExpr::OpKind> 
      parseBinaryOp(unsigned priority, SourceRange& range);

      SourceRange parseExponentOp();

      //---------------------------------//
      // Current Decl Parent (curParent_) helpers
      //---------------------------------//

      // Should be called whenever a decl is done parsing and about 
      // to be returned.
      void actOnDecl(Decl* decl);

      DeclContext* getCurrentDeclCtxt() const;

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
      //
      // Returns a pair of the Identifier + the SourceRange of the Identifier
      // on success.
      Optional<std::pair<Identifier, SourceRange>>
      consumeIdentifier();

      // Consumes any sign but brackets.
      //
      // Returns a valid SourceLoc if the token was consumed successfully.
      SourceLoc consumeSign(SignType s);

      // Consumes a bracket and keeps the bracket count up to date.
      // Returns an invalid SourceLoc if the bracket was not found.
      // Note : In the US, a Bracket is a [], however, here the bracket noun 
      // is used in the strict sense, where 
      // Round B. = (), Square B. = [] and Curly B. = {}
      //
      // Returns a valid SourceLoc if the token was consumed successfully.
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

      DeclContext* curDeclCtxt_ = nullptr;

      bool isDone() const;
      bool isAlive() const;

      // Stops the parsing
      void die();

      //----------------------------------------------------------------------//
      // Private parser objects
      //----------------------------------------------------------------------//

      //---------------------------------//
      // RAIIDeclCtxt
      //
      // This class sets the current DeclContext at construction, 
			// and restores the last one at destruction.
      // If the previous parent wasn't null and the new parent passed
      // to the constructor is a DeclContext, set the parent of the
      // DC passed to the constructor to the last one active.
      // (TL;DR: It automatically handles "parent" registration)
      //---------------------------------//
      class RAIIDeclCtxt {
        public:
          RAIIDeclCtxt(Parser *p, DeclContext* dc);
          // Restores the original DeclContext early, instead of waiting
          // for the destruction of this object.
          void restore();
          ~RAIIDeclCtxt();

        private:
          Parser* parser_ = nullptr;
          DeclContext* lastDC_ = nullptr;
      };
      
      //----------------------------------------------------------------------//
      // Parser constants
      //----------------------------------------------------------------------//
      
      static constexpr uint8_t 
			maxBraceDepth_ = (std::numeric_limits<std::uint8_t>::max)();
  };

  namespace detail {
    constexpr unsigned bitsForPRK = 2;

    template<typename DataTy>
    struct IsEligibleForPointerIntPairStorage {
      static constexpr bool value = false;
    };

    template<typename DataTy>
    struct IsEligibleForPointerIntPairStorage<DataTy*> {
      static constexpr bool value = alignof(DataTy) >= bitsForPRK;
    };

    template<typename DataTy, bool canUsePointerIntPair = 
      IsEligibleForPointerIntPairStorage<DataTy>::value>
    class ParserResultObjectDataStorage {
      DataTy data_ = DataTy();
      Parser::ResultKind kind_;
      public:
        using value_type = DataTy;

        static value_type getDefaultValue() {
          return value_type();
        }

        ParserResultObjectDataStorage(const value_type& data, 
                                      Parser::ResultKind kind):
          data_(data), kind_(kind) {}

        ParserResultObjectDataStorage(value_type&& data, 
                                      Parser::ResultKind kind):
          data_(data), kind_(kind) {}

        value_type data() {
          return data_;
        }

        const value_type data() const {
          return data_;
        }

        value_type&& move() {
          return std::move(data_);
        }

        Parser::ResultKind kind() const {
          return kind_;
        }
    };

    template<typename DataTy>
    class ParserResultObjectDataStorage<DataTy*, true> {
      llvm::PointerIntPair<DataTy*, bitsForPRK, Parser::ResultKind> pair_;
      public:
        using value_type = DataTy*;

        static value_type getDefaultValue() {
          return nullptr;
        }

        ParserResultObjectDataStorage(DataTy* data, Parser::ResultKind kind):
          pair_(data, kind) {}

        DataTy* data() {
          return pair_.getPointer();
        }

        const DataTy* data() const {
          return pair_.getPointer();
        }

        Parser::ResultKind kind() const {
          return pair_.getInt();
        }
    };
  }

  template<typename DataTy>
  class Parser::Result {
    using StorageType = detail::ParserResultObjectDataStorage<DataTy>;
    protected:
      using DefaultValue = DataTy;
      using CTorValueTy = const DataTy&;
      using CTorRValueTy = DataTy && ;
      static constexpr bool isPointerType = std::is_pointer<DataTy>::value;

    public:
      Result() : storage_(DefaultValue(), ResultKind::Error) {}

      explicit Result(const DataTy& data, 
                      ResultKind kind = ResultKind::Success):
        storage_(data, kind) {}

      template<typename Ty = DataTy, 
               typename = typename std::enable_if<!isPointerType, Ty>::type>
      explicit Result(Ty&& data, ResultKind kind = ResultKind::Success):
        storage_(data, kind) {}

      explicit Result(ResultKind kind) :
        storage_(StorageType::getDefaultValue(), kind) {}

      bool isSuccess() const {
        return getResultKind() == ResultKind::Success;
      }

      bool isNotFound() const {
        return getResultKind() == ResultKind::NotFound;
      }

      bool isError() const {
        return getResultKind() == ResultKind::Error;
      }

      ResultKind getResultKind() const {
        return storage_.kind();
      }

      DataTy get() {
        return storage_.data();
      }

      const DataTy get() const {
        return storage_.data();
      }
      
      explicit operator bool() const {
        return getResultKind() == ResultKind::Success;
      }

      static Result<DataTy> Error() {
        return Result<DataTy>(ResultKind::Error);
      }

      static Result<DataTy> NotFound() {
        return Result<DataTy>(ResultKind::NotFound);
      }
      
      template<typename Ty>
      auto castTo() -> typename std::enable_if<isPointerType, Ty*>::type {
        DataTy ptr = storage_.data();
        assert(ptr && "Can't use this on a null pointer");
        return cast<Ty>(ptr);
      }

      template<typename RtrTy = DataTy, 
               typename = typename std::enable_if<!isPointerType, RtrTy>::type>
      RtrTy&& move() {
        return storage_.move();
      }

    private:
      StorageType storage_;
  };
}
