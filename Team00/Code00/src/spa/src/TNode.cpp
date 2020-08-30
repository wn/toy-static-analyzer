#include "TNode.h"

namespace backend {

std::string TNode::getName(TNodeType t) {
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
    case TNodeType::NotEq:
        return "NotEq";
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

std::string TNode::toString(TNode const& tNode, int tabs) {
    std::string name = getName(tNode.type);
    std::ostringstream stringStream;

    for (int i = 0; i < tabs; ++i)
        stringStream << " ";
    stringStream << name << " " << tNode.name;
    if (tNode.line >= 0) {
        stringStream << " @ " << tNode.line;
    }
    stringStream << " : [\n";

    for (const auto& child : tNode.children) {
        stringStream << toString(child, tabs + 1);
    }

    for (int i = 0; i < tabs; ++i)
        stringStream << " ";
    stringStream << "]\n";
    return stringStream.str();
}
} // namespace backend
