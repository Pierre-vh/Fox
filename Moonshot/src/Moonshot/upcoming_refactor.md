The project is sick, and needs a refactoring, as you might have guessed if you read the comment in main.cpp 
(maybe you didn't, I don't blame you, but now you know it needs a refactoring!)</br>
</br>
Right now, my focus is finishing the parser, and add the last required nodes to the AST.</br>
It's gonna be quick, maybe 2 to 4h of work with some polish.</br>
</br>
After that and a backup to another branch, *armageddon*. This will come in different phases. </br>

## Deletion 
</br>
I'll delete a lot of code. A LOT.
Most of that code is either obsolete, or trivial to rewrite, or ill-formed. (Overcomplicated, over engineered, bad design,etc.)
</br>
The folder/files that are going to be deleted include:</br>

</br> target -> reason

* `Fox/Eval` -> Obsolete, bad code, bad design.
* `Fox/Semantic/*` -> Obsolete, will be rewritten under the new visitor system.
* `Common/Datamap`  -> Obsolete, bad code, bad design.
* `Common/Context/Options` -> Over-engineered, most of the code is pointless, I'll replace it with a much nicer system.
* `Tests` -> Bad code, bad design. *Contains a lot of copy-pasted code, can be generalized easily with some macro magic.*
* `Common/Types/Types.h:FoxValue,VarRef` Completely unneeded, obsolete. All of the code related with it is going too. This includes `Common/Types/TypeCast.*`


I'm also thinking about refactoring and deleting a lot of code in /Types. I'll take a look and remove every unused code/structs/classes/etc. </br>
Then, I'll apply some *band-aid* code to make the project compile again. 

## Replacement

Some of the code deleted earlier will need to be replaced. This is the tasklist:

* New options system. Will use an .def file -generated enum class to generate the options. Every option will be a int (because i don't need anything else!)
* New test system. Design goals : ease of use, more customizable (show condensed, show detailed, show ASTs, etc)
* New ASTLiteral nodes to express different literal types : string, char, float, int, bool


## Upgrading

I'll upgrade some outdated design patterns. Main target is the visitor system. I'll use a system inspired by Clang/Swift 
(well, it will be terribly close to their system, but I'll avoid copy-pasting and I'll adapt/simplify it to fit my needs.). </br>

New classes will include a AST Traversal class, a upgraded ASTVisitor class, ASTWalker class. I'll also remove completely the accept/visit methods. </br>

</br>
I might also switch to CMake build system, but I'm pretty unfamiliar with that so there's no guarantees on that side. </br>
If I switch to CMake I might start coding on CMake more, heck, maybe even install Linux on my laptop and code more on linux!


## Moving on

Well, once I'm done I'll continue with the language implementation. That includes:

* ASTContext
* ParserDriver 
* DeclContext (support for the symbols table)
* Distinction between resolved/unresolved IDs in the nodes instead of just a string for everything
* Name resolver, semantics checker for decl,stmt,expr.
* *If the project is still going strong here, it will be time for designing the IR & Badger Bytecode, generating it, optimizing it and translating it to Badger BC! (*
