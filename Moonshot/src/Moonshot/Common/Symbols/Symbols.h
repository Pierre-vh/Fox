#pragma once

#include "../Errors/Errors.h"

// SINGLETON
class Symbols
{
	public:
		~Symbols();

		static Symbols& getInstance();
		
	private:
		Symbols(); // Prevent instancing
		Symbols(Symbols const&); // Prevent copy @ construct
		Symbols& operator=(Symbols const&) {} // Prevent copying/assignement
};

