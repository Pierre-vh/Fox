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

#include "Source.hpp"
#include "Typedefs.hpp"
#include "StringManipulator.hpp"
#include <string>
#include <sstream>

namespace fox
{
	class DiagnosticEngine;
	enum class DiagID : std::uint16_t
	{
		// Important : first value must always be 0 to keep sync with the severities and strs arrays.
		#define DIAG(SEVERITY,ID,TEXT) ID,
			#include "Diags/DiagsAll.def"
	};

	enum class DiagSeverity : std::uint8_t
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
			Diagnostic(DiagnosticEngine *engine, const DiagID& dID, const DiagSeverity& dSev,const std::string& dStr, const SourceRange& range = SourceRange());
			
			// Copy constructor that kills the copied diagnostic and steals it's information
			Diagnostic(Diagnostic &other);

			// Move constructor that kills the moved diagnostic and steal it's information
			Diagnostic(Diagnostic &&other);

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
				std::stringstream ss;
				ss << value;
				return replacePlaceholder(ss.str(), phIndex);
			}

			// For std::strings
			template<>
			inline Diagnostic& addArg(const std::string& value, const unsigned char& phIndex)
			{
				return replacePlaceholder(value, phIndex);
			}

			// for CharType
			template<>
			inline Diagnostic& addArg(const CharType& value, const unsigned char& phIndex)
			{
				return replacePlaceholder(
					StringManipulator::charToStr(value), phIndex
				);
			}

			bool isActive() const;

			// A Frozen diagnostic can't be edited any further. it's locked.
			bool isFrozen() const;
			Diagnostic& freeze();

			// Checks if the Diag is valid for emission
			explicit operator bool() const;
		private:
			friend class DiagnosticEngine;
			Diagnostic& operator=(const Diagnostic&) = default;

			// replaces every occurence of "%(value of index)" in a string with the replacement.
			// e.g: replacePlaceholder("foo",0) -> replaces every %0 in the string by foo
			Diagnostic& replacePlaceholder(const std::string& replacement, const unsigned char& index);

			void kill(); // Kills this diagnostic (sets isActive to false and remove most of it's information)
			
			bool isActive_ = true; 
			bool isFrozen_ = false; 
			unsigned char curPHIndex_ = 0; 
			DiagSeverity diagSeverity_; 

			DiagnosticEngine* engine_ = nullptr;
			DiagID diagID_;
			std::string diagStr_;
			SourceRange range_;
	};
}