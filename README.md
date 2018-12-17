# Moonshot
Moonshot is a project that aims to create an interpreter for a language called Fox.
This is currently a work in progress, only a fraction of the project is completed.

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
