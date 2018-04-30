////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : Dumper.cpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
//			SEE HEADER FILE FOR MORE INFORMATION			
////------------------------------------------------------////

#include "Dumper.hpp"
#include "Moonshot/Fox/Common/Exceptions.hpp"
#include "Moonshot/Fox/Lexer/StringManipulator.hpp"
#include "Moonshot/Fox/AST//IdentifierTable.hpp"
#include "Moonshot/Fox/AST/Expr.hpp"
#include "Moonshot/Fox/AST/Decl.hpp"
#include "Moonshot/Fox/AST/Stmt.hpp"
#include "Moonshot/Fox/Common/Utils.hpp"

using namespace Moonshot;

Dumper::Dumper(std::ostream & outstream, const unsigned char& offsettabs) : out_(outstream), offsetTabs_(offsettabs)
{

}

void Dumper::visit(UnitDecl & node)
{
	if (node.isValid())
	{
		out_ << getIndent() << "ASTUnit \"" << node.getIdentifier()->getStr() << "\" containing " << node.getDeclCount() << " declaration.\n";
		curindent_++;
		for (auto it = node.decls_beg(); it != node.decls_end(); it++)
			it->accept(*this);

		curindent_--;
		out_ << getIndent() << "This unit recorded " << node.getNumberOfRecordedDecls() << " declarations ";
		if (node.hasParentDeclRecorder())
		{
			if (auto ptr = dynamic_cast<NamedDecl*>(node.getParentDeclRecorder()))
				out_ << "(It has a parent DeclRecorder named " << ptr->getIdentifier()->getStr() << ")";
			else 
				out_ << "(It has a parent DeclRecorder)";
		}
		out_ << "\n";
		curindent_++;
		for (auto it = node.recordedDecls_begin(); it != node.recordedDecls_end(); it++)
		{
			out_ << getIndent() << "> Declaration with name: " << it->first->getStr() << "\n";
		}
		curindent_--;
	}
	else
		out_ << getIndent() << "Invalid ASTUnit\n";
}

void Dumper::visit(BinaryExpr & node)
{
	std::string op = Util::getFromDict(Dicts::kBinopToStr_dict, node.getOp());
	if (op.size() == 0)
		op = std::to_string(Util::enumAsInt(node.getOp()));

	out_ << getIndent() << "BinaryExpression : Operator " << op;

	// newline
	out_ << "\n";

	if (!node.isComplete())
	{
		throw Exceptions::ast_malformation("BinaryExpression node did not have a left and right child.");
		return;
	}
	else
	{
		// PRINT LEFT CHILD
		curindent_++;
		out_ << getIndent() << "Left child:\n";
		curindent_++;
		node.getLHS()->accept(*this);
		curindent_ -= 2;
		// PRINT RIGHT CHILD
		curindent_++;
		out_ << getIndent() << "Right child:\n";
		curindent_++;
		node.getRHS()->accept(*this);
		curindent_ -= 2;
	}
	
}

void Dumper::visit(UnaryExpr & node)
{
	std::string op = Util::getFromDict(Dicts::kUnaryOpToStr_dict, node.getOp());
	if (op.size() == 0)
		op = std::to_string(Util::enumAsInt(node.getOp()));

	out_ << getIndent() << "UnaryExpression : Operator " << op << "\n";

	curindent_++;
	out_ << getIndent() << "Child:\n";
	curindent_++;

	if (!node.getChild())
	{
		throw Exceptions::ast_malformation("UnaryExpression node did not have a child.");
		return;
	}

	node.getChild()->accept(*this);
	curindent_ -= 2;
}

void Dumper::visit(CastExpr & node)
{
	out_ << getIndent() << "CastExpression : Cast Goal:" << node.getCastGoal()->getString() << "\n";
	curindent_++;
	out_ << getIndent() << "Child:\n";
	curindent_++;

	if (!node.getChild())
	{
		throw Exceptions::ast_malformation("CastExpression node did not have a child.");
		return;
	}

	node.getChild()->accept(*this);
	curindent_ -= 2;
}
void Dumper::visit(CharLiteralExpr & node)
{
	std::string str;
	UTF8::StringManipulator::append(str, node.getVal());
	out_ << getIndent() << "Char Literal: (" << node.getVal() << ")->'" << str << "'\n";
}
void Dumper::visit(IntegerLiteralExpr & node)
{
	out_ << getIndent() << "Int Literal: " << node.getVal() << '\n';
}
void Dumper::visit(FloatLiteralExpr & node)
{
	out_ << getIndent() << "Float Literal: " << node.getVal() << '\n';
}
void Dumper::visit(StringLiteralExpr & node)
{
	out_ << getIndent() << "String Literal: \"" << node.getVal() << "\"\n";
}
void Dumper::visit(BoolLiteralExpr & node)
{
	out_ << getIndent() << "Bool Literal: " << (node.getVal() ? "true" : "false") << '\n';
}

void Dumper::visit(ArrayLiteralExpr & node)
{
	out_ << getIndent() << "Array Literal";
	if (!node.isEmpty() && node.hasExprList())
	{
		out_ << "(" << node.getExprList()->size() << " elements):\n";
		auto elist = node.getExprList();
		curindent_++;
		std::size_t count = 0;
		for (auto it = elist->begin(); it != elist->end(); it++)
		{
			out_ << getIndent() << "Element " << count << ":\n";
			curindent_++;
			it->accept(*this);
			curindent_--;
		}
		curindent_--;
	}
	else
		out_ << " (empty)\n";
}

void Dumper::visit(VarDecl & node)
{
	if (node.isValid())
	{
		out_ << getIndent() << "VarDeclStmt : Name:" << node.getIdentifier()->getStr() << " Type:" << node.getType().getString() << "\n";
		if (node.hasInitExpr())
		{
			curindent_ += 1;
			out_ << getIndent() << "InitExpr\n";
			curindent_ += 1;
			node.getInitExpr()->accept(*this);
			curindent_ -= 2;
		}
	}
	else
	{
		out_ << getIndent() << "Invalid VarDeclStmt\n";
	}
}

void Dumper::visit(ArrayAccessExpr & node)
{
	out_ << getIndent() << "ArrayAccess Expr:\n";
	curindent_++;
	out_ << getIndent() << "Base:\n";
	curindent_++;
	node.getBase()->accept(*this);
	curindent_--;
	out_ << getIndent() << "Index expression:\n";
	curindent_++;
	node.getAccessIndexExpr()->accept(*this);
	curindent_ -= 2;
}

void Dumper::visit(MemberOfExpr & node)
{
	out_ << getIndent() << "MemberOf id:" << node.getMemberName()->getStr() << "\n";
	curindent_++;
	out_ << getIndent() << "Base:\n";
	curindent_++;
	node.getBase()->accept(*this);
	curindent_--;
}

void Dumper::visit(DeclRefExpr & node)
{
	out_ << getIndent() << "DeclRef: " << node.getDeclIdentifier()->getStr() << std::endl;
}

void Dumper::visit(FunctionCallExpr & node)
{
	out_ << getIndent() << "Function Call\n";
	curindent_++;
	out_ << getIndent() << "Callee :\n";
	curindent_++;
	node.getCallee()->accept(*this);
	curindent_--;

	if (node.getExprList()->size())
	{
		out_ << getIndent() << "Args:\n";
		curindent_++;

		std::size_t count = 0;
		auto exprList = node.getExprList();
		for(auto it = exprList->begin(); it != exprList->end(); it++)
		{
			out_ << getIndent() << "Arg" << count << '\n';

			curindent_++;
			it->accept(*this);
			curindent_--;

			count++;
		}

		curindent_--;
	}
	curindent_--;
}

void Dumper::visit(NullExpr&)
{
	out_ << getIndent() << "Null\n";
}

void Dumper::visit(ArgDecl & node)
{
	out_ << getIndent() << "Arg Declaration: Name:" << node.getIdentifier()->getStr() << " Type:" << node.getType().getString() << "\n";
}

void Dumper::visit(FunctionDecl & node)
{
	if (node.isValid())
	{
		out_ << getIndent() << "FunctionDecl : Name:" << node.getIdentifier()->getStr() << " Return type:" << node.getReturnType()->getString() << "\n";
		curindent_ += 2;

		for (auto it = node.args_begin(); it != node.args_end(); it++)
			it->accept(*this);

		curindent_ -= 1;
		out_ << getIndent() << "Body:" << std::endl;
		curindent_ += 1;
		node.getBody()->accept(*this);
		curindent_ --;


		out_ << getIndent() << "This Function Declaration recorded " << node.getNumberOfRecordedDecls() << " declarations ";
		if (node.hasParentDeclRecorder())
		{
			if (auto ptr = dynamic_cast<NamedDecl*>(node.getParentDeclRecorder()))
				out_ << "(It has a parent DeclRecorder named " << ptr->getIdentifier()->getStr() << ")";
			else
				out_ << "(It has a parent DeclRecorder)";
		}
		out_ << "\n";
		curindent_++;
		for (auto it = node.recordedDecls_begin(); it != node.recordedDecls_end(); it++)
		{
			out_ << getIndent() << "> Declaration with name: " << it->first->getStr() << "\n";
		}
		curindent_-=2;
	}
	else
		out_ << getIndent() << "Invalid FunctionDecl\n";
}

void Dumper::visit(ReturnStmt & node)
{
	out_ << getIndent() << "Return statement\n";
	if (node.hasExpr())
	{
		curindent_ += 1;
		node.getExpr()->accept(*this);
		curindent_ -= 1;
	}
}

void Dumper::visit(CompoundStmt & node)
{
	out_ << getIndent() << "Compound Statement (Contains " << node.size() << " statements)\n";

	curindent_ += 1;

	for (auto it = node.stmts_beg(); it != node.stmts_end(); it++)
		it->accept(*this);

	curindent_ -= 1;
}

void Dumper::visit(ConditionStmt & node)
{
	out_ << getIndent() << "Condition\n";
	curindent_++;
	// if
	out_ << getIndent() << "Expression (Condition):\n";
	curindent_++;
	node.getCond()->accept(*this);
	curindent_--;
	out_ << getIndent() << "Body:\n";
	curindent_++;
	node.getThen()->accept(*this);
	curindent_--;
	// has else?
	if (node.getElse())
	{
		out_ << getIndent() << "Else:\n";
		curindent_++;
		node.getElse()->accept(*this);
		curindent_--;
	}
	curindent_--;
}

void Dumper::visit(WhileStmt & node)
{
	out_ << getIndent() << "While Loop\n";

	curindent_++;
	out_ << getIndent() << "Expression:\n";

	curindent_++;
	node.getCond()->accept(*this);
	curindent_--;

	out_ << getIndent() << "Body:\n";
	
	curindent_++;
	node.getBody()->accept(*this);
	curindent_--;

	curindent_--;
}

void Dumper::visit(DeclStmt & node)
{
	// just visit the decl if there is one
	if (node.hasDecl())
		node.getDecl()->accept(*this);
	else
		out_ << getIndent() << "Empty Declaration Statement";
}

std::string Dumper::getIndent() const
{
	std::string i = getOffsetTabs();
	for (unsigned char k(0); k < curindent_; k++)
		i += "    ";
	if (curindent_ > 1)
		i += u8"┗";
	return i;
}

std::string Dumper::getOffsetTabs() const
{
	std::string i;
	for (unsigned char k(0); k < offsetTabs_; k++)
		i += '\t';
	return i;
}