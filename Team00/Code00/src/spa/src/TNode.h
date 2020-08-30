#include <map>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

namespace backend {
enum TNodeType {
    Program,
    Procedure,
    StatementList,

    // Stmts
    Assign,
    Call,
    IfElse,
    Print,
    Read,
    While,

    // Expressions
    NotEq,
    And,
    Or,
    Greater,
    GreaterThanOrEqual,
    Lesser,
    LesserThanOrEqual,
    Equal,
    NotEqual,

    Plus,
    Minus,
    Multiply,
    Divide,
    Modulo,

    // Leaves
    Variable,
    Constant,

    // Used for failure (I hate null)
    INVALID,
};


class TNode {
  public:
    // Some TNode don't need line number.
    TNode(TNodeType type, std::vector<TNode> children, int line = -1)
    : type(type), children(std::move(children)), line(line) {
    }
    explicit TNode(TNodeType type, int line = -1)
    : type(type), children(std::vector<TNode>()), line(line) {
    }
    TNodeType type;
    std::vector<TNode> children;
    int line;

    // For variables and procedures
    std::string name;

    static std::string toString(TNode const& tNode, int tabs);
    static std::string getName(TNodeType t);
};
} // namespace backend
