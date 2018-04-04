////------------------------------------------------------////
// This file is a part of The Moonshot Project.				
// See LICENSE.txt for license info.						
// File : Version.hpp											
// Author : Pierre van Houtryve								
////------------------------------------------------------//// 
// This file contains the version Major/Minor/Patch and other information
// About the current development state of the Moonshot Project
////------------------------------------------------------////

#define MOONSHOT_UTIL_TOSTR_HELPER(X) #X
#define MOONSHOT_UTIL_TOSTR(X) MOONSHOT_UTIL_TOSTR_HELPER(X)


/* Moonshot Version Numbers */
#define MOONSHOT_VER_MAJOR 0
#define MOONSHOT_VER_MINOR 1
#define MOONSHOT_VER_PATCH 0

// Possible version types:
	// inDev : inDevelopement, nothing is complete/working as intended
	// Alpha : It's taking shape, but still not quite usable
	// Beta  : Polishing, "internal changes" (changes not visible by the user, like solving perf problems, polishing the API, bugfixing)
	// Release : Usable release
#define MOONSHOT_VER_TYPE "inDev"	

#define MOONSHOT_VERSION MOONSHOT_UTIL_TOSTR(MOONSHOT_VER_MAJOR) "." MOONSHOT_UTIL_TOSTR(MOONSHOT_VER_MINOR) "." MOONSHOT_UTIL_TOSTR(MOONSHOT_VER_PATCH)
#define MOONSHOT_VERSION_COMPLETE MOONSHOT_VERSION " (" MOONSHOT_VER_TYPE ")"


// Next planned version number changes
	// Next Major incrementation : Badger and Fox are fully operational, ready for public use. (Estimation : Q4 2018/Q1 2019 with some luck)
	// Next Minor incrementation 
			// "inDev"
		// 2 - Parser is complete with AST Rework done
		// 3 - Semantic analysis done
		// 4 - IRGen done
		// 5 - Badger works
			// "Alpha"
		// 6 - Badger's performance is good enough to be acceptable and the focus can shift again on fox to implement other much needed features
		// 7 - Fox implements multiple files handling
		// 8 - Fox implements other language features considered vital, like arrays/tuples/dictionaries if they're not in yet.
			// "Beta"
		// 9 - Add a Foreign Function Interface to Badger
		// 10 - Add a proper driver and API.
		// etc. It's far away, we'll see what the future holds, but when we reach this point it shouldn't be too late until we go 1.0.0
	// Next Patch Incrementation : Not planned until  1.0.0 since no patch's needed. It's all indev.