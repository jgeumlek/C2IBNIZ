#include "parser.h"
#include <map>
#include <string>

struct Variable {
  std::string name;
  int last_on_stack;
  int heap_addr;
};

typedef std::pair<std::string,Variable> VarEntry;
typedef std::pair<std::string,BasicBlockNode*> BasicBlockEntry;

class simple_translator {
public:  
  std::map<std::string,Variable> vars;
  std::map<std::string,BasicBlockNode*> blocks;
  bool var_allocated(std::string name) {
   auto it = vars.find(name);
   if (it == vars.end()) return false;
   return true;
  };
  
  int heap_ptr = 0;
  std::string output = "";
  
  bool isPhiLine(ASTsubtree &line) {
    if (line->type != ASSIGNMENT) return false;
    AssignmentNode* assignment = (AssignmentNode*) line.get();
    return (assignment->rightside->type == PHI);
  }

  void processPhiLine(ASTsubtree &line) {
    if (!isPhiLine(line)) return;
    AssignmentNode* assign = (AssignmentNode*) line.get();
    WriteNode* write = (WriteNode*) assign->leftside.get();
    std::string varname = write->varname;
    PhiNode* phi = (PhiNode*) assign->rightside.get();
    for (int i = 0; i < phi->values.size(); i++) {
      std::string blockname = phi->source_blocks.at(i);
      AssignmentNode* hoisted = new AssignmentNode();
      hoisted->leftside = ASTsubtree(assign->leftside);
      hoisted->rightside = ASTsubtree(phi->values.at(i));
      BasicBlockNode* bb = (BasicBlockNode*) blocks.find(blockname)->second;
      bb->lines.insert(bb->lines.end()-2,ASTsubtree(hoisted));
    }
  }


  void translate(ASTNode* ast) { 
   if (ast->type != ROOT) return;
   RootNode* root = (RootNode*) ast;
   translate_funcdef(root->video_tyx);
  }
   
    
  void translate_funcdef(ASTsubtree ast) {
   if (ast->type != FUNCTION_DEFINITION) return;
   FuncDefNode* func = (FuncDefNode*) ast.get();
   if (func->name != "@ibniz_run") return;
   std::string timevar = func->params.at(0);
   std::string xvar = func->params.at(1);
   std::string yvar = func->params.at(2);
   vars.insert(VarEntry(timevar,{timevar,-1,0}));
   vars.insert(VarEntry(xvar,{xvar,-1,2}));
   vars.insert(VarEntry(yvar,{yvar,-1,1}));
   heap_ptr = 3;
   output = "2!1!0!";
   //stack is now empty!
   if (func->body->type == BLOCK) {
     BlockNode* body = (BlockNode*) func->body.get();
     //log all the basic blocks, allocate space.
     for (auto it = body->children.begin(); it != body->children.end(); it++) {
       if ((*it)->type == BASICBLOCK) {
         BasicBlockNode* bb = (BasicBlockNode*)(*it).get();
         blocks.insert(BasicBlockEntry(bb->label, bb));
         vars.insert(VarEntry(bb->label,{bb->label,-1,heap_ptr}));
         heap_ptr += 1;
       }
     }
     //Process phi nodes, place relevant assignments in the previous blocks.
     for (auto it = body->children.begin(); it != body->children.end(); it++) {
       if ((*it)->type == BASICBLOCK) {
         BasicBlockNode* bb = (BasicBlockNode*)(*it).get();
         for (auto it2 = bb->lines.begin(); it2 != bb->lines.end(); it2++) {
           if ((*it2)->type == NOOP) continue;
           if (!isPhiLine(*it2)) break; //Phi nodes must come first.
           processPhiLine(*it2);
           bb->lines.erase(it2);
         }
       }
     }
   }
   translate_body(func->body);
  }
  void translate_body(ASTsubtree &ast) {
    if (ast->type != BLOCK) return;
    BlockNode* block = (BlockNode*) ast.get();
    for (auto it = block->children.begin(); it != block->children.end(); it++) {
      translate_basic_block((*it));
    }
    auto bb = (BasicBlockNode*) block->children.front().get();
    auto head = vars.find(bb->label);
    output += std::to_string(head->second.heap_addr) + "V";

  }
  void translate_basic_block(ASTsubtree &ast) {
   if (ast->type != BASICBLOCK) return;
   BasicBlockNode* block = (BasicBlockNode*) (ast.get());
   auto addr = vars.find(block->label);
   output += std::to_string(addr->second.heap_addr);
   output += "{";
   for (auto it = block->lines.begin(); it != block->lines.end(); it++) {
     translate_line(*it);
   }
   /*Optimization?: Close brace not needed if we are "jumping" away...*/
   output += "}";
  } 
  void translate_line(ASTsubtree &ast) {
  if (ast->type == NOOP) return;
  if (ast->type == STORE) { translate_store(ast); return;}
  if (ast->type == ASSIGNMENT) { translate_assignment(ast); return;}
  if (ast->type == RETURN) { translate_return(ast); return;}
  if (ast->type == JUMP) { translate_jump(ast); return;}
  if (ast->type == BRANCH) { translate_branch(ast); return;}
  return;  
  };
  void translate_branch(ASTsubtree &ast) {
  if (ast->type != BRANCH) return;
  BranchNode* branch = (BranchNode*) ast.get();
  translate_expr(branch->condition);
  output += "?";
  auto trueside = vars.find(branch->truelabel);
  output += std::to_string(trueside->second.heap_addr) + "V:";
  auto falseside = vars.find(branch->falselabel);
  output += std::to_string(falseside->second.heap_addr) + "V;";
  }
  void translate_jump(ASTsubtree &ast) {
  if (ast->type != JUMP) return;
  JumpNode* jump = (JumpNode*) ast.get();
  auto target = vars.find(jump->label);
  output += std::to_string(target->second.heap_addr) +"V";
  }
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
    heap_ptr += 1;
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
    output += (lit->hexprint()) + ",";
    return;
  }
  if (ast->type == ALLOCATE) {
    output += std::to_string(heap_ptr) + ",";
    heap_ptr+=1;
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
  if (oper->oper_name == "sub") output += "-";
  if (oper->oper_name == "shl") output += "l";
  if (oper->oper_name == "mul") output += "*";
  if (oper->oper_name == "sin") output += "s";
  if (oper->oper_name == "atan") output += "a";
  if (oper->oper_name == "sqrt") output += "q";
  if (oper->oper_name == "srem") output += "%";
  if (oper->oper_name == "eq") output += "-=";
  if (oper->oper_name == "sgt") output += "->=";
  if (oper->oper_name == "xor") output += "^";
  if (oper->oper_name == "or") output += "|";
  if (oper->oper_name == "and") output += "&";
  if (oper->oper_name == "sdiv") output += "/";
  if (oper->oper_name == "ashr") output += "r";
  }

   
};
