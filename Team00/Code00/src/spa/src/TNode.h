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

std::string getTNodeTypeString(TNodeType t);


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

    void addChild(const TNode& c);
    std::string toString() const;
    bool operator==(const TNode& s) const;

  private:
    std::string toStringHelper(int tabs) const;
};
} // namespace backend
