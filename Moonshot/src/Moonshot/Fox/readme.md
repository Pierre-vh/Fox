Source code for FOX.

What is Fox ? Fox is the base (front end) of the interpreter. 
Fox includes :

-> Lexical Analysis (Lexer)
-> Syntaxic Analysis (Parser)
-> AST
-> Outputs the IR (Bytecode for Badger, a stack-based vm)

Current task list:
	After the parser is done:
		Create the SSTBuilder class. SST stands for "Scoped Symbols Table", which is a tree of symbols table.
		the SST will be passed to the revamped Semantics class.

I'll split the TypeChecker class in multiple files, and have the typechecker class inherit from them.

General tasklist: Before beginning implementation, create a comment in each function's defintion with how it should behave, so I cna detect potential issues
before writing a single line of code.

ExprSema : Will check the rules for every node that inherits from IASTExpr
	ASTBinaryExpr			: Check if the operation is legal.
	ASTUnaryExpr			: Check if the operation is legal
	ASTCastExpr				: Check if the explicit cast is legal
	ASTLiteral				: Returns the literal's type 
	ASTVarCall				: Check if the variable is accessible, and return it's type if yes.

Tasklist:
	Find a way to represent language semantics in a effective and easy to adapt way.

TypeSema : Calls ExprChecker to check on the expression trees when needed.
	ASTFunctionDeclaration	: Doesn't really check anything for now.
	ASTVarDeclStmt			: Check if the init expr can be implicitely cast to the variable's type
	ASTFunctionCall			: Check if the function is accessible, and then checks if each argument can be implicitely cast to the respective argument type.
	ASTWhileLoop			: Checks if the expression can be implicitely cast to bool
	ASTCondition			: Checks if the expression can be implicitely cast to bool
	ASTReturnStmt			: Checks if the return statement returns the exact same type as the function's return type.

Semantics : Check a package's semantics. Calls the TypeChecker on every function declaration.
	ASTPackageRoot			: Checks every file in the package.
	ASTSourceFile			: This function will call the TypeChecker to visit every declaration, variable or function, and visit every import statement.
	ASTImportStmt			: Check if the import statement is valid (the file exists)

Tasklist: Find a way to store the current function's signature. (Have a private variable currentFunctionSignature with the signature of the function currently being analyzed.)
Aside from that,Not much, this is going to be a normal visitor that won't return anything I think. I just need to plan it well to avoid another rewrite.

After the rework, make some changes to the RTExpr class to make it more efficient and adapted to the new semantics, and remove the RTStmt class. It's not needed currently.

