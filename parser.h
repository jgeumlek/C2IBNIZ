#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <sstream>

#define MAIN_FUNC "@ibniz_run"

enum tokentype { IDENT, VAR, CONSTANT, EQUALS, OPENBRACE, CLOSEBRACE, ENDLINE};

struct token {
    enum tokentype type;
    std::string text;
};

void tokenize(std::istream &in, std::vector<token> &out);

//We might not end up supporting a lot of these...
enum ASTtype { FUNCTION_DEFINITION, BLOCK, ASSIGNMENT, CALL, OPERATION, READ, STORE, LOAD, LOOP, IF_THEN_ELSE, RETURN, NOOP, UNKNOWN };
class ASTNode;
typedef std::unique_ptr<ASTNode> ASTsubtree;

class ASTNode {
public:
    enum ASTtype type = UNKNOWN;
    ASTNode() {};
    ASTNode(enum ASTtype type) : type(type) {};
    virtual ~ASTNode() {};
};

class FuncDefNode : public ASTNode {
public:
    FuncDefNode() : ASTNode(FUNCTION_DEFINITION) {};
    std::string name;
    std::vector<std::string> params;
    ASTsubtree body;
};
class BlockNode : public ASTNode {
public:
    BlockNode() : ASTNode(BLOCK) {};
    std::vector<ASTsubtree> children;
};
class AssignmentNode : public ASTNode {
public:
    AssignmentNode() : ASTNode(ASSIGNMENT) {};
    ASTsubtree leftside;
    ASTsubtree rightside;
};
class StoreNode : public ASTNode {
public:
    StoreNode() : ASTNode(STORE) {};
    ASTsubtree value;
    ASTsubtree address;
};
class LoadNode : public ASTNode {
public:
    LoadNode() : ASTNode(LOAD) {};
    ASTsubtree address;
};
class ReturnNode : public ASTNode {
public:
    ReturnNode() : ASTNode(RETURN) {};
    ASTsubtree address;
};

class ReadNode : public ASTNode {
public:
    ReadNode() : ASTNode(READ) {};
    std::string varname;
};

class NOOPNode : public ASTNode {
public:
    NOOPNode() : ASTNode(NOOP) {};
};

