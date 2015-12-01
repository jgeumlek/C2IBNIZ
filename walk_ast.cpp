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
BasicBlockNode *get_basic_block_by_label(ASTsubtree &ast, std::string s) {
  void *res = walk_ast(ast, ast, get_basic_block_by_label_sub, (void *)(s.c_str()));
  return (BasicBlockNode *)(res);
}

// mutates a BasicBlockNode, removing any jump to the provided label
void remove_jumps_by_label(BasicBlockNode *bblock, std::string label) {
  for(auto it = bblock->lines.begin(); it != bblock->lines.end(); ++it) {
    if((*it).get()->type == JUMP) {
      JumpNode *jn = (JumpNode *)(*it).get();
      if(label.compare(jn->label) == 0) {
        bblock->lines.erase(it);
      }
    }
  }
}

void *structure_ast_sub(ASTsubtree &ast, ASTsubtree &root, void *parm) {
  if(ast.get()->type == BRANCH) {
    BranchNode *ast_c = (BranchNode *)ast.get();
    BasicBlockNode *true_block =  get_basic_block_by_label(root, ast_c->truelabel);
    BasicBlockNode *false_block = get_basic_block_by_label(root, ast_c->falselabel);

    // current heuristic:
    // branches must have only one predecessor
    // branches must have only one immediate successor
    // this is the if-then-else case
    if(true_block->preds.size()  == 1 &&
       false_block->preds.size() == 1 &&
       true_block->succs.size()  == 1 &&
       false_block->succs.size() == 1) {

         std::string merge_label = true_block->succs.front();
         // finally, successors must be the same
         if(true_block->succs.front().compare(false_block->succs.front()) == 0) {
           BasicBlockNode *merge_block = get_basic_block_by_label(root, merge_label);
           IfThenElseNode *iten = new IfThenElseNode();
           iten->condition = ast_c->condition;
           remove_jumps_by_label(true_block, merge_label);
           iten->truebranch = ASTsubtree(true_block);
           remove_jumps_by_label(false_block, merge_label);
           iten->falsebranch = ASTsubtree(false_block);
           iten->merged = ASTsubtree(merge_block);
           ast = ASTsubtree(iten);
         }
    }
    // if-else case
    else if(true_block->preds.size() == 1 &&
            false_block->preds.size() == 2 &&
            true_block->succs.size() == 1) {

              std::string merge_label = true_block->succs.front();
              if(merge_label.compare(false_block->label) == 0) {
                BasicBlockNode *merge_block = get_basic_block_by_label(root, merge_label);
                IfThenNode *itn = new IfThenNode();
                itn->condition = ast_c->condition;
                remove_jumps_by_label(true_block, merge_label);
                itn->truebranch = ASTsubtree(true_block);
                itn->merged = ASTsubtree(false_block);
                ast = ASTsubtree(itn);
              }
    }

  }
  return NULL;
}

// convert the AST so that it uses if statements where possible
void *structure_ast(ASTsubtree &ast) {
  return walk_ast(ast, ast, structure_ast_sub, NULL);
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
