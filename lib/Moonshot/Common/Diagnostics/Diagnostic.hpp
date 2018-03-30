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

#include <string>
#include <sstream>

namespace Moonshot
{
	enum class DiagID
	{
		dummyDiag = -1,
		#define DIAG(SEVERITY,ID,TEXT) ID,
		#define KEEP_DIAG_DEF
			#include "Diags/DiagsAll.def"
		#undef DIAG
		#undef KEEP_DIAG_DEF
	};

	enum class DiagSeverity
	{
		IGNORE,		// Ignore the diagnostic
		NOTE,		// Present this diag as a note, a log entry.
		WARNING,	// Present this diag as a warning.
		ERROR,		// "					" error
		FATAL		// "					" Fatal error, which means an error grave enough that it stopped the compilation
	};

	class IDiagConsumer;

	class Diagnostic
	{
		public:
			Diagnostic(IDiagConsumer *cons, const DiagID& dID, const DiagSeverity& dSev,const std::string& dStr);
			Diagnostic(Diagnostic &other);

			// Creates a silenced diagnostic object, which is a diagnostic of id SilencedDiag with no consumer, no str and a IGNORE severity.
			// In short, it's a dummy!
			static Diagnostic createEmptyDiagnostic(); 

			~Diagnostic();
			
			// Emit this diagnostic
			void emit();

			DiagID getDiagID() const;
			std::string getDiagStr() const;
			DiagSeverity getDiagSeverity() const;

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
		private:
			// Empty ctor to create dummy diagnostics objects.
			Diagnostic();  
			// replaces every occurence of "%(value of index)" in a string with the replacement.
			// e.g: replacePlaceholder("foo",0) -> replaces every %0 in the string by foo
			Diagnostic& replacePlaceholder(const std::string& replacement, const unsigned char& index);

			void kill(); // Kills this diagnostic (sets isActive to false and remove (frees) most of it's information)
			
			Diagnostic& operator=(const Diagnostic&) = delete;

			bool isActive_ = true; // sets to false when emit is called. if this is false, the diagnostic shouldn't be used.

			IDiagConsumer *consumer_ = nullptr;
			DiagID diagID_;
			std::string diagStr_;
			DiagSeverity diagSeverity_;

			// The next %x to replace.
			unsigned char curPHIndex_ = 0;
	};
}