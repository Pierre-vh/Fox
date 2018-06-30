////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : DeclRecorder.hpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
// DeclRecorder acts as a Declaration Recorder. While parsing, the 
// parser "registers" every declaration in the most recent declaration recorder found.
// DeclRecorder assists during name resolution to find members of a unit (or, in the future, Class/Namespaces/etc), or just to find if a
// identifier is linked to one or more declaration in a specific scope.
//
// This class has no notion of scope, it doesn't care about compound statements and stuff, this will be managed by the semantic analysis phase
// to confirm "Visibility" of a variable.
//
//	Potential area of improvements:
//
////------------------------------------------------------////

#pragma once

#include <map>
#include <vector>
#include <type_traits>

namespace fox
{
	class Decl;
	class NamedDecl;
	class IdentifierInfo;
	class LookupResult;

	// An iterator that abstracts the underlying structure used by DeclRecorder to only show
	// the DeclRecorder*
	// Operator * returns a DeclRecorder*
	// Operator -> Lets you directly access the NamedDecl's members.
	template <typename BaseIterator>
	class DeclRecorderIterator : public BaseIterator
	{
		public:
			using value_type = typename BaseIterator::value_type::second_type;

			DeclRecorderIterator(const BaseIterator &baseIt) : BaseIterator(baseIt)
			{
				static_assert(std::is_same<value_type, NamedDecl*>::value, "Pointer type isn't a NamedDecl*");
			}

			// Operator * returns the pointer
			value_type operator*() const { return (this->BaseIterator::operator*()).second; }
			// Operator -> lets you access the members directly. It's equivalent to (*it)->
			value_type operator->() const { return (this->BaseIterator::operator*()).second; }
	};

	class DeclRecorder
	{
		private:
			using NamedDeclsMapTy = std::multimap<IdentifierInfo*, NamedDecl*>;
			using NamedDeclsMapIter = DeclRecorderIterator<NamedDeclsMapTy::iterator>;
			using NamedDeclsMapConstIter = DeclRecorderIterator<NamedDeclsMapTy::const_iterator>;
		public:
			DeclRecorder(DeclRecorder * parent = nullptr);
			inline virtual ~DeclRecorder() {}

			// "Record" a declaration within this DeclRecorder
			void recordDecl(NamedDecl* decl);

			// Searches for every NamedDecl whose identifier == id in this DeclRecorder
			LookupResult restrictedLookup(IdentifierInfo *id) const;

			// Performs a restrictedLookup on this DeclRecorder and recursively searches parent
			// DeclRecorders.
			LookupResult fullLookup(IdentifierInfo *id) const;

			// Manage parent decl recorder
			bool hasParentDeclRecorder() const;
			DeclRecorder* getParentDeclRecorder();
			void setParentDeclRecorder(DeclRecorder *dr);
			void resetParentDeclRecorder();

			// Get information
			std::size_t getNumberOfRecordedDecls()  const;

			NamedDeclsMapIter recordedDecls_begin();
			NamedDeclsMapIter recordedDecls_end();

			NamedDeclsMapConstIter recordedDecls_begin() const;
			NamedDeclsMapConstIter recordedDecls_end() const;
		private:
			// Pointer to the Declaration Recorder "above" this one.
			DeclRecorder * parent_ = nullptr;
			NamedDeclsMapTy namedDecls_;
	};

	// A Class that encapsulates a lookup result.
	// All NamedDecls stored here are assumed to have the same IdentifierInfo.
	// an assertion in addResult ensures this.
	class LookupResult
	{
		private:
			using ResultVecTy = std::vector<Decl*>;
			using ResultVecIter = ResultVecTy::iterator;
			using ResultVecConstIter = ResultVecTy::const_iterator;
		public:
			LookupResult();

			// Returns false if this LookupResult is empty.
			bool isEmpty() const;

			// Returns true if this LookupResult contains only one result.
			bool isUnique() const;

			std::size_t getSize() const;

			// If this LookupResult contains only one result, returns it, else, returns a nullptr.
			NamedDecl* getResultIfUnique() const;

			explicit operator bool() const;
		protected:
			friend class DeclRecorder;

			// Add another lookup result.
			void addResult(NamedDecl* decl);
			// Clear this LookupResult
			void clear();
			// If the target contains at least 1 result,
			// copies all of the results from target into this lookupresult then clears the target.
			void absorb(LookupResult &target);
		private:
			std::vector<NamedDecl*> results_;
	};
}