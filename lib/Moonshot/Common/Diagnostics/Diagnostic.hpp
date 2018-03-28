////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : DiagnosticEngine.hpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
// This file declares the Diagnostic struct (which stores 
// informations on a specific diagnostic).
// This file also contains the DiagsID enum, which store every possible
// diagnostics
////------------------------------------------------------////

#pragma once

#include <string>

namespace Moonshot
{
	enum class DiagsID
	{
		#define DIAG(SEVERITY,ID,TEXT) ID,
		#define KEEP_DIAG_DEF
			#include "DiagsAll.def"
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
			Diagnostic(IDiagConsumer *cons, const DiagsID& dID, const DiagSeverity& dSev,const std::string& dStr);
			Diagnostic(Diagnostic &other);
			~Diagnostic();
			void emit();

			/*
				TODO : Add functions to massage the diagnostic.
			*/
		
			DiagsID getDiagID() const;
			std::string getDiagStr() const;
			DiagSeverity getDiagSeverity() const;
			bool isActive() const;
		private:
			void kill(); // Kills this diagnostic (sets isActive to false and remove most of it's information)
			
			Diagnostic& operator=(const Diagnostic&) = delete;

			bool isActive_ = true; // sets to false when emit is called. if this is false, the diagnostic shouldn't be used.
			IDiagConsumer *consumer_ = 0;
			DiagsID diagID_;
			std::string diagStr_;
			DiagSeverity diagSeverity_;
	};
}