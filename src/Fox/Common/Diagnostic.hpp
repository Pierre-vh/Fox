////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : DiagnosticEngine.hpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
// This file declares the Diagnostic struct (which stores 
// informations on a specific diagnostic).
// This file also contains the DiagID enum, which store every possible
// diagnostics
////------------------------------------------------------////

#pragma once

#include "SourceManager.hpp"
#include <string>
#include <sstream>

namespace fox
{
	class SourceRange;
	class DiagnosticConsumer;

	enum class DiagID : int16_t
	{
		dummyDiag = -1,
		// Important : first value must always be 0 to keep sync with the severities and strs arrays.
		#define DIAG(SEVERITY,ID,TEXT) ID,
			#include "Diags/DiagsAll.def"
	};

	enum class DiagSeverity : int8_t
	{
		IGNORE,		// Ignore the diagnostic
		NOTE,		// Present this diag as a note, a log entry.
		WARNING,	// Present this diag as a warning.
		ERROR,		// "					" error
		FATAL		// "					" Fatal error, which means an error grave enough that it stopped the compilation
	};

	class Diagnostic
	{
		public:
			Diagnostic(DiagnosticConsumer *cons, const DiagID& dID, const DiagSeverity& dSev,const std::string& dStr, const SourceRange& range = SourceRange());
			
			// Copy constructor that kills the copied diagnostic and steals it's information
			Diagnostic(Diagnostic &other);

			// Move constructor that kills the moved diagnostic and steal it's information
			Diagnostic(Diagnostic &&other);

			// Creates a silenced diagnostic object, which is a diagnostic of id SilencedDiag with no consumer, no str and a IGNORE severity.
			// In short, it's a dummy!
			static Diagnostic createDummyDiagnosticObject(); 

			~Diagnostic();
			
			// Emit this diagnostic
			void emit();

			DiagID getDiagID() const;
			std::string getDiagStr() const;
			DiagSeverity getDiagSeverity() const;
			SourceRange getSourceRange() const;
			bool hasValidSourceRange() const;

			/*
				Small note: addArg functions return a reference to (this). Why, you ask? Because this allows chaining, like so:
				diagengine.report(DiagID::foo_is_not_bar).addArg("foo").addArg("bar").emit();
			*/

			// Replace the latest unremoved arg by std::to_string(value)
			template<typename ReplTy>
			inline Diagnostic& addArg(const ReplTy& value)
			{
				auto tmp = curPHIndex_;
				curPHIndex_++;
				return addArg(value,tmp);
			}

			// Replace the "%(phIndex)" arg by value (as a string)
			template<typename ReplTy>
			inline Diagnostic& addArg(const ReplTy& value, const unsigned char& phIndex)
			{
				if constexpr (std::is_same<std::string,ReplTy>::value)
					return replacePlaceholder(value, phIndex);
				else
				{
					std::stringstream ss;
					ss << value;
					return replacePlaceholder(ss.str(), phIndex);
				}
			}

			bool isActive() const;

			// A Frozen diagnostic can't be edited any further. it's locked.
			bool isFrozen() const;
			Diagnostic& freeze();

			// Does this diag possess a valid consumer?
			bool hasValidConsumer() const;

			// Checks if the arg is valid for emission
			operator bool() const;
		private:
			// friends
			friend class DiagnosticEngine;

			// Empty ctor to create dummy diagnostics objects.
			// It's private so it's not abused by other classes or by automatic C++ constructions. But it's accessible through createDummyDiagnosticObject()
			// and to friend classes
			Diagnostic();  
			// Deleted assignement operator
			Diagnostic& operator=(const Diagnostic&) = default;

			// replaces every occurence of "%(value of index)" in a string with the replacement.
			// e.g: replacePlaceholder("foo",0) -> replaces every %0 in the string by foo
			Diagnostic& replacePlaceholder(const std::string& replacement, const unsigned char& index);

			void kill(); // Kills this diagnostic (sets isActive to false and remove most of it's information)
			
			bool isActive_ = true; 
			bool isFrozen_ = false; 
			unsigned char curPHIndex_ = 0; 
			DiagSeverity diagSeverity_; 

			DiagnosticConsumer *consumer_ = nullptr;
			DiagID diagID_;
			std::string diagStr_;
			SourceRange range_;
	};
}