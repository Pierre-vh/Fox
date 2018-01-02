#include "Utils.h"

using namespace Moonshot;

std::string util::filepath_MoonshotProj(const std::string & s)
{
	// Build path is \Moonshot\<config>\
	// Moonshot source is:
	// \Moonshot\Moonshot
	return ".\\..\\Moonshot\\" + s;
}
