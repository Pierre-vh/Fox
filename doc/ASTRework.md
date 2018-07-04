# AST 3.0

Oh boy, here I go reworking again!

This time, it's going to be major rework of the AST.

* ### What's wrong with the current AST?  
  * The current AST is too simple and rigid. It's barely modifiable right after Parsing, and for Semantic Analysis that's not good.
    Semantic Analysis should be able to alter the AST however it wants to by adding, replacing and deleting nodes. Also, the constant
    use of `std::unique_ptr` expose the internals of the AST a bit too much to my liking.
    
* ## What's going to change?
  * First of all, the ASTContext is going to be the one to own the memory.
    It will have several memory "pools" for Decls/Stmts/Exprs provided by a *Linear Allocator*, which is essentially a "Pointer Bump Allocator"
    (It's a really fast type of allocator with dirt cheap allocations/deallocations)
  * Every `std::unique_ptr` in nodes will become raw pointers. This will make it incredibly easier to replace nodes in the AST.
  
  * All three hierarchies will become separated. Expr will be it's own hierarchy, and not a derived of Stmt.
   * A "Node" class will be created, which will be a discriminated union of Stmt/Expr/Decl pointers. This "Node" class will be used by nodes
     when they'll need to accept different type of nodes (such as a CompoundStmt body. The content of it's body will be a `std::vector<Node>`
      * The node class will have to implement some operators to make as painless to use as possible, such as `=`, `==`, `!=` and some
      member functions like `isExpr`, `isDecl`, `isStmt`, `getAsExpr`, `getAsDecl`, `getAsStmt`
      
  * Expressions will be divided in 2 "types". Resolved and Unresolved.
    * Resolved expressions are unambiguous expressions. Like a literal, a binary expression, etc.
    * Unresolved expressions are expressions that could be translated to 2 different things, like a "DotSyntax" expression. (e.g. `foo.bar`)
      * A MemberOf Expression can be translated to a UFCS (a Function call) or a "MemberOf" expr.
    * The ASTContext will probably allocate Resolved and Unresolved exprs in 2 separate pools, so the UnresolvedExpr pool can be freed after Semantic Analysis if possible.
      
* ## What's going to be added?
  * ASTReplacer. This special visitor will be given a Node A and a Node B, then it's going to traverse the AST and search for the node A to 
    "swap" it with Node B. This will fullfil the "nodes must be replacable" requirement.
  * If needed, a ASTDeleter will be added too, which will work pretty much the same as the ASTReplacer but will be tasked to delete the node A.

* ## How are we going to get there?
  * Prerequisites
    * Implement the classof operators
    * Add LLVM's special casting function and use them to remove RTTI completely
    * Finish Writing/Polishing/Testing the LinearAllocator
  * Then, once theses are fullfilled, I'll follow the "What's going to change?" list.
  
* ## Consequences
  * Cleaner AST
  * Easier to manipulate AST
  * Faster parsing times
  * Will allow for a more expressive AST (through Resolved/Unresolved Exprs) 
  * Will allow for a cleaner Semantic Analysis, with less hacks.
