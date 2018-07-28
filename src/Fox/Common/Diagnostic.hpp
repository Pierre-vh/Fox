////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : DiagnosticEngine.hpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
// This file declares the Diagnostic class (which stores 
// informations on a specific diagnostic).
// This file also contains the DiagID enum, which store every possible
// diagnostics
////------------------------------------------------------////

#pragma once

#include "Source.hpp"
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
		IGNORE,
		NOTE,
		WARNING,
		ERROR,	
		FATAL		
	};

	class Diagnostic
	{
		public:
			Diagnostic(DiagnosticEngine *engine, DiagID dID, DiagSeverity dSev,const std::string& dStr, const SourceRange& range = SourceRange());
			// Note : both copy/move ctors kill the copied diag.
			Diagnostic(Diagnostic &other);
			Diagnostic(Diagnostic &&other);

			// Destructor that emits the diag.
			~Diagnostic();
			
			void emit();

			// Getters for basic args values
			DiagID getDiagID() const;
			std::string getDiagStr() const;
			DiagSeverity getDiagSeverity() const;
			SourceRange getSourceRange() const;

			bool hasValidSourceRange() const;

			// File-wide diagnostics are diagnostics that concern
			// a whole file. 
			Diagnostic& setIsFileWide(bool fileWide);
			bool isFileWide() const;

			// Replace a %x placeholder.
			template<typename ReplTy>
			inline Diagnostic& addParamDecl(const ReplTy& value)
			{
				auto tmp = curPHIndex_;
				curPHIndex_++;
				return addParamDecl(value,tmp);
			}

			// Frozen diags are locked, they cannot be modified further.
			bool isFrozen() const;
			Diagnostic& freeze();

			// Inactive diags won't be emitted.
			bool isActive() const;
			explicit operator bool() const;
		private:
			friend class DiagnosticEngine;
			Diagnostic& operator=(const Diagnostic&) = default;

			// Internal addParamDecl overloads
			template<typename ReplTy>
			inline Diagnostic& addParamDecl(const ReplTy& value, std::uint8_t phIndex)
			{
				std::stringstream ss;
				ss << value;
				return replacePlaceholder(ss.str(), phIndex);
			}

			// For std::strings
			template<>
			inline Diagnostic& addParamDecl(const std::string& value, std::uint8_t phIndex)
			{
				return replacePlaceholder(value, phIndex);
			}

			// for CharType
			template<>
			inline Diagnostic& addParamDecl(const CharType& value, std::uint8_t phIndex)
			{
				return replacePlaceholder(
					StringManipulator::charToStr(value), phIndex
				);
			}

			// replaces every occurence of "%(value of index)" in a string with the replacement.
			// e.g: replacePlaceholder("foo",0) -> replaces every %0 in the string by foo
			// Replace the "%(phIndex)" arg by value (as a string)
			Diagnostic& replacePlaceholder(const std::string& replacement, std::uint8_t index);

			void kill(); 
			
			void initBitFields();  

			// Packed in 8 bits (0 left)
			bool active_ :1; 
			bool frozen_ :1; 
			std::uint8_t curPHIndex_ :6;

			// Packed in 8 bits (3 left)
			DiagSeverity diagSeverity_ : 4; 
			bool fileWide_ : 1;

			DiagnosticEngine* engine_ = nullptr;
			DiagID diagID_;
			std::string diagStr_;
			SourceRange range_;
	};
}