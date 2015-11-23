#include "parser.h"
#include <cstring>


#define PARSE_DEBUG true

/*helpful macros. Note that START_PARSE can't be wrapped, since we want to declare in scope.*/
#define START_PARSE(X) auto old_it = it; X *node = new X(); if (PARSE_DEBUG) std::cerr<<prefix<<"Starting parse " << ASTtypenames[node->type] << std::endl; GO_DOWN;

std::string prefix  = "";
#define GO_DOWN if (PARSE_DEBUG)prefix.push_back(' ');
#define GO_UP if (PARSE_DEBUG)prefix.erase(prefix.begin());

#define FAIL_PARSE do {it = old_it; delete node; GO_UP;return nullptr;} while(0);

#define SUCCEED_PARSE do {GO_UP;if (PARSE_DEBUG)std::cerr<<prefix<<"Parse of " << ASTtypenames[node->type] << " suceeded" <<std::endl; return node;} while(0);

#define PARSE_ERR(X) do {if (PARSE_DEBUG)std::cerr<<"FATAL:"<<X<<std::endl;GO_UP;it = old_it; delete node; return nullptr;} while(0);

#define READ_OF_TYPE(X) do { if (!check_next(tokens,it,X)) {\
if (it!=tokens.end()&&(it+1)!=tokens.end()) \
   std::cerr<<prefix<<"Wanted type:"<<tokentypenames[X]<<" Got:"<< tokentypenames[(it+1)->type] <<"("<<(it+1)->text<<")" << std::endl;\
FAIL_PARSE;}; it++;} while(0);

#define READ_TEXT(X) do { \
if (!check_next_text(tokens,it,X))\
{\
if (it!=tokens.end()&&(it+1)!=tokens.end()) \
  std::cerr<<prefix<<"Wanted:"<<X<<" Got:\""<< (it+1)->text << "\""<<std::endl;\
FAIL_PARSE;};\
it++;} while(0);


std::string tokentypenames[] = {"IDENT","VAR","CONSTANT","EQUALS","OPENBRACE","ENDLINE","ENDLINE"};
std::string ASTtypenames[] = { "ROOT","FUNCTION_DEFINITION", "BLOCK", "ASSIGNMENT", "CALL", "OPERATION", "READ","WRITE", "STORE", "LOAD", "LITERAL","LOOP",  "RETURN", "NOOP", "ALLOCATE", "UNKNOWN","BASIC_BLOCK","PHI","JUMP","BRANCH", };

//empty string dentes the end of this array
std::string ignoredwords[] = {"i1","exact","tail","notail","musttail","zeroext","signext","inreg","void","noreturn","nounwind","readonly","readnone","nuw","nsw",""};

bool ignoreword(std::string &str) {
  for (int i = 0; !ignoredwords[i].empty(); i++) {
    if (str == ignoredwords[i]) return true;
  }
  return false;
}

char delim[] = {'{','}',' ',',',';','(',')','[',']','\0'};

bool isDelim(char c) {
    for (int i = 0; delim[i];i++)
        if (c == delim[i]) return true;
    return false;
}

void check_for_token(std::string &str, std::vector<token> &out) {
    std::string word = str;
    str.clear();
    if (word.empty()) return;
    if (word.front() == '#' || word.front() == '!') return;
    if (ignoreword(word)) return;
    
    if (!strncmp(word.c_str(),"<label>:",8)) {
        token token;
        token.type = LABEL;
        token.text = "%" + std::string(word.c_str()+8);
        out.push_back(token);
        return;
    }
    if (word.front() == '%' || word.front() == '@') {
        out.push_back({VAR,word});
        return;
    }
    if (word == "=") {
        out.push_back({EQUALS,word});
        return;
    }
    out.push_back({IDENT,word});
    word.clear();
}

void check_for_delim(char c, std::vector<token> &out) {
    if (c == '(' || c == '{' || c == '[') out.push_back({OPENBRACE,"{"});
    if (c == ')' || c == '}' || c == ']') out.push_back({CLOSEBRACE,"}"});
}
    
    

void tokenize(std::istream &in, std::vector<token> &out) {
    std::string line;
    while(!in.fail() && !in.eof()) {
        std::getline(in,line,'\n');
        std::string word;
        for (char c : line) {
            if (isDelim(c)) {
                if (word == "attributes") break;
                if (word == "align") {word.clear();break;}
                check_for_token(word,out);
                check_for_delim(c,out);
            } else {
            if (c != '\n') word.push_back(c);
            }
        }
         check_for_token(word,out);
        out.push_back({ENDLINE,"\n"});
    }
}



typedef std::vector<token>::iterator tokIter;

/*PARSER INVARIANTS
 * On a failed parse, the iterator should be unwound.
 * At the start of the parse, the iterator points to the last read token.
 * At the end of the parse, the iterator points to the last read token.
 */

/*Awful icky globals, instead of actually passing this information down.*/
std::string current_function = "";
BasicBlockNode* current_block = nullptr;

bool check_next(std::vector<token> &tokens, tokIter &it, enum tokentype type) {
    if (it == tokens.end()) return false;
    it++;
    if (it == tokens.end()) { it--; return false;}
    if (it->type != type) { it--; return false;}
    it--;
    return true;
}
bool check_next_text(std::vector<token> &tokens, tokIter &it, std::string text) {
    if (it == tokens.end()) return false;
    it++;
    if (it == tokens.end()) { it--; return false;}
    if (it->text != text) { it--; return false;}
    it--;
    return true;
}
bool skipTo(std::vector<token> &tokens, tokIter &it, enum tokentype type) {
    while (it != tokens.end() && it->type != type) it++;
    if (it == tokens.end()) return false;
    return true;
}
ASTNode* parse_expression(std::vector<token> &tokens, tokIter &it);

ASTNode* parse_literal(std::vector<token> &tokens, tokIter &it) {
  START_PARSE(LiteralNode);
  //IF WE ADD A "LITERAL" TOKEN TYPE, FIX THIS!
  READ_OF_TYPE(IDENT);
  int value = 0;
  std::string text = it->text;
  try {
       if (text != "double") throw 1;
       if (!check_next(tokens,it,IDENT)) throw 1;
       it++; text = it->text;
       double dvalue = std::stod(text);
       node->value = (int) (dvalue * (1 << 16));
       SUCCEED_PARSE;
  } catch(...) {};
  try {
          value = std::stoi(text);
          //NOTE: above stoi has some quirks. "123abc" -> 123, even though there is text.
          //POSSIBLY DO SOME BOUNDS CHECKING?
          node->value = value<<16;
          SUCCEED_PARSE;
  } catch (...) {
    //stoi exception when it fails.
    PARSE_ERR(text + " was not any expression?");
  }
  return nullptr;
}
//empty string denotes end of array.
std::string operators[] = {"add","sub","mul","sdiv","srem","sin","sqrt","atan","xor","and","or","ne","eq","ugt","uge","ule","sgt","sge","slt","ult","sle","shl","lshr","ashr",""};
bool check_oper(std::string oper) {
  for (int i = 0; !operators[i].empty(); i++) {
    if (operators[i] == oper) return true;
  }
  return false;
}

ASTNode* parse_operation(std::vector<token> &tokens, tokIter &it) {
  START_PARSE(OperNode);
  //skip icmp, we care about the compare operator itself
  if (check_next_text(tokens,it,"icmp")) it++;
  READ_OF_TYPE(IDENT);
  node->oper_name = it->text;
  if (!check_oper(it->text)) FAIL_PARSE;
  if (check_next_text(tokens,it,"nsw")) it++;
  ASTNode* expr = parse_expression(tokens,it);
  while (expr) {
    node->operands.push_back(ASTsubtree(expr));
    expr = parse_expression(tokens,it);
  }
  if (node->oper_name == "xor" && node->operands.back()->to_string() == "lit_-1") {
    node->oper_name = "complement";
    node->operands.erase(node->operands.end()-1);
  }
  SUCCEED_PARSE;
}
  

ASTNode* parse_load(std::vector<token> &tokens, tokIter &it) {
  START_PARSE(LoadNode);
  READ_TEXT("load");
  READ_TEXT("at");
  ASTNode* addr = parse_expression(tokens,it);
  if (!addr) PARSE_ERR("load had no address?");
  node->address = ASTsubtree(addr);
  SUCCEED_PARSE;
}

ASTNode* parse_allocate(std::vector<token> &tokens, tokIter &it) {
  START_PARSE(AllocateNode);
  READ_TEXT("allocate");
  SUCCEED_PARSE;
}

ASTNode* parse_read(std::vector<token> &tokens, tokIter &it) {
  START_PARSE(ReadNode);
  READ_OF_TYPE(VAR);
  node->varname = it->text;
  SUCCEED_PARSE;
}
ASTNode* parse_phi(std::vector<token> &tokens, tokIter &it) {
  START_PARSE(PhiNode);
  READ_TEXT("phi");
  while(check_next(tokens,it,OPENBRACE)) {
    it++;
    ASTNode* expr = parse_expression(tokens,it);
    if (!expr) PARSE_ERR("phi node had a bad value?");
    if (!check_next(tokens,it,VAR)) { delete expr; PARSE_ERR("phi node had bad predecessor?");}
    it++;
    std::string pred = it->text;
    if (!check_next(tokens,it,CLOSEBRACE)) { delete expr; PARSE_ERR("phi node format...");};
    it++;
    node->values.push_back(ASTsubtree(expr));
    node->source_blocks.push_back(current_function+pred);
  }
  SUCCEED_PARSE;
}
ASTNode* handle_builtins(CallNode* call) {
/*This function translates function calls into operators,
  since IBNIZ provides a few for us*/
    if (call->func_name == "@sin" || call->func_name == "@atan" || call->func_name == "@sqrt") {
        OperNode* oper = new OperNode();
        oper->oper_name = call->func_name;
        oper->operands = call->params;
        //Remove the "@"
        oper->oper_name.erase(oper->oper_name.begin());
        return oper;
    }
    if (call->func_name == "@frac") {
        LiteralNode* lit = new LiteralNode();
        LiteralNode* old = (LiteralNode*) call->params.front().get();
        if (old->type != LITERAL) return nullptr;
        lit->value = old->value;
        return lit;
    }
    if (call->func_name == "@fixed16") {
        LiteralNode* lit = new LiteralNode();
        LiteralNode* whole = (LiteralNode*) call->params.front().get();
        LiteralNode* frac = (LiteralNode*) call->params.back().get();
        if (whole->type != LITERAL || frac->type != LITERAL) return nullptr;
        lit->value = whole->value  &0xFFFF0000;
        lit->value |= (frac->value >> 16) & 0xFFFF;
        return lit;
    } 
     return nullptr;
}

ASTNode* parse_call(std::vector<token> &tokens, tokIter &it) {
    START_PARSE(CallNode);
    READ_TEXT("call");
    READ_OF_TYPE(VAR);
    node->func_name = it->text;
    READ_OF_TYPE(OPENBRACE);
    while(!check_next(tokens,it,CLOSEBRACE)) {
        ASTNode* expr = parse_expression(tokens,it);
        if (!expr) PARSE_ERR("call had a bad argument?");
        node->params.push_back(ASTsubtree(expr));
    }
    READ_OF_TYPE(CLOSEBRACE);
    ASTNode* builtin = handle_builtins(node);
    if (builtin) {
        delete node;
        node = (CallNode*) builtin; //Not acutally type valid, but we are returning it as an ancestor immediately.
    } 
    SUCCEED_PARSE;
}
ASTNode* parse_array_addr(std::vector<token> &tokens, tokIter &it) {
  START_PARSE(OperNode);
  node->oper_name = "add";
  READ_TEXT("getelementptr");
  while (!check_next_text(tokens,it,"*")) { it++; if (it == tokens.end()) FAIL_PARSE;};
  it++;
  ASTNode* expr = parse_expression(tokens,it);
  if (!expr) PARSE_ERR("array addr not recognized?");
  node->operands.push_back(ASTsubtree(expr));
  it++;
  expr = parse_expression(tokens,it);
  if (!expr) PARSE_ERR("array offset not recognized?");
  node->operands.push_back(ASTsubtree(expr));
  SUCCEED_PARSE;
}
        
ASTNode* parse_expression(std::vector<token> &tokens, tokIter &it) {
   if (PARSE_DEBUG)std::cerr<<prefix<<"Looking for an expr..." << std::endl;
  if (check_next(tokens,it,ENDLINE)) return nullptr;
   ASTNode* expr;
  //check for allocate
  if (check_next_text(tokens,it,"allocate")) {
    return parse_allocate(tokens,it);
  }
  //check for load
  if (check_next_text(tokens,it,"load")) {
    return parse_load(tokens,it);
  }
  //check for phi
  if (check_next_text(tokens,it,"phi")) {
    return parse_phi(tokens,it);
  }
  //check for call
  if (check_next_text(tokens,it,"call")) {
    return parse_call(tokens,it);
  }
  //check for a single var
  if (check_next(tokens,it,VAR)) {
    return parse_read(tokens,it);
  }
  //check for an array access
  if (check_next_text(tokens,it,"getelementptr")) {
      return parse_array_addr(tokens,it);
  }
  //try for a operation;
  expr = parse_operation(tokens,it);
  if (expr) return expr;
  //try for a constant
  return parse_literal(tokens,it);
}

ASTNode* parse_return(std::vector<token> &tokens, tokIter &it) {
    START_PARSE(ReturnNode);
    
    READ_TEXT("ret");
    ASTNode* expr = parse_expression(tokens,it);
    if (!expr) PARSE_ERR("return had no value?");
    node->value = ASTsubtree(expr);
    READ_OF_TYPE(ENDLINE);
    SUCCEED_PARSE; 
    
}

ASTNode* parse_assignment(std::vector<token> &tokens, tokIter &it) {
    START_PARSE(AssignmentNode);
    READ_OF_TYPE(VAR);
    node->leftside = ASTsubtree(new WriteNode(it->text));
    READ_OF_TYPE(EQUALS);
    ASTNode* expr = parse_expression(tokens,it);
    if (!expr) FAIL_PARSE;
    node->rightside = ASTsubtree(expr);
    READ_OF_TYPE(ENDLINE);
    SUCCEED_PARSE;
}

ASTNode* parse_store(std::vector<token> &tokens, tokIter &it) {
    START_PARSE(StoreNode);
    
    READ_TEXT("store");
    ASTNode* expr = parse_expression(tokens,it);
    if (!expr) PARSE_ERR("store had no value?");
    node->value = ASTsubtree(expr);
    READ_TEXT("at");
    expr = parse_expression(tokens,it);
    if (!expr) PARSE_ERR("store had no address?");
    node->address = ASTsubtree(expr);
    READ_OF_TYPE(ENDLINE);
    SUCCEED_PARSE;
}

ASTNode* parse_cond_branch(std::vector<token> &tokens, tokIter &it) {
  START_PARSE(BranchNode);
  READ_TEXT("br");
  ASTNode* expr = parse_expression(tokens,it);
  if (!expr) PARSE_ERR("branch had no condition?");
  node->condition = ASTsubtree(expr);
  READ_TEXT("label");
  READ_OF_TYPE(VAR);
  node->truelabel = current_function+it->text;
  READ_TEXT("label");
  READ_OF_TYPE(VAR);
  node->falselabel = current_function+it->text;
  READ_OF_TYPE(ENDLINE);
  current_block->succs.push_back(node->truelabel);
  current_block->succs.push_back(node->falselabel);
  SUCCEED_PARSE;
}
ASTNode* parse_jump(std::vector<token> &tokens, tokIter &it) {
  START_PARSE(JumpNode);
  READ_TEXT("br");
  READ_TEXT("label");
  READ_OF_TYPE(VAR);
  node->label = current_function+it->text;
  READ_OF_TYPE(ENDLINE);
  current_block->succs.push_back(node->label);
  SUCCEED_PARSE;
}

ASTNode* parse_branch(std::vector<token> &tokens, tokIter &it) {
  if (!check_next_text(tokens,it,"br")) return nullptr;
  auto it2 = it+1;
  if (check_next_text(tokens,it2,"label")) {
    return parse_jump(tokens,it);
  } else {
    return parse_cond_branch(tokens,it);
  }

}


ASTNode* parse_line(std::vector<token> &tokens, tokIter &it) {
   if (it == tokens.end()) return nullptr;
   it++;
   if (it == tokens.end()) {it--;return nullptr;}
   if (it->type == ENDLINE) { return new NOOPNode();};
   it--;
   if (check_next(tokens,it,VAR)) return parse_assignment(tokens,it);
   if (check_next_text(tokens,it,"store")) return parse_store(tokens,it);
   if (check_next_text(tokens,it,"ret")) return parse_return(tokens,it);
   if (check_next_text(tokens,it,"br")) return parse_branch(tokens,it);
   
   return nullptr;
}

void parse_basic_block_header(std::vector<token> &tokens, tokIter &it, BasicBlockNode* bb) {
  auto old_it = it;
  it++;
  bb->label = current_function + it->text;
  if (check_next_text(tokens,it,"preds")) it++;
  if (check_next(tokens,it,EQUALS)) it++;
  while (check_next(tokens,it,VAR)) {
    it++;
    bb->preds.push_back(current_function+it->text);
  }
  return;
}

ASTNode* parse_block(std::vector<token> &tokens, tokIter &it) {
    START_PARSE(BlockNode);
    READ_OF_TYPE(OPENBRACE);
    ASTNode* line;
    line = parse_line(tokens,it);
    int num = 0;
    BasicBlockNode* basic_block = new BasicBlockNode();
    basic_block->label = current_function + "%0";
    current_block = basic_block;
    while (line) {
      if (PARSE_DEBUG) std::cerr << prefix << "BLOCK line " << ++num << std::endl;
      basic_block->lines.push_back(ASTsubtree(line));
      if (check_next(tokens,it,LABEL))  {
        node->children.push_back(ASTsubtree(basic_block));
        basic_block = new BasicBlockNode();
        parse_basic_block_header(tokens,it,basic_block);
        current_block = basic_block;
      } 
      line = parse_line(tokens,it);
    }
    node->children.push_back(ASTsubtree(basic_block));
    READ_OF_TYPE(CLOSEBRACE);
    SUCCEED_PARSE;
      
}

ASTNode* parse_func_def(std::vector<token> &tokens, tokIter &it) {
    START_PARSE(FuncDefNode);
    
    READ_TEXT("define");
    READ_OF_TYPE(VAR);
    node->name = it->text;
    current_function = it->text;
    
    READ_OF_TYPE(OPENBRACE);
    while (check_next(tokens,it,VAR)) {
        it++;
        node->params.push_back(it->text);
    }
    READ_OF_TYPE(CLOSEBRACE);
    
    if (!skipTo(tokens,it,OPENBRACE)) FAIL_PARSE;
    
    it--;
    ASTNode* body = parse_block(tokens,it);
    if (!body) FAIL_PARSE;
    node->body = ASTsubtree(body);
    SUCCEED_PARSE;
}
std::string getFuncName(ASTNode* node) {
    if (node->type != FUNCTION_DEFINITION) return "";
    FuncDefNode* def = (FuncDefNode*) node;
    return def->name;
}
void parse_data(std::vector<token> &tokens, tokIter &it, RootNode* root) {
  if (it->text != "@DATA") return;
  while ( it != tokens.end() && it->type != CLOSEBRACE) it++;
  if (it == tokens.end()) return;
  if (!check_next(tokens,it,OPENBRACE)) return;
  it++;
  while (check_next(tokens,it,IDENT)) {
    it++;
    try {
      unsigned long int val = std::stol(it->text);
      root->data_segment.push_back(val);
    } catch(...) {};
  }
  return;
}
    

ASTNode* parse(std::vector<token> &tokens) {
  tokens.insert(tokens.begin(),{ENDLINE,"DUMMY"});
  auto it = tokens.begin();
  RootNode* root = new RootNode();
  while (it != tokens.end()) {
    while ( it != tokens.end() && !it->text.empty() && (it->text != "define" && it->type != VAR)) it++;
    if (it->type == VAR && it->text == "@DATA") {parse_data(tokens,it,root); it++; continue;}
    ASTNode* func = parse_func_def(tokens,--it);
    if (func) {
        std::string name = getFuncName(func);
        if (name == "@ibniz_run") {
            root->video_tyx = func;
        } 
        root->subroutines.push_back(ASTsubtree(func));
    } else {
     it++;
     if (it == tokens.end()) break;
     it++;
     if (it == tokens.end()) break;

    }
  }
  
  return root;
}
