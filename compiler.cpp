#include "parser.h"
#include "walk_ast.h"

void *fx(ASTsubtree &ast, ASTsubtree &root, void *parm) {
  if(ast.get()->type==ROOT) return NULL;
  return (void *)(ast.get());
}

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
    if (!ast) std::cerr << "PARSE FAILED" << std::endl;

    if (ast) std::cerr << "PARSE SUCCEEDED?" << std::endl;

    // std::cerr << ast->to_string();
    ASTsubtree ast2 = ASTsubtree(ast);
    std::cerr << ast2.get()->to_string();
    std::cerr << "AFTER TRANSFORMATION:" << std::endl;
    structure_ast(ast2); // experimental...
    std::cerr << ast2.get()->to_string();
    // structure_ast(ast2);
    //ASTNode *out = get_basic_block_by_label(ast2.get(), "ibniz_run%0");

    // ASTNode *out = (ASTNode *)(walk_ast(ast2, ast2, fx, NULL));
    //BasicBlockNode *out = get_basic_block_by_label(ast2, "@ibniz_run%19");
    // void *out = walk_ast(ast2, ast2, fx, NULL);
    //if(out != NULL) std::cerr << "FOUND NODE!" << std::endl << out->to_string();
    //else std::cerr << "DIDN'T FIND NODE!" << std::endl;

    // if(ast) delete ast;
    
    // unsure how to free the smart pointer...
    // if (ast) delete ast;
    // if (ast2) delete ast2;
}
