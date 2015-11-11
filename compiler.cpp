#include "parser.h"


int main(int argc, char** argv) {
    FuncDefNode fnode;
    std::vector<token> tokens;
    tokenize(std::cin,tokens);
    for (auto token : tokens) {
        if (token.type == IDENT) std::cout << "str_" << token.text;
        if (token.type == OPENBRACE) std::cout << "OPEN";
        if (token.type == CLOSEBRACE) std::cout << "CLOSE";
        if (token.type == VAR) std::cout << "v_" << token.text;
        if (token.type == EQUALS) std::cout << ":=";
        if (token.type == ENDLINE) {std::cout << ";\n"; continue;}
        std::cout << " ";
    }
    std::cout << std::endl;
}