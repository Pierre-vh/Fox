# Fox Post-Mortem

This is a documented intended to reflect on my work on Fox, highlighting development mistakes. 
This isn't really complete, but I think I captured the most important stuff.

## Things I Learned

- CMake
    - I started the project using Visual Studio Solutions, but I wanted more portability. I switched to CMake as my build system and learned a ton about it, and I like it a lot now!
- The importance of:
  - include/source separation: I was really skeptical about that, so I started the project mixing headers and implementation. It became a mess quite rapidly and I ended up separating them. I never really regretted it.
  - Documentation: At first, I didn't document that much, and when I did it was done poorly. Now I tend to document much more using doxygen-style comments, and I really prefer that.
  - Testing: back when I started the project, I didn't have unit-tests. Well, I soon realized how important unit-tests were and started writing them. I unit-test pretty much everything that can be unit-tested now, and I love it. It makes finding problems much easier.
      - I also discovered LLVM lit and FileCheck, and I absolutely love them. They make testing language features, parsing, typechecking and bytecode generation infinitely easier.
  - Coding conventions: Being consistent (with identation, style, naming variables, etc.) is *really* important. It quickly becomes a mess if you aren't. I won't pretend that I'm now fully consistent, but I got a lot better!
  - Assertions: Assertions everywhere saves hours of debugging!
- Compiler/Interpreters: I learned a ton, more than any book would have ever taught me. The list is too long to fit here, trust me. Pretty much every component of the project has taught me a **TON** of things.

## Mistakes I Made

- Trying to bite more than I could chew & Overthinking simple things: I often planned too much in advanced and over-engineered some parts of the project *just in case*™. The Type/TypeBase* separation is a good example of an over-engineered part of the project.
- Relying on Swift/Clang too much: Often, I feared to try new things and relied too much on how the Swift/Clang compilers worked. Now, some parts of my project are too inspired by them and could be designed a lot better for their actual use cases. (e.g. Type/TypeBase, DeclContext)
    - However, it was greatly beneficial for other things: DiagnosticEngine, ASTVisitor/Walkers, the Lexer, etc. The design ideas I borrowed from Swift were often a lot more efficient than what I initially did.
- Not planning enough. In the end, it wasn't a bad thing because trial-and-error taught me a lot, but spending more time on planning could've saved me a few months of time.
