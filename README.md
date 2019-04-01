# The Fox Programming Language
Fox is a language that aims to be easily embeddable in other projects and easy to dive into. In Fox, you won't
see any complicated or confusing semantics. Everything should be clear at a glance!

Note: This is currently a work in progress, only a fraction of the project is completed.

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
*Note: the project is in early development and thus is changing very rapidly. I won't document everything because things are certainly not definitive, but I'll try to keep this as up to date as I can*

**Note:** Currently the interpreter is *not* functional. It's a work in progress, and I'm just starting to work on the VM. The language's features are also very limited for now (only a single file can be compiled, no objects, no tuples, no overloads, no UFCS, no FFI). For now, I'm working on getting the main pipeline working (*Make it work* phase: making the interpreter able to compile code and run the resulting bytecode). Once that's done, I'll add the rest of the features (UFCS/Overload/FFI/Multi-file support) and work on getting the interpreter 'usable' (*Make it right* phase).

Assuming that the binaries are installed in your path:
* `fox` will launch the "command line interface", which is a quick and easy way to test the interpreter. The interface is really simple: you can only enter paths to files for the interpreter to process. (This is only a testing tool. For more advanced use, invoke the compiler using command line options (see below))
* `fox path/to/file.fox` will process the given file. Errors and diagnostics will be printed to stdout, and the command line tool will return EXIT_SUCCESS if the file was processed without any diagnostic being emitted (it'll return EXIT_FAILURE otherwise). Several  options can be passed to the command line tool.
  * `-verify` enables verify mode (See **Testing**))
  * `-werr` promotes warnings to errors
  * `-dump-ast` will dump the abstract syntax tree (AST) after processing the file.


## Testing
  TODO

## Doc
I try to document the code as much as possible using Doxygen-style doc, however I don't generate Doxygen doc regularly, so I can't
guarantee that it works as intended for now. However, the documentation in the code should be enough to understand what's going on
most of the time.

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
  * These well written open source projects have been of immense help when I was in doubt, or in need of help. I've spent countless
    hours digging through the source code of Swift and Clang, and that helped me improve my programming and compiler designing skills.
    The design of the Fox interpreter has been highly influenced by the design of theses compilers. 
