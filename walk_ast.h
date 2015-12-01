#ifndef C2I_WALK_AST_H
#define C2I_WALK_AST_H

#include <iostream>
#include <string>
#include <memory>
#include <sstream>
#include "parser.h"

// type for helper functions passed into walk_ast
typedef void * (*walk_fun)(ASTsubtree &, ASTsubtree &, void *);

// function for walking an AST.
// at each node, applies the function f.
// return value is the first non-null result resturned this way.
// f can ALSO modify the tree to perform transformations.
// additionally, f knows the root of the tree
// (in case walking the ast again from the root is required)
void *walk_ast(ASTsubtree &ast, ASTsubtree &root, walk_fun f, void *parm);

// get a basic block out of an AST that has the given label
BasicBlockNode *get_basic_block_by_label(ASTsubtree &ast, std::string s);

ASTNode *get_first_thing(ASTsubtree &ast);

// convert the given AST so that it uses if statements where possible
void *structure_ast(ASTsubtree &ast);

#endif // C2I_TRANSFORM_AST_H
