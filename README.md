# The Fox Programming Language
NOTE: This is a project that I started to learn more about compilers and interpreter but even if the main goal is to learn, I'm still developing it seriously, hoping that (if I do finish it entirely) it might be useful to someone someday.

Fox is a language that aims to be easily embeddable in other projects and easy to dive into. Fox is still under development and is not functional as of now.

## Building the project
Required:
 * CMake
 * Git

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
  * `-verify` enables verify mode (See **Testing**))
  * `-werr` promotes warnings to errors
  * `-dump-ast` will dump the abstract syntax tree (AST) after processing the file.


## Testing
  TODO

## Doc
I try to re-document the code as much as possible using Doxygen-style doc, but not all the code uses it for now.

Also, I don't generate Doxygen doc regularly, so I can't guarantee that it works as intended.
However, the documentation in the code should be enough to understand what's going on most of the time.

## License
Licensed under the terms of the MIT license. 

## Contact
If you have any questions don't hesitate to send me a mail at `pierre.vanhoutryve@gmail.com`

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
    The design of the Fox interpreter has been highly influenced by the design of theses compilers. 
* /r/programminglanguages (Reddit & Discord server)
  * Friendly, passionate people that have helped me countless times. Can't thank them enough!
