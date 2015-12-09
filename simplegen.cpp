#include <map>
#include <string>
#include <string.h>
#include <algorithm>

struct Variable {
  std::string name;
  int stackLoc;
};
struct VarBlockInfo {
	std::string blockName;
	int stackLoc;
};

typedef std::pair<std::string,Variable> VarEntry;
typedef std::pair<std::string, int> BBlockIndexEntry;
typedef std::pair<int, int> StackSizeEntry;

typedef std::pair<int, std::string> FuncIdEntry;
typedef std::pair<std::string, int> FuncIdRevEntry;
typedef std::pair<int,std::map<std::string,Variable>> FuncVarEntry;
typedef std::pair<int, std::map<std::string, int>> FuncBBlockEntry;

typedef std::pair<int, std::map<std::string, int>> FuncBlockToStackSizeEntry;
typedef std::pair<int, std::map<std::string, VarBlockInfo>> FuncVarToBlockInfoEntry;
typedef std::pair<int, std::map<std::string, int>> FuncBlockToHeapLocEntry;
typedef std::pair<int, int> FuncToNoOfVarEntry;
typedef std::pair<int, std::map<std::string, int>> FuncBlockToBlockNoEntry;
class output {
	public: 
	std::string val;
	int length;
	std::string debug;
	output() {
		val = "";
		debug = "";
		length = 0;	
	}
	void append(std::string s) {
		debug += (s + " " + std::to_string(s.length()) + " " + std::to_string(length + s.length()) + "\n");
		val += s;
		length += s.length();
	}
	void append(std::string s, int len) {
		debug += (s + " " + std::to_string(len) + " " + std::to_string(length + len) + "\n");
		val += s;
		length += len;
	}
		
};

class simple_translator {
public: 
   
//  std::map<int,std::map<std::string,Variable>> funcVars;
  std::map<int, std::map<std::string, int>> funcbblock;
  std::map<int, std::string> funcToId;
  std::map<int, std::map<std::string, int>> funcBlockToHeapLoc;
  std::map<int, std::map<std::string,VarBlockInfo>> funcVarToBlockInfo; 
  std::map<int, std::map<std::string, int>> funcBlockToStackSize;
  std::map<int, int> funcToNoOfVar;  
  std::map<int, std::map<std::string, int>> funcBlockToBlockNo; 
  std::string curBlockName;
   
  int heapLoc = 0;
  int heapLocForNoOfVar = 0;
  int heapLocForFuncNoOfVar = 100; 
  std::map<std::string, int> funcIdRevMap; 
  bool var_allocated(std::string name, int funcId) {
 /*  auto vars = funcVars.find(funcId)->second;
   auto it = vars.find(name);
   if (it == vars.end()) return false;
   return true;*/
return false;
  };
  int noOfFunctions = 0;
  int noOfBasicBlocks = 0; 
  int stack_ptr = 0;
  std::map<int, int> stackSize;
  int heap_ptr = 0;
  output *res = new output(); 

  void translate(ASTNode* ast) {

  // initializing the location on heap storing number of variables and the number of basic blocks
  res->append("0,", 1);
  res->append("0");
  res->append("!");
  res->append("0,", 1);
  res->append("1");
  res->append("!");

   std::cout << "translating " <<std::endl;
   if (ast->type != ROOT) return;
   RootNode* root = (RootNode*) ast;
   for(auto func : root->subroutines) {
	translateFunction(func);
   } /*
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
   translate_body(func->body);*/
   res->val = postProcess(res->val);
   std::cout << res->debug << std::endl;
  }

  void addParameter(FuncDefNode &func, int funcId) {
       //storing the number of variables on the stack before this function executes
	 res->append("0");
        res->append("@");
        res->append(std::to_string(heapLocForFuncNoOfVar + funcId), 1);
        res->append("!");
	
	std::vector<std::string> params = func.params;		
	for(int i = 0; i < params.size(); i++) {
		VarBlockInfo varBlockInfo;
		varBlockInfo.blockName =  "default";
		varBlockInfo.stackLoc = i;	
		funcVarToBlockInfo.find(funcId)->second[params.at(i)] = varBlockInfo;			  // incremeting the static count of variables in the block 
		funcBlockToStackSize.find(funcId)->second["default"] = i + 1;
	}

	
	// incrementing teh number of basic blocks and storing the block index of teh default block
	funcBlockToBlockNo.find(funcId)->second["default"] = noOfBasicBlocks++;
	// storing the number of variables before this block 
	res->append(std::to_string(heapLocForNoOfVar), 1);
	res->append("@");
	res->append(std::to_string(funcBlockToBlockNo.find(funcId)->second["default"] + 1), 1);
	res->append("!");	

	// adding the number of parameters to the number of variables and then storing it back again 
	res->append(std::to_string(params.size()) + ",",  1);
	res->append(std::to_string(heapLocForNoOfVar), 1);
	res->append("@");
	res->append("+");
	res->append(std::to_string(heapLocForNoOfVar), 1);
	res->append("!");
  }


  
  void translateFunction(ASTsubtree& ast) {
	if(ast->type != FUNCTION_DEFINITION) return;
	int funcId = noOfFunctions++;
	FuncDefNode* func = (FuncDefNode*) ast.get();
	std::cout << "function id " << funcId << " " << func->name << std::endl;	
        funcIdRevMap.insert(FuncIdRevEntry(func->name, funcId));	
			
//	std::map<std::string,Variable> varMap;
//	funcVars[funcId] = varMap;
	
	std::map<std::string, int> blockToSizeEntry;
	funcBlockToStackSize[funcId] = blockToSizeEntry;

	std::map<std::string, VarBlockInfo> varToBlockInfoEntry;
	funcVarToBlockInfo[funcId] = varToBlockInfoEntry;
	
	std::map<std::string, int> blockToBlockNo;
	funcBlockToBlockNo[funcId] = blockToBlockNo;	
	
	std::map<std::string, int>  bblockMap;
	funcbblock[funcId] = bblockMap;
	/*	for(int i = 0; i < func->params.size(); i++) {
		auto param = func->params.at(i);
		varMap[param] = {param, i};
	} */


	 // storing the number of variables before this function
	if(func->name != IBNIZ_VIDEO_TYX_NAME) {				
		res->append(std::to_string(funcId + 2000), 1);
		res->append("{");
		addParameter(*func, funcId);
		translate_body(func->body, funcId);
		res->append("}");	
	} else {
		addParameter(*func, funcId);
		translate_body(func->body, funcId);
	}		
  }

  std::string postProcess(std::string res) {
	std::cout << "result " << std::endl;
	std::cout << res << std::endl;
	std::string ans = "";
	for(auto it = res.begin(); it < res.end(); it++) {
		if(*it == 'D') {
			bool foundDelim = false;
			std::string label = "";
			it++;
			std::string funcName = "";
				
			while(*it != 'D') {
				if(*it == '%') {
					foundDelim = true;
				}
				if(!foundDelim) {
					funcName += *it;
				}
				label += *it;
				it++;
			}
			int funcId = funcIdRevMap.find(funcName)->second;
			std::cout << "func id " << funcId << " "  << label<< std::endl;	
			auto bblockMap = funcbblock.find(funcId)->second; 
			auto val = bblockMap.find(label);
			std::cout << "value " << val->second << std::endl;			
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
  void translate_body(ASTsubtree &ast, int funcId) {
   if (ast->type != BLOCK) return;
   BlockNode* block = (BlockNode*) (ast.get());
     // before every block I add the block number of the previous block, since the first block has no previous block we add a dummy 0 before we enter the first block
   res->append("0,", 1);
   for (auto it = block->children.begin(); it != block->children.end(); it++) { 
     translate_basic_block(*it, funcId);
   }
   // subtracting the number of variables before from the number of variables now
   res->append("0");
   res->append("@");
   res->append(std::to_string(heapLocForFuncNoOfVar + funcId), 1);	
   res->append("@");
   res->append("-");
/*
  //duplicating the result to use it to bury the value to be returned
   res->append("d");

 // adding 2 to it which give the location in stack of the first function variable
  res->append("1");
  res->append("+"); 

// trirot and exchange ensures we get the order we won't. 
// at the top of the stack we have the location of the first function variable on the stack, below that we have the value to be returned, below that we have 

  res->append("v");
  res->append("x");
  
// now we can bury 
  res->append("(");
  12 0 0
  12 0 1
  0 1 12
  0 12 1
 */  
// number of numbers that need to popped out has reduced bu 1 since we have added 
  // need to pop out these numbers
  
  // duplicating the condition variable since we need to ise it for if and while
  res->append("d");
  res->append("?");
  res->append("[");
  // trirot brings the function variable to the top since we have the value to be returned and the condition variable on top
  res->append("v");
  res->append("p");
  res->append(",1", 1);
  res->append("-");
  // duplicating it since the top gets removed after the check
  res->append("d");
  res->append("]");
  res->append("p");
  res->append(":");
  res->append("p");
  res->append(";");    

  //reverting the number of variables on the stack
   res->append(std::to_string(heapLocForFuncNoOfVar + funcId), 1);	
   res->append("@");
   res->append(std::to_string(heapLocForNoOfVar), 1);
   res->append("!");
  }
 
  void translate_basic_block(ASTsubtree &ast, int funcId) {
	if (ast->type != BASICBLOCK) return;
/*	// added to inrement the number of basic blocks
	res->append(std::to_string(heapLocForBlockNo), 1);
	res->append("@");
	res->append("1");
	res->append("+");
	res->append(std::to_string(heapLocForBlockNo), 1);
	res->append("!");
*/

	
   	BasicBlockNode* block = (BasicBlockNode*) (ast.get());
	funcbblock.find(funcId)->second[block->label] = res->length + 1;
	
	//incrementing the static count of the basic blocks and assi
        funcBlockToBlockNo.find(funcId)->second[block->label] = noOfBasicBlocks++;

	// storing the number of variables before this block 
	res->append(std::to_string(heapLocForNoOfVar), 1);
	res->append("@");
	res->append(std::to_string(funcBlockToBlockNo.find(funcId)->second[block->label] + 1), 1);
	res->append("!");	



	curBlockName = block->label;
	//should increment stack pointer

        // keeping track of where the basic block starts to implement jump
	
	//removing previous block no from the top of the stack if the current block does not start with a phi node
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
     		translate_line(*it, block->label, funcId);
   	}
	std::cout << res->val << " " << res->length << std::endl;
	/*			
	res->append(std::to_string(heapLocForBlockNo), 1);
	res->append("@");
	res->append("1");
	res->append("-");
	res->append(std::to_string(heapLocForBlockNo), 1);
	res->append("!");*/	
  }

  void translate_line(ASTsubtree &ast, std::string blockLabel, int funcId) {
  stack_ptr = 0;
  if (ast->type == NOOP) return;
  if (ast->type == BRANCH) { translate_branch(ast, blockLabel, funcId);}
  if (ast->type == STORE) { translate_store(ast, funcId); return;}
  if (ast->type == ASSIGNMENT) { translate_assignment(ast, funcId); return;}
  if (ast->type == RETURN) { translate_return(ast, funcId); return;}
  if (ast->type == JUMP) { translate_jump(ast, blockLabel, funcId); return;}
  if (ast->type == CALL) { translate_expr(ast, funcId); return;}
  return;  
  };

  void translate_phi(ASTsubtree & ast, int funcId) {
	if(ast->type != PHI) return;
	PhiNode* phiNode = (PhiNode*) (ast.get());
	rec(phiNode->values, phiNode->source_blocks, 0, funcId); 
	res->append("xp");	
	stack_ptr = 0; 
  }
  
  void rec(std::vector<ASTsubtree> &values, std::vector<std::string> &source_blocks, int index, int funcId) {
  		if(index < values.size()) {
			res->append("d");
			res->append("D" + source_blocks.at(index) + "D", 1);
			res->append("-");
			res->append("=");
			res->append("?");
			stack_ptr = 1;
			translate_expr(values.at(index), funcId);
			res->append(":");
			rec(values, source_blocks, index + 1, funcId);
			res->append(";");	
		}
  }

  void translate_branch(ASTsubtree &ast, std::string blockLabel, int funcId) {
	if(ast->type != BRANCH)	return;
	
	BranchNode* block = (BranchNode*) (ast.get());
	translate_expr(block->condition, funcId);	
	res->append("?");
	res->append("D" + blockLabel + "D,", 1);
	res->append("D" + block->truelabel + "D",1);
	res->append("J");	
	res->append(":");	
        res->append("D" + blockLabel + "D,", 1);	
	res->append("D" + block->falselabel + "D", 1);		
	res->append("J"); 
	res->append(";");  
//	output += "?";	
  }
  void error(std::string log, std::string function) {
 	std::cout << "ERROR : " << log << " IN " << function << std::endl; 
  }

 void translate_jump(ASTsubtree &ast, std::string blockLabel, int funcId) {
	if(ast->type != JUMP) return;
	JumpNode* jump = (JumpNode*) ast.get();
	res->append("D" + blockLabel + "D,", 1);
	res->append("D" + jump->label + "D", 1);
	res->append("J");				
 }
 
  void translate_return(ASTsubtree &ast, int funcId) {
//  int stacksize = stackSize.find(funcId)->second;
 /* for(int i = 0; i < stacksize; i++) {
  	res->append("p");
  }*/ 
  if (ast->type != RETURN) return;
  ReturnNode* ret = (ReturnNode*) ast.get();
  translate_expr(ret->value, funcId);
  return;
  }

  void translate_store(ASTsubtree &ast, int funcId) {
  if (ast->type != STORE) return;
  StoreNode* store = (StoreNode*) ast.get();
  translate_expr(store->address, funcId);
  translate_expr(store->value, funcId);
 // output += "!";
  }
  
  void translate_assignment(ASTsubtree &ast, int funcId) {
  if (ast->type != ASSIGNMENT) return;
  AssignmentNode* line = (AssignmentNode*) ast.get();
  translate_expr(line->rightside, funcId);
  translate_write(line->leftside, funcId);
  }
  
  void translate_write(ASTsubtree &ast, int funcId) {
  if (ast->type != WRITE) return;
  WriteNode* write = (WriteNode*) ast.get();
  int loc = 0;
//  auto vars = funcVars.find(funcId)->second;
  if (var_allocated(write->varname, funcId)) {
    std::cout << "DANGE : should never happen ******************" << std::endl;
  //  auto it = vars.find(write->varname);
//    loc = stacksize - it->second.stackLoc - 1;
    res->append("x");
    res->append("d");
//    res->append(std::to_string(it->second.stackLoc) + "-");
    res->append("v");   //  size, value ,  // value, size, // value, size, size
                        // value, size, size, 1 -   // value, size, 3   
                        // size value 3 //  
    res->append(std::to_string(loc) + "(", 2);
  } else {
   
    VarBlockInfo varBlockInfo;
    varBlockInfo.blockName = curBlockName;
    varBlockInfo.stackLoc = funcBlockToStackSize.find(funcId)->second[curBlockName]++;

    // incrementing the number of variables
    res->append(std::to_string(heapLocForNoOfVar), 1);  
    res->append("@");
    res->append("1");
    res->append("+");
    res->append(std::to_string(heapLocForNoOfVar), 1);
    res->append("!");
    // storing the number of elements in the stack from the current basic block as the local location of the variable
    funcVarToBlockInfo.find(funcId)->second[write->varname] = varBlockInfo;
    //size, val   
 //   funcVars.find(funcId)->second[write->varname] = {write->varname, stacksize};
 //   stackSize[funcId] = stacksize;
  }
   
  }

  void translate_expr(ASTsubtree &ast, int funcId) {
std::cout << "Expression " << ast->type <<" " <<CALL <<std::endl;
  if (ast->type == READ) {
    ReadNode* read = (ReadNode*) ast.get();
    VarBlockInfo varBlockInfo = funcVarToBlockInfo.find(funcId)->second[read->varname];
    std::cout << curBlockName << " "  << varBlockInfo.blockName << std::endl;
    if(curBlockName.compare(varBlockInfo.blockName) == 0) {
	std::cout << "comparison true" << std::endl;
    	int curBlockStackSize = funcBlockToStackSize.find(funcId)->second[curBlockName];
	int stackLoc = varBlockInfo.stackLoc;
        int loc = curBlockStackSize - stackLoc - 1;
	res->append(std::to_string(loc + stack_ptr) + ")", 2);
    } else {
        //adding 1 since heap location 0 is reserved for storing the number of variables 
	int heapLoc = funcBlockToBlockNo.find(funcId)->second[varBlockInfo.blockName] + 1;

	//getting the stack size 
	res->append(std::to_string(heapLocForNoOfVar), 1);
	res->append("@");
	
	// getting the number of variables before the current basic block 
	res->append(std::to_string(heapLoc), 1);
	res->append("@");
	
	// now we get the static index of variable in the block, adding 1 - stack_ptr since we need to add this eventually anyway 
	res->append(std::to_string(varBlockInfo.stackLoc + 1 - stack_ptr), 1);

	// by adding static index to the number of variables before the current basic block we get the index of the variable
	res->append("+");
	

	// now subtractiing gives stacksize - index - 1
	res->append("-");
	res->append(")");						
    }	
    stack_ptr++; 
    return;
  }

  if (ast->type == PHI) { translate_phi(ast, funcId); return; }

  if (ast->type == LITERAL) {
    LiteralNode* lit = (LiteralNode*) ast.get();
	std::cout << "literal " << lit->hexprint()<< std::endl;
    res->append(lit->hexprint(), 1);
    return;
  }
  if (ast->type == ALLOCATE) {
//    output += std::to_string(heap_ptr);
//    heap_ptr+=4;
    return;
  }
  if (ast->type == OPERATION) {
    translate_operation(ast, funcId);
    return;
  }
  if(ast->type == CALL) {
	std::cout << "CALL************ " << std::endl;
	CallNode *call = (CallNode*) ast.get();
	auto funcId = funcIdRevMap.find(call->func_name)->second;
	for(auto param : call->params) {
		translate_expr(param, funcId);
		res->append(",", 0);
	}
	res->append(std::to_string(funcId + 2000), 1);
 	res->append("V");		 
  }
  }

  void translate_operation(ASTsubtree &ast, int funcId) {
  if (ast->type != OPERATION) return;
  OperNode* oper = (OperNode*) ast.get();
  /*Perhaps reverse?*/
  for (auto it = oper->operands.begin(); it != oper->operands.end(); it++) {
    translate_expr(*it, funcId);
  }
  if (oper->oper_name == "add") res->append("+");
  if (oper->oper_name == "mul") res->append("*");
  if (oper->oper_name == "sin") res->append("s");
  if (oper->oper_name == "srem") res->append("%");
  if (oper->oper_name == "xor") res->append("^");
  if (oper->oper_name == "sdiv") res->append("/");
  if (oper->oper_name == "eq") res->append("-=", 1); 
  if (oper->oper_name == "sgt") res->append("->", 1);
  if (oper->oper_name == "slt") res->append("-<", 1);
  if (oper->oper_name == "shl") res->append("l");
  if (oper->oper_name == "and") res->append("l");
  if (oper->oper_name == "and") res->append("&");
  }

   
};
