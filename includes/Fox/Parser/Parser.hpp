//----------------------------------------------------------------------------//
// This file is a part of The Moonshot Project.        
// See the LICENSE.txt file at the root of the project for license information.            
// File : Parser.hpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//
// This file implements the recursive descent parser.    
// The parser is implemented as a set of functions, each  
// "parseXXX" method represents a rule in the grammar.        
//                              
// The grammar can be found in  /doc/                                    
//
// Terminology :
//      Parens always mean Round Brackets only.
//      Brackets always mean Round/Curly/Square Bracket (Every kind of bracket)
// 
// Status: Up to date with latest grammar changes, except import/using rules that 
//				 aren't implemented yet.
//
//    Improvements Ideas:
//      Recovery Efficiency
//        Tweak it by running different test situations and adding 
//				special recovery cases wherever needed.
//      Speed
//      Try to cut as many includes as possible.
//
//
//    Parser "to-do" list. Important stuff is marked with (*)
//      Add better error recovey with common cases support in
//			if/while parsing & function declaration
//
//      Add a way for the parser to ignore comment tokens.
//
//----------------------------------------------------------------------------//

#pragma once

#include "Fox/Lexer/Token.hpp"          
#include "Fox/AST/Type.hpp"
#include "Fox/AST/Decl.hpp"
#include "Fox/Parser/Scope.hpp"
#include "Fox/AST/Expr.hpp"
#include "Fox/AST/Stmt.hpp"
#include "Fox/Common/ResultObject.hpp"

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
    public:
      /*-------------- Forward Declarations --------------*/
      template<typename DataTy>
      class Result;

      /*-------------- Type Aliases --------------*/
      using ExprResult = Result<Expr*>;
      using DeclResult = Result<Decl*>;
      using StmtResult = Result<Stmt*>;
      using NodeResult = Result<ASTNode>;
      using UnitResult = Result<UnitDecl*>;

    private:
      using TokenIteratorTy = TokenVector::iterator;

    public:
      Parser(DiagnosticEngine& diags, SourceManager &sm, 
        ASTContext& astctxt, TokenVector& l, DeclContext* dr = nullptr);

      /*-------------- Parsing Methods --------------*/
			// UNIT
      UnitDecl* parseUnit(FileID fid, Identifier unitName, bool isMainUnit);

      // EXPRESSIONS
      Result<ExprVector> parseExprList();

      Result<ExprVector> 
			parseParensExprList(SourceLoc* LParenLoc = nullptr, 
				SourceLoc *RParenLoc = nullptr);

      ExprResult 
			parseParensExpr(SourceLoc* leftPLoc = nullptr, 
			SourceLoc* rightPLoc = nullptr);

      ExprResult parseSuffix(Expr* base);
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
      NodeResult parseExprStmt();
      StmtResult parseCompoundStatement();
      NodeResult parseStmt();
      NodeResult parseBody();
      StmtResult parseCondition();
      StmtResult parseWhileLoop();

      // DECLS
      DeclResult parseParamDecl();
      DeclResult parseVarDecl();
      DeclResult parseFuncDecl();
      DeclResult parseDecl();

      // Getters
      ASTContext& getASTContext();
      SourceManager& getSourceManager();
      DiagnosticEngine& getDiagnosticEngine();

    private:
      /*-------------- Parser Setup --------------*/
      void setupParser();

      /*-------------- "Basic" Parse Methods --------------*/
      // Parses a builtin type name
      // Parser::Result::getRange does not contain the range, use
      // the TypeLoc's getRange method to retrieve the range.
      Result<Type> parseBuiltinTypename();

      // Parses a complete type e.g. int[], &float[][]
      // Parser::Result::getRange does not contain the range, use
      // the TypeLoc's getRange method to retrieve the range.
      Result<Type> parseType();

      // Parses a QualType 
      struct ParsedQualType {
        Type type;
        bool isConst = false;
        bool isRef = false;
      };
      Result<ParsedQualType> 
			parseQualType(SourceRange* constLoc = nullptr, SourceLoc* refLoc = nullptr);

      Result<BinaryExpr::OpKind> parseAssignOp();
      Result<UnaryExpr::OpKind> parseUnaryOp();
      Result<BinaryExpr::OpKind> parseBinaryOp(std::uint8_t priority);
      SourceRange parseExponentOp();

      /*-------------- Token Consuming --------------*/
      /*  
        Consume methods all return a result that evaluates to true 
				if the "consume" operation finished successfully 
        (found the requested token), false otherwise

        Note: SourceLocs and SourceRanges can be both evaluated in 
				a condition to check their validity (operator bool is implemented on both)
      */

      // Consumes an Identifier
      // The Result object will contain the SourceRange of the identifier 
			// on a success
      Result<Identifier> consumeIdentifier();

      // Consumes any sign but brackets.
      SourceLoc consumeSign(SignType s);

      // Consumes a bracket and keeps the bracket count up to date. Returns an invalid SourceLoc if the bracket was not found.
      // Note : In the US, a Bracket is a [], however, here the bracket noun is used in the strict sense, where Round B. = (), Square B. = [] and Curly B. = {}
      SourceLoc consumeBracket(SignType s);

      // Consumes a keyword. Returns an invalid SourceRange if not found.
      SourceRange consumeKeyword(KeywordType k);

      // Dispatch to the appriate consume method. Won't return any loc information.
      // Used to skip a token, updating any necessary counters.
      void consumeAny();

      // Reverts the last consume operation, updates counters if needed.
      void revertConsume();

      // Increments the iterator if possible. Used to skip a token without updating any counters.
      void next();

      // Decrements the iterator if possible. Used to revert a consume operation. 
			// Won't change updated counters.
      // Only use in cases where a counter wasn't updated by the last consume operation. 
			// Else, use a Parser State Backup.
      void previous();  

      // Helper for consumeSign & consumeBracket
      // Brackets are one of the following : '(' ')' '[' ']' '{' '}'
      bool isBracket(SignType s) const;

      Token getCurtok() const;
      Token getPreviousToken() const;
      
      /*-------------- Error Recovery --------------*/
        // Last Parameter is an optional pointer to a SourceRange. 
				// If the recovery was successful, the SourceRange of the token found
        // will be saved there.
      bool resyncToSign(SignType sign, bool stopAtSemi, bool shouldConsumeToken);
      bool resyncToSign(const std::vector<SignType>& signs, bool stopAtSemi,
				bool shouldConsumeToken);
      bool resyncToNextDecl();

      /*-------------- Error Reporting --------------*/
      // Reports an error of the "unexpected" family.
      // The SourceLoc of the error is right past the end of the previous token.
      Diagnostic reportErrorExpected(DiagID diag);

      /*-------------- Parser State --------------*/
      struct ParserState {
        ParserState();
        
        // The current token
        TokenIteratorTy tokenIterator;

				// This is set to false when the parser dies (gives up)
        bool isAlive : 1;
      
        // Brackets counters
        std::uint8_t curlyBracketsCount  = 0;
        std::uint8_t roundBracketsCount  = 0;
        std::uint8_t squareBracketsCount = 0;

        // Current Decl Recorder
        DeclContext* declContext = nullptr;

        // The current scope
        Scope* scope = nullptr;
      } state_;

      // Interrogate state_
      bool isDone() const;
      bool isAlive() const;

      // Stops the parsing
      void die();

      // Register a declaration in state_.declContext, asserting that it's not null.
      void recordDecl(NamedDecl *nameddecl);

      // Creates a state_ backup
      ParserState createParserStateBackup() const;
      // Restores state_ from a backup.
      void restoreParserStateFromBackup(const ParserState& st);

      /*-------------- RAIIDeclContext --------------*/
      // This class sets the current DeclContext at construction, 
			// and restores the last one at destruction.
      // If the DeclContext that was here before isn't null,
			// it's marked as being the parent of the DeclContext passed 
			// as argument to the constructor.
      // It assists in registering Decl in the appropriate DeclContext.
      class RAIIDeclContext {
        public:
          RAIIDeclContext(Parser &p,DeclContext *dr);
          ~RAIIDeclContext();
        private:
          Parser& parser_;
          DeclContext* declCtxt_ = nullptr;
      };

      /*-------------- RAIIScope --------------*/
      // This class creates and own a Scope, and restores the old
      // scope upon destruction.
      class RAIIScope {
        public:
          RAIIScope(Parser& p);
          ~RAIIScope();
        private:
          Parser& parser_;
          std::unique_ptr<Scope> scope_;
      };

      /*-------------- Member Variables --------------*/
      ASTContext& ctxt_;
      DiagnosticEngine& diags_;
      SourceManager& srcMgr_;
      TokenVector& tokens_;
      
      /*-------------- Constants --------------*/
      static constexpr uint8_t 
			maxBraceDepth_ = (std::numeric_limits<std::uint8_t>::max)();

    public:
      /*-------------- Result Classes --------------*/
      // Class for encapsulating a parsing function's result.
      // It also stores a SourceRange to store a Position/Range if needed.
      template<typename DataTy>
      class Result : public ResultObject<DataTy> {
        using Inherited = ResultObject<DataTy>;
        public:
          explicit Result(bool success = true):
            Inherited(success) {

          }

          explicit Result(Inherited::CTorValueTy val, 
						SourceRange range = SourceRange()):
            Inherited(true, val), range_(range) {

          }

          explicit Result(Inherited::CTorRValueTy val, 
						SourceRange range = SourceRange()):
            Inherited(true, val), range_(range) {

          }

          bool isUsable() const {
            return Inherited::hasData() && Inherited::wasSuccessful();
          }

          explicit operator bool() const {
            return isUsable();
          }

          using Inherited::ResultObject;

          SourceRange getRange() const {
            return range_;
          }

          static Result<DataTy> Error() {
            return Result<DataTy>(false);
          }

          static Result<DataTy> NotFound() {
            return Result<DataTy>(true);
          }

          // Extra function for Result<Type>, which creates a TypeLoc from
          // a Type stored in the ResultObject and it's range.
          template<
						typename = typename 
						std::enable_if<std::is_same<Type, DataTy>::value, TypeLoc>::type>
          TypeLoc createTypeLoc() const {
            return TypeLoc(Inherited::get(), range_);
          }

        private:
          SourceRange range_;
      };
  };
}
