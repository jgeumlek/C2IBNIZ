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
enum ASTtype { FUNCTION_DEFINITION, BLOCK, ASSIGNMENT, CALL, OPERATION, READ,WRITE, STORE, LOAD, LITERAL,LOOP, IF_THEN_ELSE, RETURN, NOOP, ALLOCATE, UNKNOWN };
class ASTNode;
typedef std::unique_ptr<ASTNode> ASTsubtree;
ASTNode* parse(std::vector<token> &tokens);

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

class CallNode : public ASTNode {
public:
    CallNode() : ASTNode(CALL) {};
    std::string func_name;
    std::vector<ASTsubtree> params;
};

class OperNode : public ASTNode {
public:
    OperNode() : ASTNode(OPERATION) {};
    std::string oper_name;
    std::vector<ASTsubtree> operands;
};

class ReturnNode : public ASTNode {
public:
    ReturnNode() : ASTNode(RETURN) {};
    ASTsubtree value;
};

class ReadNode : public ASTNode {
public:
    ReadNode() : ASTNode(READ) {};
    ReadNode(std::string varname) : ASTNode(READ), varname(varname) {};
    std::string varname;
};
class WriteNode : public ASTNode {
public:
    WriteNode() : ASTNode(WRITE) {};
    WriteNode(std::string varname) : ASTNode(WRITE), varname(varname) {};
    std::string varname;
};
class LiteralNode : public ASTNode {
public:
    LiteralNode() : ASTNode(LITERAL) {};
    LiteralNode(int value) : ASTNode(LITERAL), value(value) {};
    int value;
};
class AllocateNode : public ASTNode {
public:
    AllocateNode() : ASTNode(ALLOCATE) {};
};
class NOOPNode : public ASTNode {
public:
    NOOPNode() : ASTNode(NOOP) {};
};

