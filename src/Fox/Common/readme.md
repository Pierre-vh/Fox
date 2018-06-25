This folder contains all of the utilities that are used by every other part of Fox.

This folder contains several utilities:

* Context.hpp/.cpp : The "Compilation" context.
* Diags/, Diagnostic.hpp/.cpp, DiagnosticEngine.hpp/.cpp, IDiagConsumer.hpp, BuiltinDiagConsumers.hpp/.cpp : The Diagnostic System. To learn more, see /docs/DiagEngine.md.
* Exceptions.hpp/.cpp : std::exception derived class to throw some custom exceptions.
* Flags/, FlagsManager.hpp/.cpp : the Flag system.
* Memory.hpp -> Memory related stuff, typedefs and iterators.
* Utils.hpp -> General purpose utilities