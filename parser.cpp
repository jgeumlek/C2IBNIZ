#include "parser.h"

/*helpful macros. Note that START_PARSE can't be wrapped, since we want to declare in scope.*/
#define START_PARSE(X) auto old_it = it; X *node = new X();
#define FAIL_PARSE do {it = old_it; delete node; return nullptr;} while(0);
#define READ_OF_TYPE(X) do { if (!check_next(tokens,it,X)) {FAIL_PARSE;}; it++;} while(0);
#define READ_TEXT(X) do { if (!check_next_text(tokens,it,X)) {FAIL_PARSE;}; it++;} while(0);

void tokenize(std::istream &in, std::vector<token> &out) {
    std::string text;
    while (!in.fail() && !in.eof()) {
        std::getline(in,text,' ');
        token token;
        token.type = IDENT;
        token.text = text;
        while (text.size() >= 2 && text.front() == '\n') { text.erase(text.begin());};
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

ASTNode* parse_return(std::vector<token> &tokens, tokIter &it) {
    START_PARSE(ReturnNode);
    FAIL_PARSE;
}

ASTNode* parse_assignment(std::vector<token> &tokens, tokIter &it) {
    START_PARSE(AssignmentNode);
    FAIL_PARSE;
}

ASTNode* parse_store(std::vector<token> &tokens, tokIter &it) {
    START_PARSE(StoreNode);
    FAIL_PARSE;
}


ASTNode* parse_line(std::vector<token> &tokens, tokIter &it) {
   if (it == tokens.end()) return nullptr;
   if (it->type == ENDLINE) return new NOOPNode();
   if (it->type == VAR) return parse_assignment(tokens,it);
   if (it->text == "store") return parse_store(tokens,it);
   if (it->text == "return") return parse_return(tokens,it);
   return nullptr;
}

ASTNode* parse_block(std::vector<token> &tokens, tokIter &it) {
    START_PARSE(BlockNode);
    READ_OF_TYPE(OPENBRACE);
    ASTNode* line;
    line = parse_line(tokens,it);
    FAIL_PARSE;
}

ASTNode* parse_func_def(std::vector<token> &tokens, tokIter &it) {
    START_PARSE(FuncDefNode);
    
    if (!check_next_text(tokens,it,"define")) FAIL_PARSE;
    it++;
    READ_OF_TYPE(IDENT);
    node->name = it->text;
    
    READ_OF_TYPE(OPENBRACE);
    while (check_next(tokens,it,VAR)) {
        it++;
        node->params.push_back(it->text);
    }
    READ_OF_TYPE(CLOSEBRACE);
    
    if (!skipTo(tokens,it,OPENBRACE)) FAIL_PARSE;
    
    
    ASTNode* body = parse_block(tokens,it);
    if (!body) FAIL_PARSE;
    node->body = ASTsubtree(body);
    return node;
}

ASTNode* parse(std::vector<token> &tokens) {
}