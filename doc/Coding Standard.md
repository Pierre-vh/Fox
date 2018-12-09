## Style guidelines:

// Note: theses are pretty rough. I'll have to rewrite them one day.

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

* const-correctness
  * Returning pointrs
    * If the class doesn't own the pointer it returns
      * Only 1 getter : `T* get() const`
    * If the pointer is owned by the class
      * 2 getters `const T* get() const` and `T* get()`
  * For functions that cast `this` to something else, always create 2 getters.
    * e.g. `const T* getAs() const` and `T* getAs()`
    * e.g. `const Type withoutLoc() const` and `Type withoutLoc()`

## Error reporting
* Classed from the most to the least favored;
  * Use the DiagnosticEngine to report language-related errors to the user
    * `Expected a semicolon`  
  * Use assertions to catch bugs & detect abnormal situations
    * `assert(ptr && "My pointer is null !")`
    * `assert(mySourceLoc.getIndex() <= fileData->str.size() && "SourceLoc is out of range!")
  * Use exceptions for the rest, but try to avoid them if possible. 
    * Generally, use them through Error reporting methods

