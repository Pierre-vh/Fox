## Parser : implementation of the grammar rules in the Parser and AST :

Note : rules  xxx_kw   / id   / type    are already implemented through the "matchXXX" methods.

### IMPLEMENTED
 expr   
 binary_operator   
 unary_operator   


### PARTIALLY IMPLEMENTED
 term    : Missing "as  type   " (type conversion support. Note : already supported in ASTExpr. I just need to add it to the parser)
 value    : Missing  callable    support

### TO BE IMPLEMENTED 
Base rule:
 fox_program   

General rules

 namespace   
 ext_id   
 general_id   
 type_spec   
 arg_type_spec   
 eoi   
 block   
 stmt   
 callable   
        
Import:

 import_stmt   

Variables

 var_decl   
 var_assign   

Functions:

 func_decl   
 arg_list_decl   
 arg_decl   
 f_call   
 f_call_stmt   
 expr_list   
 rtr_stmt   

Control flow:

 ctrl_flow   
 cond_cf   
 if_branch   
 elif_branch   
 else_branch   
 while_cf   
