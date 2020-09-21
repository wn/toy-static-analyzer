#include "TNode.h"

#include "Logger.h"

#include <set>
#include <sstream>

namespace backend {
std::string getTNodeTypeString(TNodeType t) {
    switch (t) {
    case TNodeType::Program:
        return "Program";
    case TNodeType::Procedure:
        return "Procedure";
    case TNodeType::StatementList:
        return "StatementList";
    case TNodeType::Assign:
        return "Assign";
    case TNodeType::Call:
        return "Call";
    case TNodeType::IfElse:
        return "IfElse";
    case TNodeType::Print:
        return "Print";
    case TNodeType::Read:
        return "Read";
    case TNodeType::While:
        return "While";
    case TNodeType::Not:
        return "Not";
    case TNodeType::And:
        return "And";
    case TNodeType::Or:
        return "Or";
    case TNodeType::Greater:
        return "Greater";
    case TNodeType::GreaterThanOrEqual:
        return "GreaterThanOrEqual";
    case TNodeType::Lesser:
        return "Lesser";
    case TNodeType::LesserThanOrEqual:
        return "LesserThanOrEqual";
    case TNodeType::Equal:
        return "Equal";
    case TNodeType::NotEqual:
        return "NotEqual";
    case TNodeType::Plus:
        return "Plus";
    case TNodeType::Minus:
        return "Minus";
    case TNodeType::Multiply:
        return "Multiply";
    case TNodeType::Divide:
        return "Divide";
    case TNodeType::Modulo:
        return "Modulo";
    case TNodeType::Variable:
        return "Variable";
    case TNodeType::Constant:
        return "Constant";
    case TNodeType::INVALID:
        return "INVALID";
    default:
        // if TNodeType indeed cover all possible cases, throw statement should not occur.
        throw "Error: Unknown TNodeType is not implemented yet.";
    };
}

std::string getOperatorStringFromTNodeType(TNodeType t) {
    switch (t) {
    case TNodeType::Not:
        return "!";
    case TNodeType::And:
        return "&&";
    case TNodeType::Or:
        return "||";
    case TNodeType::Greater:
        return ">";
    case TNodeType::GreaterThanOrEqual:
        return ">=";
    case TNodeType::Lesser:
        return "<";
    case TNodeType::LesserThanOrEqual:
        return "<=";
    case TNodeType::Equal:
        return "==";
    case TNodeType::NotEqual:
        return "!=";
    case TNodeType::Plus:
        return "+";
    case TNodeType::Minus:
        return "-";
    case TNodeType::Multiply:
        return "*";
    case TNodeType::Divide:
        return "/";
    case TNodeType::Modulo:
        return "%";
    default:
        throw "Error: getOperatorTNodeTypeString only takes in an operator type";
    };
}

std::string TNode::toString() const {
    return toStringHelper(0);
}

std::string TNode::toShortString() const {
    std::string typeString = getTNodeTypeString(type);
    std::ostringstream stringStream;
    stringStream << typeString << " " << name;
    stringStream << " @ " << line;
    return stringStream.str();
}

std::string TNode::toStringHelper(int tabs) const {
    std::string nodeName = getTNodeTypeString(type);
    std::ostringstream stringStream;

    for (int i = 0; i < tabs; ++i)
        stringStream << " ";
    stringStream << nodeName;
    stringStream << " @ " << line << "(" << name << ", " << constant << ")";
    stringStream << " : [\n";

    for (const auto& child : children) {
        stringStream << child.toStringHelper(tabs + 1);
    }

    for (int i = 0; i < tabs; ++i)
        stringStream << " ";
    stringStream << "]\n";
    return stringStream.str();
}

bool TNode::operator==(const TNode& rhs) const {
    if (type != rhs.type || line != rhs.line || name != rhs.name || constant != rhs.constant) {
        // Helpful to resolve test.
        logLine("type not equal: " + getTNodeTypeString(type) + " : " + getTNodeTypeString(rhs.type),
                type != rhs.type);
        logLine("type line not equal: " + std::to_string(line) + " : " + std::to_string(rhs.line),
                line != rhs.line);
        logLine("name not equal: " + name + " : " + rhs.name, name != rhs.name);
        logLine("constant not equal: " + constant + " : " + rhs.constant, constant != rhs.constant);
        logLine();
        return false;
    }
    if (children.size() != rhs.children.size()) {
        return false;
    }
    for (int i = 0; i < children.size(); ++i) {
        if (children[i] == rhs.children[i]) {
            continue;
        }
        return false;
    }
    return true;
}

void TNode::addChild(const TNode& c) {
    children.emplace_back(c);
}

bool TNode::isStatementNode() const {
    std::set<TNodeType> statementTypes{
        TNodeType::Assign, TNodeType::Call, TNodeType::IfElse,
        TNodeType::Print,  TNodeType::Read, TNodeType::While,
    };
    return statementTypes.count(type);
}

int TNode::uniqueIdentifier = 0;
int TNode::getNewUniqueIdentifier() {
    TNode::uniqueIdentifier += 1;
    return TNode::uniqueIdentifier;
}
std::ostream& operator<<(std::ostream& os, const TNode& t) {
    os << t.toString();
    return os;
}
} // namespace backend
