#ifndef C2I_PARSER_H
#define C2I_PARSER_H
#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <sstream>

#define MAIN_FUNC "@ibniz_run"

enum tokentype { IDENT, VAR, CONSTANT, EQUALS, OPENBRACE, CLOSEBRACE, ENDLINE,LABEL};
struct token {
    enum tokentype type;
    std::string text;
};

void tokenize(std::istream &in, std::vector<token> &out);

//We might not end up supporting a lot of these...
enum ASTtype { ROOT,FUNCTION_DEFINITION, BLOCK, ASSIGNMENT, CALL, OPERATION, READ,WRITE, STORE, LOAD, LITERAL,LOOP, RETURN, NOOP, ALLOCATE, UNKNOWN,BASICBLOCK,PHI,JUMP,BRANCH,IFTHEN,IFTHENELSE};
class ASTNode;
typedef std::shared_ptr<ASTNode> ASTsubtree;
ASTNode* parse(std::vector<token> &tokens);

class ASTNode {
public:
    enum ASTtype type = UNKNOWN;
    ASTNode() {};
    ASTNode(enum ASTtype type) : type(type) {};
    virtual ~ASTNode() {};
    virtual std::string to_string() { return "UNKNOWN NODE";};
};

class RootNode : public ASTNode {
public:
    RootNode() : ASTNode(ROOT) {};
    ASTsubtree video_tyx;
    ASTsubtree video_t;
    ASTsubtree audio;
    std::vector<ASTsubtree> subroutines;
    virtual std::string to_string() {
      std::string text;
      text = "\n====Our Main Func====\n";
      text += video_tyx->to_string();
      text += "\n=====End Main Func====\n";
      for (auto func : subroutines) {
        text  += "\n=========================\n";
        text += func->to_string();
        text += "\n=========================\n";
      }
      return text;
    };
};

class FuncDefNode : public ASTNode {
public:
    FuncDefNode() : ASTNode(FUNCTION_DEFINITION) {};
    std::string name;
    std::vector<std::string> params;
    ASTsubtree body;

    virtual std::string to_string() {
      std::string text;
      text  = "function " + name + "(";
      for (auto var: params) {
        text += var;
        text += ",";
      }
      text += ")";
      text += body->to_string();
      return text;
    };
};
class BlockNode : public ASTNode {
public:
    BlockNode() : ASTNode(BLOCK) {};
    std::vector<ASTsubtree> children;
    //The children are BasicBlockNodes.
    virtual std::string to_string() {
      std::string text;
      text  = "\n{\n";
      for (auto bb = children.begin(); bb != children.end(); bb++) {
        text += (*bb)->to_string();
        text += ";\n";
      }
      text += "}\n";
      return text;
    };
};
class BasicBlockNode : public ASTNode {
public:
    BasicBlockNode() : ASTNode(BASICBLOCK) {};
    std::vector<ASTsubtree> lines;
    std::string label;
    std::vector<std::string> preds;
    std::vector<std::string> succs;

    virtual std::string to_string() {
      std::string text;
      text  = "\n---- Basic Block ";
      text += label + " ";
      if (!preds.empty()) {
        text += "in(";
        for (auto pred : preds) {
          text+= pred + ",";
        }
        text += ")";
      }

      if (!succs.empty()) {
        text += "out(";
        for (auto succ : succs) {
          text+= succ + ",";
        }
        text += ")";
      }

      text += "----\n";
      for (auto line = lines.begin(); line != lines.end(); line++) {
        text += (*line)->to_string();
        text += ";\n";
      }
      text += "---- End Block ----\n";
      return text;
    };
};
class BranchNode : public ASTNode {
public:
    BranchNode() : ASTNode(BRANCH) {};
    ASTsubtree condition;
    std::string truelabel;
    std::string falselabel;
    virtual std::string to_string() { return "cond jump on " + condition->to_string() + " to " + truelabel +" or "+ falselabel;};
};

class IfThenNode : public ASTNode {
public:
  IfThenNode() : ASTNode(IFTHEN) {};
  ASTsubtree condition;
  ASTsubtree truebranch; // conditionally executed code
  ASTsubtree merged; // code following if statement
  // TODO - format differently?
  virtual std::string to_string() { return "if " + condition->to_string() + " then " + truebranch->to_string() + " endif\n" + merged->to_string();};
};

class IfThenElseNode : public ASTNode {
public:
  IfThenElseNode() : ASTNode(IFTHENELSE) {};
  ASTsubtree condition;
  ASTsubtree truebranch;
  ASTsubtree falsebranch;
  ASTsubtree merged; // code following if statement
  // TODO - format differently
  virtual std::string to_string() { return "if " + condition->to_string() + " then " + truebranch->to_string() + " else " + falsebranch->to_string() + " endif\n" + merged->to_string();}
};

class JumpNode : public ASTNode {
public:
    JumpNode() : ASTNode(JUMP) {};
    std::string label;
    virtual std::string to_string() { return "jump " + label;};
};
class PhiNode : public ASTNode {
public:
    PhiNode() : ASTNode(PHI) {};
    std::vector<ASTsubtree> values;
    std::vector<std::string> source_blocks;
    virtual std::string to_string() { std::string text = "phi ";
      for (int i = 0; i < values.size(); i++) {
        text += "(";
        text += (values.at(i))->to_string();
        text += " from ";
        text += source_blocks.at(i);
        text += ")";
      }
      return text;
    };
};


class AssignmentNode : public ASTNode {
public:
    AssignmentNode() : ASTNode(ASSIGNMENT) {};
    ASTsubtree leftside;
    ASTsubtree rightside;
    virtual std::string to_string() { return leftside->to_string() + " := " + rightside->to_string();};
};
class StoreNode : public ASTNode {
public:
    StoreNode() : ASTNode(STORE) {};
    ASTsubtree value;
    ASTsubtree address;
    virtual std::string to_string() { return "store " + value->to_string() + " at addr " + address->to_string();};
};
class LoadNode : public ASTNode {
public:
    LoadNode() : ASTNode(LOAD) {};
    ASTsubtree address;
    virtual std::string to_string() { return "load from addr " + address->to_string();};
};

class CallNode : public ASTNode {
public:
    CallNode() : ASTNode(CALL) {};
    std::string func_name;
    std::vector<ASTsubtree> params;
    virtual std::string to_string() {
      std::string text;
      text  = "call " + func_name + "(";
      for (auto var : params) {
        text += var->to_string();
        text += ",";
      }
      text += ")";
      return text;

    };

};

class OperNode : public ASTNode {
public:
    OperNode() : ASTNode(OPERATION) {};
    std::string oper_name;
    std::vector<ASTsubtree> operands;
    virtual std::string to_string() {
      std::string text;
      text  = "oper_" + oper_name + " ";
      for (auto var = operands.begin(); var != operands.end(); var++) {
        text += (*var)->to_string();
        text += ",";
      }
      return text;

    };
};

class ReturnNode : public ASTNode {
public:
    ReturnNode() : ASTNode(RETURN) {};
    ASTsubtree value;
    virtual std::string to_string() { return "return " + value->to_string();};
};

class ReadNode : public ASTNode {
public:
    ReadNode() : ASTNode(READ) {};
    ReadNode(std::string varname) : ASTNode(READ), varname(varname) {};
    std::string varname;
    virtual std::string to_string() { return "readvar_" + varname;};
};
class WriteNode : public ASTNode {
public:
    WriteNode() : ASTNode(WRITE) {};
    WriteNode(std::string varname) : ASTNode(WRITE), varname(varname) {};
    std::string varname;
    virtual std::string to_string() { return "writevar_" + varname;};
};

class LiteralNode : public ASTNode {
public:
    LiteralNode() : ASTNode(LITERAL) {};
    LiteralNode(int value) : ASTNode(LITERAL), value(value) {};
    static std::string format(int value) {
        if (value == 0) return "0";
        int whole = (value >> 16) & 0x0000FFFF;
        int frac = (value & 0x0000FFFF);
        size_t size = snprintf(nullptr,0,"%.4X.%.4X",whole,frac) + 1;
        std::unique_ptr<char[]> buffer(new char[size]);
        snprintf(buffer.get(),size,"%.4X.%.4X",whole,frac);
        char* start = buffer.get();
        char* end = buffer.get()+size-2;
        while(*start == '0') start++;
        while(*end == '0') end--;
        if (*end != '.') end++;
        return std::string(start,end);
    }
    std::string hexprint() { return format(value);};
    int value;
    virtual std::string to_string() { return "lit_" + format(value);};
};
class AllocateNode : public ASTNode {
public:
    AllocateNode() : ASTNode(ALLOCATE) {};
    virtual std::string to_string() { return "allocate stack space? this is a pointer...";};

};
class NOOPNode : public ASTNode {
public:
    NOOPNode() : ASTNode(NOOP) {};
     virtual std::string to_string() { return "NO OP";};
};

#endif
