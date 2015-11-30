#include "parser.h"
#include "walk_ast.h"
#include <string>
#include <iostream>
#include <cstring>

// Utility functions for transforming ASTs.

// convenient macro to encapsulate some boilerplate
// #define DO_NODE(sm_ptr) sm_ptr = ASTsubtree(structure_ast(sm_ptr.get()))

void *get_basic_block_by_label_sub(ASTsubtree &ast, ASTsubtree &root, void *parm) {
  const char *s = (const char *)(parm);

  if (ast.get()->type == BASICBLOCK) {
    auto ast_c = (BasicBlockNode *)(ast.get());
    if(strcmp(s, ast_c->label.c_str()) == 0) {
      return (void *)(ast.get());
    }
    else {
      return NULL;
    }
  } else {
    return NULL;
  }
}

// Look up a basic block in an AST by its label
// Used to determine successor nodes when turning branches into ifs
// MUST be called on a root node (probably not actually)
ASTNode *get_basic_block_by_label(ASTsubtree &ast, std::string s) {
  void *res = walk_ast(ast, ast, get_basic_block_by_label_sub, (void *)(s.c_str()));
  return (ASTNode *)(res);
}

// subfunctions for each node type (to avoid polluting namespace
// with tons of variables)
void *walk_ast_root(ASTsubtree &ast, ASTsubtree &root, walk_fun f, void *f_parm) {
  RootNode * ast_c = (RootNode *)(ast.get());
  void *res = f(ast, root, f_parm);
  res = res ? res : walk_ast(ast_c->video_tyx, root, f, f_parm);
  res = res ? res : walk_ast(ast_c->video_t, root, f, f_parm);
  res = res ? res : walk_ast(ast_c->audio, root, f, f_parm);
  for(auto &s : ast_c->subroutines) {
    res = res ? res : walk_ast(s, root, f, f_parm);
  }
  return res;
}

void *walk_ast_func_def(ASTsubtree &ast, ASTsubtree &root, walk_fun f, void *f_parm) {
  FuncDefNode * ast_c = (FuncDefNode *)(ast.get());
  void *res = f(ast, root, f_parm);
  res = res ? res : walk_ast(ast_c->body, root, f, f_parm);
  return res;
}

void *walk_ast_block(ASTsubtree &ast, ASTsubtree &root, walk_fun f, void *f_parm) {
  BlockNode * ast_c = (BlockNode *)(ast.get());
  void *res = f(ast, root, f_parm);
  for(auto &s : ast_c->children) {
    res = res ? res : walk_ast(s, root, f, f_parm);
  }
  return res;
}

void *walk_ast_basic_block(ASTsubtree &ast, ASTsubtree &root, walk_fun f, void *f_parm) {
  BasicBlockNode * ast_c = (BasicBlockNode *)(ast.get());
  void *res = f(ast, root, f_parm);
  for(auto &s : ast_c->lines) {
    res = res ? res : walk_ast(s, root, f, f_parm);
  }
  return res;
}

void *walk_ast_assignment(ASTsubtree &ast, ASTsubtree &root, walk_fun f, void *f_parm) {
  AssignmentNode * ast_c = (AssignmentNode *)(ast.get());
  void *res = f(ast, root, f_parm);
  res = res ? res : walk_ast(ast_c->leftside, root, f, f_parm);
  res = res ? res : walk_ast(ast_c->rightside, root, f, f_parm);
  return res;
}

void *walk_ast_branch(ASTsubtree &ast, ASTsubtree &root, walk_fun f, void *f_parm) {
  BranchNode * ast_c = (BranchNode *)(ast.get());
  void *res = f(ast, root, f_parm);
  res = res ? res : walk_ast(ast_c->condition, root, f, f_parm);
  return res;
}

void *walk_ast_if_then(ASTsubtree &ast, ASTsubtree &root, walk_fun f, void *f_parm) {
  IfThenNode * ast_c = (IfThenNode *)(ast.get());
  void *res = f(ast, root, f_parm);
  res = res ? res : walk_ast(ast_c->condition, root, f, f_parm);
  res = res ? res : walk_ast(ast_c->truebranch, root, f, f_parm);
  res = res ? res : walk_ast(ast_c->merged, root, f, f_parm);
  return res;
}

void *walk_ast_if_then_else(ASTsubtree &ast, ASTsubtree &root, walk_fun f, void *f_parm) {
  IfThenElseNode * ast_c = (IfThenElseNode *)(ast.get());
  void *res = f(ast, root, f_parm);
  res = res ? res : walk_ast(ast_c->condition, root, f, f_parm);
  res = res ? res : walk_ast(ast_c->truebranch, root, f, f_parm);
  res = res ? res : walk_ast(ast_c->falsebranch, root, f, f_parm);
  res = res ? res : walk_ast(ast_c->merged, root, f, f_parm);
  return res;
}

void *walk_ast_phi(ASTsubtree &ast, ASTsubtree &root, walk_fun f, void *f_parm) {
  PhiNode * ast_c = (PhiNode *)(ast.get());
  void *res = f(ast, root, f_parm);
  for(auto &s : ast_c->values) {
    res = res ? res : walk_ast(s, root, f, f_parm);
  }
  return res;
}

void *walk_ast_store(ASTsubtree &ast, ASTsubtree &root, walk_fun f, void *f_parm) {
  StoreNode * ast_c = (StoreNode *)(ast.get());
  void *res = f(ast, root, f_parm);
  res = res ? res : walk_ast(ast_c->value, root, f, f_parm);
  res = res ? res : walk_ast(ast_c->address, root, f, f_parm);
  return res;
}

void *walk_ast_load(ASTsubtree &ast, ASTsubtree &root, walk_fun f, void *f_parm) {
  LoadNode * ast_c = (LoadNode *)(ast.get());
  void *res = f(ast, root, f_parm);
  res = res ? res : walk_ast(ast_c->address, root, f, f_parm);
  return res;
}

void *walk_ast_call(ASTsubtree &ast, ASTsubtree &root, walk_fun f, void *f_parm) {
  CallNode *ast_c = (CallNode *)(ast.get());
  void *res = f(ast, root, f_parm);
  for(auto &s : ast_c->params) {
    res = res ? res : walk_ast(s, root, f, f_parm);
  }
  return res;
}

void *walk_ast_oper(ASTsubtree &ast, ASTsubtree &root, walk_fun f, void *f_parm) {
  OperNode *ast_c = (OperNode *)(ast.get());
  void *res = f(ast, root, f_parm);
  for(auto &s : ast_c->operands) {
    res = res ? res : walk_ast(s, root, f, f_parm);
  }
  return res;
}

void *walk_ast_return(ASTsubtree &ast, ASTsubtree &root, walk_fun f, void *f_parm) {
  ReturnNode *ast_c = (ReturnNode *)(ast.get());
  void *res = f(ast, root, f_parm);
  res = res ? res : walk_ast(ast_c->value, root, f, f_parm);
  return res;
}

// for all others, we just call the provided function and return the result
void *walk_ast_other(ASTsubtree &ast, ASTsubtree &root, walk_fun f, void *f_parm) {
  return f(ast, root, f_parm);
}

// jump is a noop, as is write, read, allocate, literal, noop

// "walk_ast" transformation:
// jumps into "if-then" or "if-then-else" statements
void *walk_ast(ASTsubtree &ast, ASTsubtree &root, walk_fun f, void *f_parm) {
  if(ast.get() == NULL) return NULL;
  switch(ast.get()->type) {
    case ROOT:
      return walk_ast_root(ast, root, f, f_parm);
    break;
    case FUNCTION_DEFINITION:
      return walk_ast_func_def(ast, root, f, f_parm);
    break;
    case BLOCK:
      return walk_ast_block(ast, root, f, f_parm);
    break;
    case ASSIGNMENT:
      return walk_ast_assignment(ast, root, f, f_parm);
    break;
    case CALL:
      return walk_ast_call(ast, root, f, f_parm);
    break;
    case OPERATION:
      return walk_ast_oper(ast, root, f, f_parm);
    break;
    case STORE:
      return walk_ast_store(ast, root, f, f_parm);
    break;
    case LOAD:
      return walk_ast_load(ast, root, f, f_parm);
    break;
    // TODO - need a LOOP case??
    // TODO - what about JUMP?
    case RETURN:
      return walk_ast_return(ast, root, f, f_parm);
    break;
    case BASICBLOCK:
      return walk_ast_basic_block(ast, root, f, f_parm);
    break;
    case PHI:
      return walk_ast_phi(ast, root, f, f_parm);
    break;
    case BRANCH:
      return walk_ast_branch(ast, root, f, f_parm);
    break;
    case IFTHEN:
      return walk_ast_if_then(ast, root, f, f_parm);
    break;
    case IFTHENELSE:
      return walk_ast_if_then_else(ast, root, f, f_parm);
    break;
    default:
      return walk_ast_other(ast, root, f, f_parm);
    break;
  }
  // ought to be unreachable...
  return NULL;
}
