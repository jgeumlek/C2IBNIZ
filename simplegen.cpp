#include "parser.h"
#include <map>
#include <string>

struct Variable {
  std::string name;
  int last_on_stack;
  int heap_addr;
};

typedef std::pair<std::string,Variable> VarEntry;

class simple_translator {
public:  
  std::map<std::string,Variable> vars;
  bool var_allocated(std::string name) {
   auto it = vars.find(name);
   if (it == vars.end()) return false;
   return true;
  };
  
  int heap_ptr = 0;
  std::string output = "";
  
  void translate(ASTNode* ast) {
   if (ast->type != FUNCTION_DEFINITION) return;
   FuncDefNode* func = (FuncDefNode*) ast;
   if (func->name != "@ibniz_run") return;
   std::string timevar = func->params.at(0);
   std::string xvar = func->params.at(1);
   std::string yvar = func->params.at(2);
   vars.insert(VarEntry(timevar,{timevar,-1,0}));
   vars.insert(VarEntry(xvar,{xvar,-1,4}));
   vars.insert(VarEntry(yvar,{yvar,-1,8}));
   heap_ptr = 12;
   output = "8!4!0!";
   //stack is now empty!
   translate_body(func->body);
  }
  void translate_body(ASTsubtree &ast) {
   if (ast->type != BLOCK) return;
   BlockNode* block = (BlockNode*) (ast.get());
   for (auto it = block->children.begin(); it != block->children.end(); it++) {
     translate_line(*it);
   }
  } 
  void translate_line(ASTsubtree &ast) {
  if (ast->type == NOOP) return;
  if (ast->type == STORE) { translate_store(ast); return;}
  if (ast->type == ASSIGNMENT) { translate_assignment(ast); return;}
  if (ast->type == RETURN) { translate_return(ast); return;}
  return;  
  };
  void translate_return(ASTsubtree &ast) {
  if (ast->type != RETURN) return;
  ReturnNode* ret = (ReturnNode*) ast.get();
  translate_expr(ret->value);
  return;
  }
  void translate_store(ASTsubtree &ast) {
  if (ast->type != STORE) return;
  StoreNode* store = (StoreNode*) ast.get();
  translate_expr(store->address);
  translate_expr(store->value);
  output += "!";
  }
  void translate_assignment(ASTsubtree &ast) {
  if (ast->type != ASSIGNMENT) return;
  AssignmentNode* line = (AssignmentNode*) ast.get();
  translate_expr(line->rightside);
  translate_write(line->leftside);
  }
  void translate_write(ASTsubtree &ast) {
  if (ast->type != WRITE) return;
  WriteNode* write = (WriteNode*) ast.get();
  int loc = heap_ptr;
  if (var_allocated(write->varname)) {
    auto it = vars.find(write->varname);
    loc = it->second.heap_addr;
  } else {
    vars.insert(VarEntry(write->varname,{write->varname,-1,heap_ptr}));
    heap_ptr += 4;
  }
  output += std::to_string(loc);
  output += "!";
  }
  void translate_expr(ASTsubtree &ast) {
  if (ast->type == READ) {
    ReadNode* read = (ReadNode*) ast.get();
    auto it = vars.find(read->varname);
    int loc = it->second.heap_addr;
    output += std::to_string(loc);
    output += "@";
    return;
  }
  if (ast->type == LITERAL) {
    LiteralNode* lit = (LiteralNode*) ast.get();
    output += std::to_string(lit->value);
    return;
  }
  if (ast->type == ALLOCATE) {
    output += std::to_string(heap_ptr);
    heap_ptr+=4;
    return;
  }
  if (ast->type == OPERATION) {
    translate_operation(ast);
    return;
  }
  }

  void translate_operation(ASTsubtree &ast) {
  if (ast->type != OPERATION) return;
  OperNode* oper = (OperNode*) ast.get();
  /*Perhaps reverse?*/
  for (auto it = oper->operands.begin(); it != oper->operands.end(); it++) {
    translate_expr(*it);
  }
  if (oper->oper_name == "add") output += "+";
  if (oper->oper_name == "mul") output += "*";
  if (oper->oper_name == "sin") output += "s";
  if (oper->oper_name == "srem") output += "%";
  if (oper->oper_name == "xor") output += "^";
  if (oper->oper_name == "sdiv") output += "/";
  }

   
};
