////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : Decl.hpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
// Declares the Decl interface as well as derived nodes.
////------------------------------------------------------////

#pragma once
#include "Type.hpp"
#include "DeclContext.hpp"
#include "Fox/Common/Source.hpp"

namespace fox
{
	// Kinds of Decls
	enum class DeclKind : std::uint8_t
	{
		#define DECL(ID,PARENT) ID,
		#define DECL_RANGE(ID,FIRST,LAST) First_##ID = FIRST, Last_##ID = LAST,
		#include "DeclNodes.def"
	};

	// Forward Declarations
	class Expr;
	class Identifier;
	class ASTContext;
	class CompoundStmt;

	// Decl
	//		Common base class for every Declaration
	class Decl
	{
		public:
			DeclKind getKind() const;

			void setRange(SourceRange range);
			SourceRange getRange() const;

			bool isValid() const;

			// Prohibit the use of builtin placement new & delete
			void *operator new(std::size_t) throw() = delete;
			void operator delete(void *) throw() = delete;
			void* operator new(std::size_t, void*) = delete;

			// Only allow allocation through the ASTContext
			void* operator new(std::size_t sz, ASTContext &ctxt, std::uint8_t align = alignof(Decl));

			// Companion operator delete to silence C4291 on MSVC
			void operator delete(void*, ASTContext&, std::uint8_t) {}

		protected:
			Decl(DeclKind kind, const SourceRange& range);

		private:
			SourceRange range_;
			const DeclKind kind_;
	};

	// NamedDecl
	//		Common base class for every named Declaration
	class NamedDecl : public Decl
	{
		public:
			NamedDecl(DeclKind kind, Identifier* id, SourceRange range);

			Identifier* getIdentifier();
			const Identifier* getIdentifier() const;
			void setIdentifier(Identifier* nname);
			bool hasIdentifier() const;

			bool isValid() const;

			static bool classof(const Decl* decl)
			{
				return (decl->getKind() >= DeclKind::First_NamedDecl) && (decl->getKind() <= DeclKind::Last_NamedDecl);
			}

		private:
			Identifier* identifier_;
	};

	// ValueDecl
	//		Common base class for every value declaration 
	//		(declares a value of a certain type & name)
	class ValueDecl : public NamedDecl
	{
		public:
			ValueDecl(DeclKind kind, Identifier* id, Type* ty, 
				bool isConst, SourceRange typeRange, SourceRange range);

			Type* getType();
			const Type* getType() const;
			void setType(Type* ty);

			SourceRange getTypeRange() const;
			void setTypeRange(SourceRange range);

			bool isConstant() const;
			void setIsConstant(bool k);

			bool isValid() const;

			static bool classof(const Decl* decl)
			{
				return (decl->getKind() >= DeclKind::First_ValueDecl) && (decl->getKind() <= DeclKind::Last_ValueDecl);
			}

		private:
			bool isConst_;
			SourceRange tyRange_;
			Type* type_;
	};

	// ParamDecl
	//		A declaration of a function parameter
	class ParamDecl : public ValueDecl
	{
		public:
			ParamDecl();
			ParamDecl(Identifier* id, Type* type, bool isConst, SourceRange tyRange, SourceRange range);

			bool isValid() const;

			static bool classof(const Decl* decl)
			{
				return decl->getKind() == DeclKind::ParamDecl;
			}
	};

	
	// FuncDecl
	//		A function declaration
	class FuncDecl : public NamedDecl, public DeclContext
	{
		private:
			using ParamVecTy = std::vector<ParamDecl*>;

			using ParamVecIter = ParamVecTy::iterator;
			using ParamVecConstIter = ParamVecTy::const_iterator;

		public:
			FuncDecl();
			FuncDecl(Type* returnType, Identifier* fnId, CompoundStmt* body,
				SourceRange range, SourceLoc headerEndLoc);
			
			void setLocs(const SourceRange& range, const SourceLoc& headerEndLoc);
			void setHeaderEndLoc(const SourceLoc& loc);

			SourceLoc getHeaderEndLoc() const;
			SourceRange getHeaderRange() const;

			// Note: Calls isValid on the args too.
			bool isValid() const;

			void setReturnType(Type* ty);
			Type* getReturnType() const;

			void setBody(CompoundStmt* body);
			CompoundStmt* getBody() const;

			void addParam(ParamDecl* arg);
			ParamDecl* getParamDecl(std::size_t ind) const;
			std::size_t getNumParams() const;

			ParamVecIter params_begin();
			ParamVecConstIter params_begin() const;

			ParamVecIter params_end();
			ParamVecConstIter params_end() const;

			static bool classof(const Decl* decl)
			{
				return decl->getKind() == DeclKind::FuncDecl;
			}

		private:
			SourceLoc headEndLoc_;
			Type* returnType_ = nullptr;
			ParamVecTy params_;
			CompoundStmt* body_ = nullptr;

			// Bitfields (7 bits left)
			bool paramsAreValid_ : 1;
	};

	// VarDecl
	//		A variable declaration
	class VarDecl : public ValueDecl
	{
		public:
			VarDecl();
			VarDecl(Identifier* id, Type* type, bool isConst,
				Expr* init, SourceRange range, SourceRange tyRange);

			bool isValid() const;

			Expr* getInitExpr() const;
			void setInitExpr(Expr* expr);
			bool hasInitExpr() const;

			static bool classof(const Decl* decl)
			{
				return decl->getKind() == DeclKind::VarDecl;
			}

		private:
			Expr* init_ = nullptr;
	};

	// UnitDecl
	//		A Unit declaration (a unit is a source file)
	//		This is declared "implicitely" when you create a new file
	class UnitDecl : public NamedDecl, public DeclContext
	{
		private:
			using DelVecTy = std::vector<Decl*>;
			using DeclVecIter = DelVecTy::iterator;
			using DeclVecConstIter = DelVecTy::const_iterator;

		public:
			UnitDecl(Identifier *id, FileID inFile);

			void addDecl(Decl* decl);
			Decl* getDecl(std::size_t idx) const;
			std::size_t getDeclCount() const;

			// Note: Checks the validity of the decls 
			// inside this too.
			bool isValid() const;

			DeclVecIter decls_beg();
			DeclVecIter decls_end();

			DeclVecConstIter decls_beg() const;
			DeclVecConstIter decls_end() const;

			FileID getFileID() const;
			void setFileID(const FileID& fid);

			static bool classof(const Decl* decl)
			{
				return decl->getKind() == DeclKind::UnitDecl;
			}

		private:
			DelVecTy decls_;
			FileID file_;

			// Bitfields (7 bits left)
			bool declsAreValid_ : 1;
	};
}

