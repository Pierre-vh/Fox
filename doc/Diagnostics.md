# The Fox Diagnostic System

### Introduction
In this file I'm going to *try* to explain how diagnostic emission is handled in the project. This will be very brief but I hope it'll help, and feel
free to message me if you have any questions! I'd love to answer them!

### The components
The diagnostic system is made of 3 distinct components.
* The DiagnosticEngine: this class handles the creation of diagnostic and coordinates their emission. It's the core of the whole system.
* The Diagnostic object: this class contains the information related to a specific diagnostic and offers methods to build it (replace placeholders for example)
* The DiagnosticConsumers: this class is called when we want to consume a diagnostic. A consumer might display it to the user, log it to a file, etc.

### The chain of events:
* 1) Somewhere, someone calls `.report(...)` on a DiagnosticEngine
  * 1) The information relative to this diagnostic is retrieved: it's severity and text
  * 2) The diagnostic's severity is demoted/promoted according to the options in the DiagnosticsEngine
    * For instance, is where we'll promote a warning to an error if we need to.
  * 3) A "Diagnostic" is created and returned.
* 2) The client manipulates the diagnostic object, for instance:
  * Placeholders (`%0`, `%1\, ...) are replaced with proper information
  * An "extended" range is set
    * The extended range of the diagnostic is usually the complete block of code affected by the error. e.g. When we report a bad use of 
    a unary operator, we'll emit a diagnostic at the unary operator's position, and set the extended range to be the range of the child expression, which
    creates a diagnostic where the caret (`^`) is displayed below the '+' and the expression is underlined with tildes (`~`)
* 3) The diagnostic object's destructor is called (it goes out of scope) or `.emit()` is called on the diagnostic: `DiagnosticEngine::handleDiagnostic` is called
* 4) `handleDiagnostic` calls the DiagnosticConsumer's `consume` method on the diagnostic.
  * For instance, if the consumer is the builtin `StreamDiagConsumer`, the diagnostic is presented to the user.
* 5) The DiagnosticEngine update it's internal counters: error count, warning count, etc.
* 6) Finally, the DiagnosticEngine checks if the error limit is not exceeded.
  * If we exceed it, any further diagnostic is ignored and a fatal "Current error count exceeded the maximum thresold of x" error is emitted.
  
