// To avoid circular dependencies, i need to do a forward declaration of the nodes in the visitors.

#pragma once

#define NODE_FORWARD_DECLARATION \
struct ASTExpr; \
struct ASTRawValue; \
struct ASTVarDeclStmt;\
struct ASTVarCall; 