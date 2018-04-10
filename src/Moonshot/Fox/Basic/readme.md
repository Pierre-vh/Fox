This folder contains all of the "Basic" utilities needed by Fox. I borrowed the name from CLang/Swift sources because I think
it's fitting for what's in there, but "Common" would also work as a folder name.

This folder contains several utilities:

* Context.hpp/.cpp : The "Compilation" context.
* Diags/ Diagnostic.hpp/.cpp, DiagnosticEngine.hpp/.cpp, IDiagConsumer.hpp, BuiltinDiagConsumers.hpp/.cpp : The Diagnostic System. To learn more, see /docs/DiagEngine.md.
* Exceptions.hpp/.cpp : std::exception derived class to throw some custom exceptions.
* Flags/, FlagsManager.hpp/.cpp : the Flag system.
