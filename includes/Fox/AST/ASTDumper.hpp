//----------------------------------------------------------------------------//
// This file is a part of The Moonshot Project.				
// See the LICENSE.txt file at the root of the project for license information.						
// File : ASTDumper.hpp											
// Author : Pierre van Houtryve								
//----------------------------------------------------------------------------//
// This file contains the "ASTDumper" class, which is used to 
// print the ast to any ostream.
//----------------------------------------------------------------------------//

#pragma once
#include "ASTVisitor.hpp"
#include <ostream>

namespace fox
{
	class SourceManager;
	class ASTDumper : public SimpleASTVisitor<ASTDumper, void>
	{
		public:
			// The first parameter is the stream where the AST should be "dumped"
			// The second is the offset. Before each line, '\t' is printed <offset> times. 
			ASTDumper(SourceManager& srcMgr, std::ostream& out, const uint8_t& offsettabs = 0);

			// Expressions
			void visitBinaryExpr(BinaryExpr* node);
			void visitCastExpr(CastExpr* node);
			void visitUnaryExpr(UnaryExpr* node);
			void visitArrayAccessExpr(ArrayAccessExpr* node);
			void visitMemberOfExpr(MemberOfExpr* node);
			void visitDeclRefExpr(DeclRefExpr* node);
			void visitFunctionCallExpr(FunctionCallExpr* node);

			// Expressions : Literals
			void visitCharLiteralExpr(CharLiteralExpr* node);
			void visitIntegerLiteralExpr(IntegerLiteralExpr* node);
			void visitFloatLiteralExpr(FloatLiteralExpr* node);
			void visitBoolLiteralExpr(BoolLiteralExpr* node);
			void visitStringLiteralExpr(StringLiteralExpr* node);
			void visitArrayLiteralExpr(ArrayLiteralExpr* node);

			// Stmts
			void visitNullStmt(NullStmt* node);
			void visitCompoundStmt(CompoundStmt* node);
			void visitConditionStmt(ConditionStmt* node);
			void visitWhileStmt(WhileStmt* node);
			void visitReturnStmt(ReturnStmt* node);

			// Decls
			void visitUnitDecl(UnitDecl* node);
			void visitVarDecl(VarDecl* node);
			void visitParamDecl(ParamDecl* node);
			void visitFuncDecl(FuncDecl* node);

			// Options
			void setPrintAllAddresses(bool opt);
			bool getPrintAllAddresses() const;

		private:
			void initDefaultOptions();

			// Prints getOffset() and getIndent() to out_ then returns out_
			// Can add a number as parameter to add a "temporary" indent, just for this line.
			std::ostream& dumpLine(std::uint8_t num = 0);

			// sets offset_ to the correct number of tabs required.
			void recalculateOffset();

			// returns the indent required by curIndent_
			std::string getIndent(const uint8_t& num = 0) const;

			// Returns the name of the node by using it's pointer
			std::string getStmtNodeName(Stmt* stmt) const;
			std::string getExprNodeName(Expr* expr) const;
			std::string getDeclNodeName(Decl* decl) const;
			std::string getTypeNodeName(TypeBase* type) const;

			// Returns a string containing basic information about a node : It's name followed by it's adress. Maybe more in the future.
			std::string getBasicStmtInfo(Stmt* stmt) const;
			std::string getBasicExprInfo(Expr* expr) const;
			std::string getBasicDeclInfo(Decl* decl) const;
			std::string getBasicTypeInfo(TypeBase* type) const;
			std::string getBasicValueDeclDump(ValueDecl* decl) const;

			// Dump an operator in 2 different ways, depending on dumpOperatorsAsNames_
			std::string getOperatorDump(BinaryExpr::OpKind op) const;
			std::string getOperatorDump(UnaryExpr::OpKind op) const;

			// Returns a formatted string "<DeclContext (adress), Parent: (adress)>"
			std::string getDeclRecorderDump(DeclContext* dr) const;
			// Returns a formatted string, "<ID:(idstring)>"
			std::string getIdentifierDump(Identifier* id) const;
			// Returns a formatted string, "<(label):(coords)>"
			std::string getSourceLocDump(const std::string& label, SourceLoc sloc) const;
			std::string getSourceRangeDump(const std::string& label, SourceRange range) const;

			std::string getSourceRangeAsStr(SourceRange range) const;
			// Returns a formatted string  "<(label):'(type)'>
			std::string getTypeDump(const std::string& label, Type ty, bool isConst = false) const;
			std::string getTypeLocDump(const std::string& label, TypeLoc ty, bool isConst = false) const;
			// Returns value enclosed with "".
			std::string addDoubleQuotes(const std::string& str) const;
			// Returns the value enclosed with ''
			std::string addSingleQuotes(const std::string& str) const;

			// Returns a formatted string "<(label):(value)>
			template<typename TyA,typename TyB>
			std::string makeKeyPairDump(TyA label,TyB value) const
			{
				std::ostringstream ss;
				ss << "<" << label << ":" << value << ">";
				return ss.str();
			}


			void indent(std::uint8_t num = 1);
			void dedent(std::uint8_t num = 1);

			std::ostream& out_;
			SourceManager& srcMgr_;
			std::string offset_;
			uint16_t curIndent_ = 0, offsetTabs_ = 0;

			// Options
			bool printAllAdresses_ : 1;
	};
}