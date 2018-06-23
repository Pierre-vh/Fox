### Premise

I write this document because I've found myself to have an inconsistent code style, and my code style is mostly bad for readability (e.g. using an
underscore at the end of a member variable). This document is mostly intended for myself as a reminder of how my code should look like.

If the project get more open to contribution in the future (and gains popularity), I'll complete it more thouroughly.


## Naming Convention

This part is pretty similar to [the LLVM Coding Standard](https://llvm.org/docs/CodingStandards.html#the-high-level-issues).

**Important:** All names should be in CamelCase, unless stated otherwise.

* **Every type name** must be a noun beginning with an uppercase letter. This concerns
  * Classes
  * Structs
  * Enums and Enum classes
  * typedefs/usings

* **Every variable name** must be a noun beginning with a lowercase letter. 
  * Also, do not use any kind of special naming (such as `m_`or a `_` at the end)  between member variables and other variables
  
* **Every function name** must be verb phrases, as they represent actions. They must begin with a lowercase letter.
  * An exception shall be made for methods that mimic STL-like behaviour (like functions returning iterators.). An example is `CompoundStatement::stmts_beg()`
  
**Eventually,** you can break rules 2 and 3 on **rare** occasions if you feel like they'll make the code less readable. 
Just try to stick to rule 1 as much as possible.

## General Stuff 

* [Don't use else after a return.](https://llvm.org/docs/CodingStandards.html#don-t-use-else-after-a-return)
* [Use early exits and continues to simplify code.](https://llvm.org/docs/CodingStandards.html#use-early-exits-and-continue-to-simplify-code)
* [Include as little as possible. Only include when needed](https://llvm.org/docs/CodingStandards.html#include-as-little-as-possible)
* [Use assert liberally.](https://llvm.org/docs/CodingStandards.html#assert-liberally) They can be life-savers when debugging large amount of code, and they're disabled at run-time. There is absolutely no reason for you to ignore them.
* Do not use `using namespace std;`. Avoid `using namespace` as much as possible, unless you're dealing with code within the `Moonshot` namespace. Then, you're free to use it.
* Prefer `'\n'` over `std::endl`, because `std::endl` inserts a newline and flushes the stream.
