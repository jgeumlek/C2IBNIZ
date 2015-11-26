#include <map>
#include <string>
#include <string.h>
#include <algorithm>
#define START_STACK_LOC = 2

struct Variable {
  std::string name;
  int stackLoc;
};

typedef std::pair<std::string,Variable> VarEntry;
typedef std::pair<std::string, int> BBlockIndexEntry;

class output {
	public: 
	std::string val;
	int length;
	output() {
		val = "";
		length = 0;	
	}
	void append(std::string s) {
		val += s;
		length += s.length();
	}
	void append(std::string s, int len) {
		val += s;
		length += len;
	}
		
};

class simple_translator {
public:  
  std::map<std::string,Variable> vars;
  std::map<std::string, int> bblock;
  std::map<std::string, std::string> basicBlockOutput;
  bool var_allocated(std::string name) {
   auto it = vars.find(name);
   if (it == vars.end()) return false;
   return true;
  };
  
  int stack_ptr = 0;
  int stackSize = 0;
  int heap_ptr = 0;
  output *res = new output(); 

  void translate(ASTNode* ast) {
   if (ast->type != FUNCTION_DEFINITION) return;
   FuncDefNode* func = (FuncDefNode*) ast;
   if (func->name != "@ibniz_run") return;
   std::string timevar = func->params.at(0);
   std::string xvar = func->params.at(1);
   std::string yvar = func->params.at(2);
   vars.insert(VarEntry(timevar,{timevar,0}));
   vars.insert(VarEntry(xvar,{xvar,1}));
   vars.insert(VarEntry(yvar,{yvar,2}));
  	 
   heap_ptr = 0;
   
   stackSize +=3;
   //stack is now empty!
   translate_body(func->body);
   res->val = postProcess(res->val);
  }


  std::string postProcess(std::string res) {
	std::string ans = "";
	for(auto it = res.begin(); it < res.end(); it++) {
		if(*it == 'b') {
			std::string label = "";
			it++;
			while(*it != 'b') {
				label += *it;
				it++;
			}
			auto val = bblock.find(label);
			ans += getHexaDecimal(getFirst16Bits(val->second));
			ans += ".";
			ans += getHexaDecimal(getLast16Bits(val->second));	
		} else {
			ans += *it;
		} 
	}
	return ans;			
  }

  std::string getHexaDecimal(int num) {
  	std::stringstream stream;
        stream << std::hex << num;
        std::string result( stream.str() );
        std::transform(result.begin(), result.end(), result.begin(), toupper);
        while(result.size() < 4) {
 	       result = "0" + result;
        }
	return result;
  }

  int getLast16Bits(int num) {
	return (num & 0xffff);
  }

  int getFirst16Bits(int num) {
 	return (num & 0xffff0000);
  }
  void translate_body(ASTsubtree &ast) {
   if (ast->type != BLOCK) return;
   BlockNode* block = (BlockNode*) (ast.get());
   res->append("0");
   for (auto it = block->children.begin(); it != block->children.end(); it++) {
     translate_basic_block(*it);
   }
  }
 
  void translate_basic_block(ASTsubtree &ast) {
	if (ast->type != BASICBLOCK) return;
   	BasicBlockNode* block = (BasicBlockNode*) (ast.get());
	bblock.insert(BBlockIndexEntry(block->label, res->length));
	bool removePreviousBlockNo = true;
	if(block->lines.size() > 0) {
		for (int i = 0; i < block->lines.size(); i ++) { 
			if(block->lines[i]->type != NOOP) {
				if(block->lines[i]->type == ASSIGNMENT && ((AssignmentNode*) block->lines[i].get())->rightside->type == PHI)
				{
					removePreviousBlockNo = false;	
				}
				break;
			}
		}
	}

	if(removePreviousBlockNo) 
		res->append("p");


   	for (auto it = block->lines.begin(); it != block->lines.end(); it++) {
     		translate_line(*it, block->label);
   	}				
  }

  void translate_line(ASTsubtree &ast, std::string blockLabel) {
  stack_ptr = 0;
  if (ast->type == NOOP) return;
  if (ast->type == BRANCH) { translate_branch(ast, blockLabel);}
  if (ast->type == STORE) { translate_store(ast); return;}
  if (ast->type == ASSIGNMENT) { translate_assignment(ast); return;}
  if (ast->type == RETURN) { translate_return(ast); return;}
  if (ast->type == JUMP) { translate_jump(ast, blockLabel); return;}
  return;  
  };

  void translate_phi(ASTsubtree & ast) {
	if(ast->type != PHI) return;
	PhiNode* phiNode = (PhiNode*) (ast.get());
	rec(phiNode->values, phiNode->source_blocks, 0); 
	res->append("xp");	 
  }
  
  void rec(std::vector<ASTsubtree> &values, std::vector<std::string> &source_blocks, int index) {
  		if(index < values.size()) {
			res->append("d");
			res->append("b" + source_blocks.at(index) + "b", 1);
			res->append("-");
			res->append("=");
			res->append("?");
			translate_expr(values.at(index));
			res->append(":");
			rec(values, source_blocks, index + 1);
			res->append(";");	
		}
  }

  void translate_branch(ASTsubtree &ast, std::string blockLabel) {
	if(ast->type != BRANCH)	return;
	
	BranchNode* block = (BranchNode*) (ast.get());
	translate_expr(block->condition);	
	res->append("?");
	res->append("b" + blockLabel + "b,", 1);
	res->append("b" + block->truelabel + "b",1);
	res->append("J");	
	res->append(":");	
        res->append("b" + blockLabel + "b,", 1);	
	res->append("b" + block->falselabel + "b", 1);		
	res->append("J"); 
	res->append(";");  
//	output += "?";	
  }
  void error(std::string log, std::string function) {
 	std::cout << "ERROR : " << log << " IN " << function << std::endl; 
  }

 void translate_jump(ASTsubtree &ast, std::string blockLabel) {
	if(ast->type != JUMP) return;
	JumpNode* jump = (JumpNode*) ast.get();
	res->append("b" + blockLabel + "b,", 1);
	res->append("b" + jump->label + "b", 1);
	res->append("J");				
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
 // output += "!";
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
  int loc = 0;
  if (var_allocated(write->varname)) {
    auto it = vars.find(write->varname);
    loc = stackSize - it->second.stackLoc - 1;
    res->append(std::to_string(loc) + "(");
  } else { 
    vars.insert(VarEntry(write->varname,{write->varname,stackSize++}));
  }
  }

  void translate_expr(ASTsubtree &ast) {
  if (ast->type == READ) {
    ReadNode* read = (ReadNode*) ast.get();
    auto it = vars.find(read->varname);
    int loc = stackSize - it->second.stackLoc - 1;
    res->append((std::to_string(loc + stack_ptr) + ")" ));
    stack_ptr++; 
    return;
  }

  if (ast->type == PHI) { translate_phi(ast); return; }

  if (ast->type == LITERAL) {
    LiteralNode* lit = (LiteralNode*) ast.get();
    res->append(std::to_string(lit->value));
    return;
  }
  if (ast->type == ALLOCATE) {
//    output += std::to_string(heap_ptr);
//    heap_ptr+=4;
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
  if (oper->oper_name == "add") res->append("+");
  if (oper->oper_name == "mul") res->append("*");
  if (oper->oper_name == "sin") res->append("s");
  if (oper->oper_name == "srem") res->append("%");
  if (oper->oper_name == "xor") res->append("^");
  if (oper->oper_name == "sdiv") res->append("/");
  if (oper->oper_name == "eq") res->append("-="); 
  }

   
};
