## Style guidelines:

 * Function Names
	* Generally, try to name them with verb phrases, as they represent actions.
	* First letter should be lowercase, then use capital letters to separate works. (CamelCase) 
		* Example: `me.amIPretty()`
	* For functions that mimic the STL, use snake_case
		* Example: 'compoundStatement.decls_beg()'

* Types Names (Usings, Typedefs, Structs & Classes, Enums)
	* CamelCase, with an uppercase first letter.
		* Example: "MyFloatType"

* Variable Names :
	* Generally, try to name them as noun. Use a lowercase first letter, then capital letters to separate words (CamelCase)
	* Private member variables : Add a trailing underscore (`_`)
		* Example: `consumer_`
	* Others variables, including public member variables : CamelCase with a lowercase first letter.
		* Example: `fooBar`

* Struct vs Classes
	* Use structs for type that have a POD-like interface (with multiple public member variables and few methods)
	* Use classes for the rest

* Assertions
	* When in doubt, use `assert()` to check if everything's alright. It no cost on performance in release mode, and can help a lot in tracking down bugs.

## Error reporting
* Classed from the most to the least favored;
	* Use the DiagnosticEngine to report language-related errors to the user
		* `Expected a semicolon`	
	* Use assertions to catch bugs & detect abnormal situations
		* `assert(ptr && "My pointer is null !")`
		* `assert(mySourceLoc.getIndex() <= fileData->str.size() && "SourceLoc is out of range!")
	* Use exceptions for the rest, and try to use them sparingly.
		* Note that generally speaking, you'll have very little need for exceptions. They shouldn't be used too often
		  because they might be removed from the project one day.

