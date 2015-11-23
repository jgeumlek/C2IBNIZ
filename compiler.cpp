#include "parser.h"


int main(int argc, char** argv) {
    std::vector<token> tokens;
    tokenize(std::cin,tokens);
    for (auto token : tokens) {
        if (token.type == IDENT) std::cerr << "str_" << token.text;
        if (token.type == OPENBRACE) std::cerr << "OPEN";
        if (token.type == CLOSEBRACE) std::cerr << "CLOSE";
        if (token.type == VAR) std::cerr << "v_" << token.text;
        if (token.type == EQUALS) std::cerr << ":=";
        if (token.type == ENDLINE) {std::cerr << ";\n"; continue;}
        if (token.type == LABEL) {std::cerr << "LABEL" << token.text;}
        std::cerr << " ";
    }
    std::cerr << std::endl;
    
    ASTNode* ast = parse(tokens);
    if (ast->type != ROOT) exit (-5);
    RootNode* root = (RootNode*) ast;
    if (root->subroutines.empty()) std::cerr << "PARSE FAILED" << std::endl;
    
    if (!root->subroutines.empty()) std::cerr << "PARSE SUCCEEDED?" << std::endl;
    
    std::cerr << ast->to_string();
    
    if (ast) delete ast;
}
