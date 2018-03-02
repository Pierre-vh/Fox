Source code for FOX.

What is Fox ? Fox is the base (front end) of the interpreter. 
Fox includes :

-> Lexical Analysis (Lexer)
-> Syntaxic Analysis (Parser)
-> AST
-> Outputs the IR (Bytecode for Badger, a stack-based vm)

Pending rework: 
	Semantic checker

I'll split the TypeChecker class in multiple files, and have the typechecker class inherit from them.

General tasklist: Before beginning implementation, create a comment in each function's defintion with how it should behave, so I cna detect potential issues
before writing a single line of code.

ExprChecker : Will check the rules for every node that inherits from IASTExpr
	ASTBinaryExpr			: Check if the operation is legal.
	ASTUnaryExpr			: Check if the operation is legal
	ASTCastExpr				: Check if the explicit cast is legal
	ASTLiteral				: Returns the literal's type 
	ASTVarCall				: Returns the variable's type 

Tasklist:
	Find a way to represent language semantics in a effective and easy to adapt way.

TypeChecker : Calls ExprChecker to check on the expression trees when needed.
	ASTFunctionDeclaration	: Doesn't really check anything for now.
	ASTVarDeclStmt			: Check if the init expr can be implicitely cast to the variable's type
	ASTFunctionCall			: Check if each argument can be implicitely cast to the respective argument type.
	ASTWhileLoop			: Checks if the expression can be implicitely cast to bool
	ASTCondition			: Checks if the expression can be implicitely cast to bool
	ASTReturnStmt			: Checks if the return statement returns the exact same type as the function's return type.

Tasklist: Find a way to store the current function's signature. (Have a private variable currentFunctionSignature with the signature of the function currently being analyzed.)
Aside from that,Not much, this is going to be a normal visitor that won't return anything I think. I just need to plan it well to avoid another rewrite.