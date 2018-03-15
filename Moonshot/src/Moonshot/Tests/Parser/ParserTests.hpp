#pragma once

#include <vector>
#include <map>
#include <string>
#include <functional>
#include "../Utils/Utils.hpp"
#include "Moonshot/Fox/Parser/Parser.hpp"

namespace Moonshot
{
	class Parser;
}
namespace Moonshot::Tests
{
	// todo : write test class
	class ParserTests
	{
		public:
			ParserTests();

			bool runTests(std::ostream& out,bool condensed = false);

		private:

			enum class ParserTestGroup
			{
				// generate the enum for every group
				#define PARSER_TEST_GROUP(STR,ID) ID,
				#include "Moonshot/Tests/def/parser.def"
			};

			enum class ReadMode
			{
				LINES,WHOLE_FILE
			};

			struct IndividualTest
			{
				IndividualTest(const bool& should_pass, const ReadMode& rm, const std::string& fp, std::function<void(Parser&)> fn);

				bool shouldPass;
				std::string filepath;
				ReadMode readmode;
				std::function<void(Parser&)> testfn;
			};

			// add a function to test a vector of IndividualTest
			// add a function to test a single individual test

			std::map<ParserTestGroup, std::string> testGroupNames = {
				#define PARSER_TEST_GROUP(GROUPNAME,ID) {ParserTestGroup::ID,GROUPNAME},
				#include "Moonshot/Tests/def/parser.def"
			};

			std::map<ParserTestGroup, std::vector<IndividualTest>> tests;
	};
}


