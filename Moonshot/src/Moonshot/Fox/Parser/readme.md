## Parser : implementation of the grammar rules in the Parser and AST :
### NOTE : This might not be 100% accurate ! But i'll try to keep it as accurate as possible.

Note : rules  xxx_kw   / id   / type    are already implemented through the "matchXXX" methods.

### IMPLEMENTED
 expr   
 expr_stmt
 binary_operator   
 unary_operator   
 term
 var_decl
 type_spec   
 eoi
 block   
 stmt   
 cond_cf   
 if_branch   
 elif_branch   
 else_branch   
 while_cf   
 value
### PARTIALLY IMPLEMENTED
 callable (no functions in yet)

### TO BE IMPLEMENTED   
 fox_program   
 id   
 arg_type_spec   
 func_decl   
 arg_list_decl   
 arg_decl   
 f_call   
 expr_list   
 rtr_stmt   
