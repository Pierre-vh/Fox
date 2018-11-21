//----------------------------------------------------------------------------//
// This file is a part of The Moonshot Project.        
// See the LICENSE.txt file at the root of the project for license information.            
// File : Version.hpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//
// This file contains the version Major/Minor/Patch and other information
// About the current version of the Moonshot Project
//----------------------------------------------------------------------------//

#define MOONSHOT_UTIL_TOSTR_HELPER(X) #X
#define MOONSHOT_UTIL_TOSTR(X) MOONSHOT_UTIL_TOSTR_HELPER(X)


/* Moonshot Version Numbers */
#define MOONSHOT_VER_MAJOR 0
#define MOONSHOT_VER_MINOR 3
#define MOONSHOT_VER_PATCH 0

#define MOONSHOT_DEV_PHASE "inDev"  

// Description of each "development" phase
// inDev: Nothing is complete/working as intended  (Make it work)
// Alpha: The interpreter is working and can run basic Fox code. 
//       This phase is mostly improving the existing code :
//       Writing more tests, Refactoring, DRYing, etc. 
//       At the end of this phase, the code should be
//       clean and pleasant to read. (Make it right)
// Beta: Optimization-oriented development phase. 
//       This include refactoring/rewriting slow code, 
//       profiling, etc (Make it fast)  
//      
// Release: The interpreter is considered good enough to be usable by the public


#define MOONSHOT_VERSION MOONSHOT_UTIL_TOSTR(MOONSHOT_VER_MAJOR) \
 "." MOONSHOT_UTIL_TOSTR(MOONSHOT_VER_MINOR) "."\
 MOONSHOT_UTIL_TOSTR(MOONSHOT_VER_PATCH)
#define MOONSHOT_VERSION_COMPLETE MOONSHOT_VERSION " (" MOONSHOT_DEV_PHASE ")"
