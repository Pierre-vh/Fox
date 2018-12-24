# Moonshot
Moonshot is a project that aims to create an interpreter for a language called Fox.
Fox is a statically typed scripting language that tries to be easily embeddable and easy to learn.

This is currently a work in progress, only a fraction of the project is completed.

## Building the project

Required:
 * CMake
 * Git

1. Open the command prompt and set your working directory to the folder where you wish to build Moonshot.
1. Clone the repo
   * `git clone https://github.com/Pierre-vh/Moonshot.git`
1. Create a new directory and set it as your working directory (we'll do an out of source build)
   * `mkdir build`
   * `cd build`
1. Generate the CMake cache `cmake ../Moonshot`
1. Building the project: `cmake --build .`
    * The executables will be available in the `bin/(Build Type)/` folder.

## Usage
*Note: the project is in early development and thus is changing very rapidly. I won't document everything because things are certainly not definitive, but I'll try to keep this as up to date as I can*

**Note:** Currently, only the Lexer, Parser and a small part of the Semantic analysis are complete, so files aren't executed and are just "compiled" with what's available.

Assuming that the Moonshot binaries are installed in your path:
* `Moonshot` will launch the "command line interface", which is a quick and easy way to test the interpreter. The interface is really simple: you can only enter paths to files for the interpreter to process. (This is only a testing tool. For more advanced use, invoke the compiler using command line options (see below))
* `Moonshot path/to/file.fox` will process the given file. Errors and diagnostics will be printed to stdout, and the command line tool will return EXIT_SUCCESS if the file was processed without any diagnostic being emitted (it'll return EXIT_FAILURE otherwise). Several  options can be passed to the command line tool.
  * `-verify` enables verify mode (See **Testing**))
  * `-werr` promotes warnings to errors
  * `-dump-ast` will dump the abstract syntax tree (AST) after processing the file.


## Testing
  TODO

## License
Licensed under the terms of the MIT license. 

## Contact
If you have any questions don't hesitate to send me a mail at `pierre.vanhoutryve@gmail.com`

## Project structure
* /includes/ Contains the headers
* /lib/ Contains the source code/implementation
* /tools/ Contains the main.cpp, compiles to moonshot.exe
* /tests/ "Runnable" language test programs which use the -verify mode and (in the future) File-Checking utilities.
* /unittests/ Unit tests powered by GoogleTest which tests utilities (/Common) of the project and some parts of the pipeline. (Including the lexer)
* /docs/ Documentation
* /thirdparty/ Source code of external libraries used by the project.

## Acknowledgments
* CLang and Swift
  * These well written open source projects have been of immense help when I was in doubt, or in need of help. I've spent countless
    hours digging through the source code of Swift and Clang, and that helped me improve my programming and compiler designing skills.
    The design of Moonshot/Fox has been highly influenced by the design of theses compilers. 
