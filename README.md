# The Fox Programming Language

Fox is a language that aims to be small and easily embeddable. 

This is a project that I started to learn more about compilers and interpreter.

NOTE: Working on Fox isn't a priority for me anymore. New commits will be relatively rare (or nonexistent).

## Quick language tour

NOTE: Not all features described here are fully working. For instance, array casting (`[int]` from/to `[double]`) doesn't work yet.

Here's a few examples of Fox code:
```swift

// 5 Basic types + Arrays
// let = immutable variable, var = mutable variable. Only the variable's value is (im)mutable, but
// if it's a reference type, the value it references might still change.

// (value type) 64 Bit signed integers
let i : int = 42;
// (value type) Double precision floating point numbers
var d : double = 3.14;
// (value type) booleans
let b : bool = false;
// (reference type) immutable utf-8 encoded string
var s : string = "Hello, Fox!";
// (value type) 32 bit unicode code point
let c : char = '禅'; 
// Arrays of dynamic size
var a : [[int]] = []; // Array literals are inferred based on their use

// Function parameters are always passed by value, and are immutable by default. Using 'mut' makes them mutable.
// ': bool' marks the return type of the function. Omitting it will just make the function return void.
func foo(x: int, y: mut int) : [int] {
  // This is a test function that doesn't do anything interesting.
  if y == 0 {
    return []; // automatically inferred to [int]
  }
  return [x, y];
}

func bar(count: mut int) {
  while count > 0 {
    printString("Count: " + $count + '\n'); // the unary $ operator converts an int/double/bool/char to string.
    count = count-1;
  }
}

```

## Building Fox
Required:
 * CMake
 * Git
 * A C++14 Compiler (preferably MSVC, I can't guarantee that it builds on other compilers for now) 

1. Open the command prompt and set your working directory to the folder where you wish to build the Fox interpreter.
1. Clone the repo
   * `git clone https://github.com/Pierre-vh/Fox.git`
1. Create a new directory and set it as your working directory (we'll do an out of source build)
   * `mkdir build`
   * `cd build`
1. Generate the CMake cache `cmake ../Fox`
1. Building the project: `cmake --build .`
    * The executables will be available in the `bin/(Build Type)/` folder.

## Usage

Assuming that the binaries are installed in your path:
* `fox` will launch the "command line interface", which is a quick and easy way to test the interpreter. The interface is really simple: you can only enter paths to files for the interpreter to process. (This is only a testing tool. For more advanced use, invoke the compiler using command line options (see below))
* `fox path/to/file.fox` will process the given file. Errors and diagnostics will be printed to stdout, and the command line tool will return EXIT_SUCCESS if the file was processed without any diagnostic being emitted (it'll return EXIT_FAILURE otherwise). Several  options can be passed to the command line tool.
  * `-verify` enables verify mode
  * `-werr` promotes warnings to errors
  * `-dump-ast` will dump the abstract syntax tree (AST) after processing the file.
  * `-parse-only` will stop the interpretation process right after parsing
  * `-dump-bcgen` will dump the bytecode
  * `-run` will run the program
  * `-v` or `-verbose` will enable verbose output (note: it's relatively limited)


## Doc
I try to document the code as much as possible using Doxygen-style doc, but not all the code uses it for now.

Also, I don't generate Doxygen doc regularly, so I can't guarantee that it works as intended.
However, the documentation in the code should be enough to understand what's going on most of the time.

## License
Licensed under the terms of the MIT license. 

## Contact
If you have any questions don't hesitate to send me an email at `pierre.vanhoutryve@gmail.com`

## Project structure
* /includes/ Contains the headers
* /lib/ Contains the source code/implementation
* /tools/ Contains the main.cpp, compiles to fox.exe
* /tests/ "Runnable" language test programs which use the -verify mode and (in the future) File-Checking utilities.
* /unittests/ Unit tests powered by GoogleTest which tests utilities (/Common) of the project and some parts of the pipeline. (Including the lexer)
* /docs/ Documentation
* /thirdparty/ Source code of external libraries used by the project.

## Acknowledgments
* CLang and Swift
  * These well written open source projects have been of immense help when I was in doubt, or in need of help.
    The design of the Fox interpreter has been highly influenced by the design of these compilers. 
* /r/programminglanguages (Reddit & Discord server)
  * Friendly, passionate people that have helped me countless times. Can't thank them enough!
