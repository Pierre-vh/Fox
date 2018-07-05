////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : ASTDumper.hpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
// This file contains the "ASTDump" class, which is used to 
// Dump the ast to any ostream (often std::cout)
////------------------------------------------------------////

#pragma once
#include "ASTVisitor.hpp"
#include <ostream>

namespace fox
{
	class SourceManager;
	class ASTDumper : public ASTVisitor<ASTDumper, void>
	{
		public:
			// The first parameter is the stream where the AST should be "dumped"
			// The second is the offset. Before each line, '\t' is printed <offset> times. 
			ASTDumper(SourceManager& srcMgr,std::ostream& out, const uint8_t& offsettabs = 0);

			// Expressions
			void visitParensExpr(ParensExpr* node);
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
			void visitBooleanLiteralExpr(BoolLiteralExpr* node);
			void visitStringLiteralExpr(StringLiteralExpr* node);
			void visitArrayLiteralExpr(ArrayLiteralExpr* node);

			// Stmts
			void visitNullStmt(NullStmt* node);
			void visitCompoundStmt(CompoundStmt* node);
			void visitConditionStmt(ConditionStmt* node);
			void visitWhileStmt(WhileStmt* node);
			void visitDeclStmt(DeclStmt* node);
			void visitReturnStmt(ReturnStmt* node);

			// Decls
			void visitUnitDecl(UnitDecl* node);
			void visitVarDecl(VarDecl* node);
			void visitArgDecl(ArgDecl* node);
			void visitFunctionDecl(FunctionDecl* node);

			// Options
			void setPrintAllAddresses(const bool& opt);
			bool getPrintAllAddresses() const;

			void setDumpOperatorsAsNames(const bool& opt);
			bool getDumpOperatorsAsNames() const;

		private:
			void initDefaultOptions();

			// Prints getOffset() and getIndent() to out_ then returns out_
			// Can add a number as parameter to add a "temporary" indent, just for this line.
			std::ostream& dumpLine(const uint8_t& num = 0);

			// sets offset_ to the correct number of tabs required.
			void recalculateOffset();

			// returns the indent required by curIndent_
			std::string getIndent(const uint8_t& num = 0) const;

			// Returns the name of the node by using it's pointer
			std::string getStmtNodeName(Stmt* stmt) const;
			std::string getDeclNodeName(Decl* decl) const;
			std::string getTypeNodeName(Type* type) const;

			// Returns a string containing basic information about a node : It's name followed by it's adress. Maybe more in the future.
			std::string getBasicStmtInfo(Stmt* stmt) const;
			std::string getBasicDeclInfo(Decl* decl) const;
			std::string getBasicTypeInfo(Type* type) const;

			// Dump an operator in 2 different ways, depending on dumpOperatorsAsNames_
			// if dumpOperatorsAsNames_ = true, returns e.g. "ADDITION" instead of '+'
			std::string getOperatorDump(const BinaryOperator& op) const;
			std::string getOperatorDump(const UnaryOperator& op) const;

			// Returns a formatted string "<DeclRecorder (adress), Parent: (adress)>"
			std::string getDeclRecorderDump(DeclRecorder *dr) const;
			// Returns a formatted string, "<ID:(idstring)>"
			std::string getIdentifierDump(IdentifierInfo *id) const;
			// Returns a formatted string, "<(label):(coords)>"
			std::string getSourceLocDump(const std::string& label,const SourceLoc& sloc) const;
			// Returns a formatted string  "<(label):'(type)'>
			std::string getTypeDump(const std::string& label,Type *ty) const;
			std::string getQualTypeDump(const std::string& label,const QualType& qt) const;
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


			void indent(const uint8_t& num = 1);
			void dedent(const uint8_t& num = 1);

			std::ostream& out_;
			SourceManager& srcMgr_;
			std::string offset_;
			uint16_t curIndent_ = 0, offsetTabs_ = 0;

			// Options
			bool printAllAdresses_ : 1;
			bool dumpOperatorsAsNames_ : 1;
	};
}