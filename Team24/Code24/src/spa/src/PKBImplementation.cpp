#include "PKBImplementation.h"

#include "DesignExtractor.h"
#include "Logger.h"

namespace backend {

PKBImplementation::PKBImplementation(const TNode& ast) {
    logWord("PKB starting with ast");
    logLine(ast.toString());
    auto tNodeToStatementNumber = extractor::getTNodeToStatementNumber(ast);
    auto statementNumberToTNode = extractor::getStatementNumberToTNode(tNodeToStatementNumber);
}

STATEMENT_NUMBER_LIST PKBImplementation::getAllStatements() {
    return STATEMENT_NUMBER_LIST();
}

VARIABLE_LIST PKBImplementation::getAllVariables() {
    return VARIABLE_LIST();
}

PROCEDURE_LIST PKBImplementation::getAllProcedures() {
    return PROCEDURE_LIST();
}

STATEMENT_NUMBER_LIST PKBImplementation::getStatementsFollowedBy(STATEMENT_NUMBER s) {
    return STATEMENT_NUMBER_LIST();
}

STATEMENT_NUMBER_LIST PKBImplementation::getStatementsFollowed() {
    return STATEMENT_NUMBER_LIST();
}

STATEMENT_NUMBER_LIST PKBImplementation::getStatementsThatFollows(STATEMENT_NUMBER s) {
    return STATEMENT_NUMBER_LIST();
}

STATEMENT_NUMBER_LIST PKBImplementation::getStatementsFollows() {
    return STATEMENT_NUMBER_LIST();
}

STATEMENT_NUMBER_LIST PKBImplementation::getAncestors(STATEMENT_NUMBER s) {
    return STATEMENT_NUMBER_LIST();
}

STATEMENT_NUMBER_LIST PKBImplementation::getStatementsThatHaveAncestors() {
    return STATEMENT_NUMBER_LIST();
}

STATEMENT_NUMBER_LIST PKBImplementation::getDescendants(STATEMENT_NUMBER statementNumber) {
    return STATEMENT_NUMBER_LIST();
}

STATEMENT_NUMBER_LIST PKBImplementation::getStatementsThatHaveDescendants() {
    return STATEMENT_NUMBER_LIST();
}

STATEMENT_NUMBER_LIST PKBImplementation::getStatementsInSameBlock(STATEMENT_NUMBER s) {
    return STATEMENT_NUMBER_LIST();
}

STATEMENT_NUMBER_LIST PKBImplementation::getPreFollows(STATEMENT_NUMBER s) {
    return STATEMENT_NUMBER_LIST();
}

STATEMENT_NUMBER_LIST PKBImplementation::getPostFollows(STATEMENT_NUMBER s) {
    return STATEMENT_NUMBER_LIST();
}

} // namespace backend
