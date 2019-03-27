//----------------------------------------------------------------------------//
// Part of the Fox project, licensed under the MIT license.
// See LICENSE.txt in the project root for license information.     
// File : Parser.hpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//
// This file implements the recursive descent parser for Fox.
//                              
// The grammar can be found in  /doc/ 
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
  class Lexer;
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
      Parser(ASTContext& astctxt, Lexer& lex, UnitDecl* parentUnit = nullptr);

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
      // This is a the same as ctxt.diagEngine
      DiagnosticEngine& diagEngine;

      // The SourceManager, use to retrieve source information
      // This is a the same as ctxt.sourceMgr
      SourceManager& srcMgr;

      // The Lexer instance tied to the Parser
      Lexer& lexer;

    private:
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

      // This should always be called after successfully parsing a decl.
      //
      // This method will handle registration of the decl if possible,
      // or it'll add it to the current instance of DelayedDeclRegistration.
      void finishDecl(Decl* decl);

      // This is a method called by DelayedDeclRegistration and
      // finishDecl to perform the actual Decl registration.
      // 
      // It should never be called by parsing methods directly.
      // Always use finishDecl instead!
      void registerDecl(Decl* decl, ScopeInfo scopeInfo);

      DeclContext* getCurrentDeclCtxt() const;

      //---------------------------------//
      // Token consumption
      //---------------------------------//

      // Returns true if the current token is an identifier
      bool isCurTokAnIdentifier() const;

      // Consumes an identifier, returning the Identifier object
      // and the range of the token.
      // The current token must be of the correct kind.
      //
      // isCurTokAnIdentifier() must return true.
      std::pair<Identifier, SourceRange> consumeIdentifier();

      // Consumes the current token, correctly updating counters
      // if needed.
      // 
      // Returns the range of the token.
      SourceRange consume();

      // Consumes the current tok iff (tok.kind == kind)
      //
      // Returns a valid SourceRange on success, false otherwise.
      SourceRange tryConsume(TokenKind kind);

      // NOTE: This has been removed because most users know what kind of token
      //       they want to unconsume and none actually need to unconsume 
      //       bracket/braces/paren so undo() works just fine for the current
      //       users. Rewrite this if needed. Take paren/brace/bracket balancing 
      //       in account!
      // Reverts the last consume operation, updating counters if needed.
      // void unconsume();

      // Increments the iterator if possible. Used to skip a token 
      // without updating any counters.
      void next();

      // Decrements the iterator if possible. Used to go back to the last token
      // without updating counters.
      void undo();  

      // Returns the current token (*tokenIterator_) without incrementing
      // the current iterator.
      Token getCurtok() const;
      
      // Returns the previous token (*(--tokenIterator)) without decrementing
      // the current iterator.
      Token getPreviousToken() const;
      
      //---------------------------------//
      // Resynchronization helpers
      //---------------------------------//

      bool resyncTo(TokenKind kind, bool stopAtSemi, bool shouldConsumeToken);
      bool resyncTo(const SmallVector<TokenKind, 4>& kinds, bool stopAtSemi,
				            bool shouldConsumeToken);
      bool resyncToNextDecl();

      //---------------------------------//
      // Error reporting
      //---------------------------------//

      TokenVector& getTokens();
      const TokenVector& getTokens() const;

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

          RAIIDeclCtxt(const RAIIDeclCtxt&) = delete;
          RAIIDeclCtxt& operator=(const RAIIDeclCtxt&) = delete;
        private:
          Parser* parser_ = nullptr;
          DeclContext* lastDC_ = nullptr;
      };

      //---------------------------------//
      // DelayedDeclRegistration 
      // 
      // This class represents a "transaction". It is used
      // to delay calls to finishDecl until the transaction
      // is completed or abandoned. 
      //
      // This class solves a very important problem: ScopeInfo
      // of CompoundStmts: To correctly parse the Decls inside
      // a CompoundStmt, I need to have ScopeInfo of the CompoundStmt,
      // but to create the CompoundStmt, I need to parse it all since
      // it uses trailing objects. This chicken-and-egg problem is solved
      // by this class that delays the registration of the declarations 
      // that are direct children of the CompoundStmt.
      //
      // Note: this object is relatively large (8+2 pointers) in order to
      //       minimize allocations in common cases.
      // 
      // The transaction can be completed in 3 ways:
      // - by calling "complete" with a ScopeInfo instance
      //
      // - by calling abandon(), which discards the decls.
      //
      // - by destroying this object (that calls abandon())
      //
      // NOTE: Theses aren't nested. When a DDR completes its transaction,
      //       it'll register the decls directly (registerDecl), it won't add
      //       them to the previous DDR (e.g. by calling finishDecl)
      //---------------------------------//      
      class DelayedDeclRegistration {
        public:
          DelayedDeclRegistration(Parser* p);
          ~DelayedDeclRegistration();

          // Adds a Decl to this transaction, this should only be called
          // by finishDecl when it notices that a transaction is currently
          // active.
          void addDecl(Decl* decl);

          // Abandons this transaction, discarding the stored decls.
          void abandon();

          // Completes this 'transaction', registering the stored decls using
          // the ScopeInfo passed as parameter.
          void complete(ScopeInfo scope);

          DelayedDeclRegistration(const DelayedDeclRegistration&) = delete;
          DelayedDeclRegistration& 
          operator=(const DelayedDeclRegistration&) = delete;
        private:
          // The parser instance, or nullptr if the transaction
          // has already been completed.
          Parser* parser_ = nullptr;

          // The pending decls
          SmallVector<Decl*, 8> decls_;
          
          // The previous curDDR_, if there's one.
          DelayedDeclRegistration* prevCurDDR_ = nullptr;
      };

      DelayedDeclRegistration* curDDR_ = nullptr;

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
