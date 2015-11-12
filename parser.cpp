#include "parser.h"

/*helpful macros. Note that START_PARSE can't be wrapped, since we want to declare in scope.*/
#define START_PARSE(X) auto old_it = it; X *node = new X(); std::cout<<prefix<<"Starting parse " << ASTtypenames[node->type] << std::endl; GO_DOWN;

std::string prefix  = "";
#define GO_DOWN prefix.push_back(' ');
#define GO_UP prefix.erase(prefix.begin());

#define FAIL_PARSE do {it = old_it; delete node; GO_UP;return nullptr;} while(0);

#define SUCCEED_PARSE do {GO_UP;std::cout<<prefix<<"Parse of " << ASTtypenames[node->type] << " suceeded" <<std::endl; return node;} while(0);

#define PARSE_ERR(X) do {std::cout<<"FATAL:"<<X<<std::endl;GO_UP;it = old_it; delete node; return nullptr;} while(0);

#define READ_OF_TYPE(X) do { if (!check_next(tokens,it,X)) {\
if (it!=tokens.end()&&(it+1)!=tokens.end()) std::cout<<prefix<<"Wanted type:"<<tokentypenames[X]<<" Got:"<< tokentypenames[(it+1)->type] <<"("<<(it+1)->text<<")" << std::endl;\
FAIL_PARSE;}; it++;} while(0);

#define READ_TEXT(X) do { \
if (!check_next_text(tokens,it,X))\
{\
if (it!=tokens.end()&&(it+1)!=tokens.end()) std::cout<<prefix<<"Wanted:"<<X<<" Got:"<< (it+1)->text << std::endl;\
FAIL_PARSE;};\
it++;} while(0);


std::string tokentypenames[] = {"IDENT","VAR","CONSTANT","EQUALS","OPENBRACE","ENDLINE"};
std::string ASTtypenames[] = { "FUNCTION_DEFINITION", "BLOCK", "ASSIGNMENT", "CALL", "OPERATION", "READ","WRITE", "STORE", "LOAD", "LITERAL","LOOP", "IF_THEN_ELSE", "RETURN", "NOOP", "ALLOCATE", "UNKNOWN" };

void tokenize(std::istream &in, std::vector<token> &out) {
    std::string text;
    while (!in.fail() && !in.eof()) {
        std::getline(in,text,' ');
        token token;
        token.type = IDENT;
        token.text = text;
        while (!text.empty() && text.front() == '\n') { out.push_back({ENDLINE,"\n"}); text.erase(text.begin());}
        if (text.empty()) continue;
        if (text == "\n" || text == ";\n") token.type = ENDLINE;
        if (text.front() == '%') token.type = VAR;
        if (text == "(" || text == "{") token.type = OPENBRACE;
        if (text == ")" || text == "}") token.type = CLOSEBRACE;
        if (text == "=") token.type = EQUALS;
        out.push_back(token);
    }
}

typedef std::vector<token>::iterator tokIter;

/*PARSER INVARIANTS
 * On a failed parse, the iterator should be unwound.
 * At the start of the parse, the iterator points to the last read token.
 * At the end of the parse, the iterator points to the last read token.
 */

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
          value = std::stoi(text);
          //NOTE: above stoi has some quirks. "123abc" -> 123, even though there is text.
          //POSSIBLY DO SOME BOUNDS CHECKING?
          node->value = value;
          SUCCEED_PARSE;
  } catch (...) {
    //stoi exception when it fails.
    PARSE_ERR(text + " was not any expression?");
  }
  return nullptr;
}

std::string operators[] = {"add","mul","sdiv","srem","sin","sqrt","atan","xor","and","or"};
bool check_oper(std::string oper) {
  for (int i = 0; i < 10; i++) {
    if (operators[i] == oper) return true;
  }
  return false;
}

ASTNode* parse_operation(std::vector<token> &tokens, tokIter &it) {
  START_PARSE(OperNode);
  READ_OF_TYPE(IDENT);
  node->oper_name = it->text;
  if (!check_oper(it->text)) FAIL_PARSE;
  if (check_next_text(tokens,it,"nsw")) it++;
  ASTNode* expr = parse_expression(tokens,it);
  while (expr) {
    node->operands.push_back(ASTsubtree(expr));
    expr = parse_expression(tokens,it);
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

ASTNode* parse_expression(std::vector<token> &tokens, tokIter &it) {
   std::cout<<prefix<<"Looking for an expr..." << std::endl;
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
  //check for a single var
  if (check_next(tokens,it,VAR)) {
    return parse_read(tokens,it);
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


ASTNode* parse_line(std::vector<token> &tokens, tokIter &it) {
   if (it == tokens.end()) return nullptr;
   it++;
   if (it == tokens.end()) {it--;return nullptr;}
   if (it->type == ENDLINE) { return new NOOPNode();};
   it--;
   if (check_next(tokens,it,VAR)) return parse_assignment(tokens,it);
   if (check_next_text(tokens,it,"store")) return parse_store(tokens,it);
   if (check_next_text(tokens,it,"ret")) return parse_return(tokens,it);
   
   return nullptr;
}

ASTNode* parse_block(std::vector<token> &tokens, tokIter &it) {
    START_PARSE(BlockNode);
    READ_OF_TYPE(OPENBRACE);
    ASTNode* line;
    line = parse_line(tokens,it);
    int num = 0;
    while (line) {
      std::cout << prefix << "BLOCK line " << ++num << std::endl;
      node->children.push_back(ASTsubtree(line));
      line = parse_line(tokens,it);
    }
    READ_OF_TYPE(CLOSEBRACE);
    SUCCEED_PARSE;
      
}

ASTNode* parse_func_def(std::vector<token> &tokens, tokIter &it) {
    START_PARSE(FuncDefNode);
    
    READ_TEXT("define");
    READ_OF_TYPE(IDENT);
    node->name = it->text;
    
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

ASTNode* parse(std::vector<token> &tokens) {
  auto it = tokens.begin();
  tokens.insert(tokens.begin(),{ENDLINE,"DUMMY"});
  return parse_func_def(tokens,it);
}