Source code for FOX.

What is Fox ? Fox is the base (front end) of the interpreter. <br>
Fox includes :<br>

-> Lexical Analysis (Lexer)<br>
-> Syntaxic Analysis (Parser)<br>
-> AST<br>
-> Outputs the IR (Bytecode for Badger, a stack-based vm)<br>

Current task list:<br>
	After the parser is done:<br>
		Create the SSTBuilder class. SST stands for "Scoped Symbols Table", which is a tree of symbols table.<br>
		the SST will be passed to the revamped Semantics class.<br>

I'll split the TypeChecker class in multiple files, and have the typechecker class inherit from them.<br>

General tasklist: Before beginning implementation, create a comment in each function's defintion with how it should behave, so I cna detect potential issues
before writing a single line of code.<br>

ExprSema : Will check the rules for every node that inherits from IASTExpr<br>

	* ASTBinaryExpr			: Check if the operation is legal.
	* ASTUnaryExpr			: Check if the operation is legal
	* ASTCastExpr			: Check if the explicit cast is legal
	* ASTLiteral			: Returns the literal's type 
	* ASTVarCall			: Check if the variable is accessible, and return it's type if yes.

Tasklist:<br>

	* Find a way to represent language semantics in a effective and easy to adapt way.<br>
<br>
TypeSema : Calls ExprChecker to check on the expression trees when needed.<br>

	* ASTFunctionDeclaration	: Doesn't really check anything for now.
	* ASTVarDeclStmt			: Check if the init expr can be implicitely cast to the variable's type
	* ASTFunctionCall			: Check if the function is accessible, and then checks if each argument can be implicitely cast to the respective argument type.
	* ASTWhileLoop			: Checks if the expression can be implicitely cast to bool
	* ASTCondition			: Checks if the expression can be implicitely cast to bool
	* ASTReturnStmt			: Checks if the return statement returns the exact same type as the function's return type.
<br>
Semantics : Check a package's semantics. Calls the TypeChecker on every function declaration.<br>

	* ASTPackageRoot			: Checks every file in the package.
	* ASTSourceFile			: This function will call the TypeChecker to visit every declaration, variable or function, and visit every import statement.
	* ASTImportStmt			: Check if the import statement is valid (the file exists)
<br>
Tasklist: Find a way to store the current function's signature. (Have a private variable currentFunctionSignature with the signature of the function currently being analyzed.)<br>
Aside from that,Not much, this is going to be a normal visitor that won't return anything I think. I just need to plan it well to avoid another rewrite.<br>
<br>
After the rework, make some changes to the RTExpr class to make it more efficient and adapted to the new semantics, and remove the RTStmt class. It's not needed currently.<br>

