#pragma once

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
    Not,
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
    // Required for accessing TNode keys in a hashmap
    TNode() = default;
    // Some TNode don't need line number.
    explicit TNode(TNodeType type, int line = 0) : type(type), line(line) {
    }

    TNodeType type{ TNodeType::INVALID };
    std::vector<TNode> children;
    int hashInteger{ TNode::getNewUniqueIdentifier() };
    int line;

    // For variables and procedures
    std::string name;
    // We set constant as -1 by default as we are guaranteed by SIMPLE
    // that constant >= 0.
    int constant{ -1 };

    void addChild(const TNode& c);
    std::string toString() const;
    std::string toShortString() const;
    bool operator==(const TNode& s) const;


    bool isStatementNode() const;

    friend std::ostream& operator<<(std::ostream& os, const backend::TNode& t);

  private:
    // Used to generate a Unique ID for a new AST. Used for hashing.
    static int uniqueIdentifier;
    static int getNewUniqueIdentifier();

    std::string toStringHelper(int tabs) const;
};
} // namespace backend

namespace std {
template <> struct std::hash<backend::TNode> {
    std::size_t operator()(backend::TNode const& tNode) const noexcept {
        return std::hash<long long>{}(tNode.hashInteger);
    }
};
} // namespace std
