The Diagnostic system is quite simple.

## The .def files

The .def files lists every possible diagnostic using macros. A diagnostic
is definition looks like this:

	`DIAG(Severity,Id,Text)`. Severity is a DiagSeverity enum member, id is the id of the diagnostic (logic) and
	text is the text.. yeah. It's that simple. The text can have simple tags like `%0` to act like placeholders for arguments.
	Placeholders are removed by a method `addArg`, which replaces the placeholder with a int, a string, anything! After processing ,the diagnostic
	might look like `foo is not a variable name`

## The DiagnosticEngine

The engine controls the diagnostic creations through `report(DiagID)`, it also owns a DiagConsumer, which consumes the emitted diagnostics, and several options, that can be used
to make all errors fatal, or all warnings errors, or mute them all!


## The DiagConsumer

The DiagConsumer is a quite simple interface which has a single function `consume(Diag)`. This function takes a diagnostic and emits the info however it wants.
`BuiltinDiagConsumers` contains several basic implementations, one of them is used to print the diags to cout.

## The Diagnostic class

This class has 2 main goals : hold information on the current diagnostic (as well as a pointer to the current diagconsumer), and offer formatting function on said information, mainly on the string to replace placeholders.
On destruction, or when `emit()` is called, the Diagnostic calls the consumer to consume it, then, the diagnostic "kills" itself. (After that it's sort of neutralized, and can't be used again).
