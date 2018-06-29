##Style guidelines:
	* Function Names
		* Generally, try to name them with verb phrases, as they represent actions.
		* First letter should be lowercase, then use capital letters to separate works. (CamelCase) 
			* Example: `me.amIPretty()`
		* For functions that mimic the STL (e.g. they return iterators), use snake_case
			* Example: 'compoundStatement.decls_beg()'

	* Types Names (Usings, Typedefs, Structs & Classes, Enums)
		* CamelCase, with an uppercase first letter.
			* Example: "MyFloatType"

	* Variable Names :
		* Generally, try to name them as noun. Use a lowercase first letter, then capital letters to separate words (CamelCase)
		* Private member variables : Add a trailing underscore (`_`)
			* Example: `consumer_`
		* Others variables, including variables in POD-Like structs : Don't use the trailing underscore.
			* Example: `fooBar`

	* Struct vs Classes
		* Use structs for type that have a POD-like interface (with multiple public member variables and few methods)
		* Use classes for the rest

	* Assertions
		* When in doubt, use `assert()` to check if everything's alright. It no cost on performance in release mode, and can help a lot in tracking down bugs.

##C++ Features
	* Exceptions
		* Avoid exceptions in Moonshot code (the code contained in src/). Try to use assertions or the DiagnosticEngine to report errors. 
		  Only use them if you think that's the best thing to do in this situation.