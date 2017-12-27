#pragma once
// To avoid circular dependencies, i need to do a forward declaration of the nodes.

#define NODE_FORWARD_DECLARATION \
struct ASTExpr; \
struct ASTRawValue; \
struct ASTVarDeclStmt;\
struct ASTVarCall; 