#include <Lexer.h>
#include <fstream>
#include <iostream>
#include <stdio.h>
#include <string>
#include <vector>

#include "PKB.h"
#include "TNode.h"

namespace backend {
int PKB::setProcToAST(PROCEDURE p, TNode* r) {
    return 0;
}

TNode* PKB::getRootAST(PROCEDURE p) {
    return nullptr;
}

STATEMENT_NUMBER_LIST PKB::getAllStatements() {
    return STATEMENT_NUMBER_LIST();
}

VARIABLE_LIST PKB::getAllVariables() {
    return VARIABLE_LIST();
}

PROCEDURE_LIST PKB::getAllProcedures() {
    return PROCEDURE_LIST();
}

STATEMENT_NUMBER_LIST PKB::getStatementsFollowedBy(STATEMENT_NUMBER s) {
    return STATEMENT_NUMBER_LIST();
}

STATEMENT_NUMBER_LIST PKB::getStatementsFollowed() {
    return STATEMENT_NUMBER_LIST();
}

STATEMENT_NUMBER_LIST PKB::getStatementsThatFollows(STATEMENT_NUMBER s) {
    return STATEMENT_NUMBER_LIST();
}

STATEMENT_NUMBER_LIST PKB::getStatementsFollows() {
    return STATEMENT_NUMBER_LIST();
}

STATEMENT_NUMBER_LIST PKB::getAncestors(STATEMENT_NUMBER s) {
    return STATEMENT_NUMBER_LIST();
}

STATEMENT_NUMBER_LIST PKB::getStatementsThatHaveAncestors() {
    return STATEMENT_NUMBER_LIST();
}

STATEMENT_NUMBER_LIST PKB::getDescendants(STATEMENT_NUMBER statementNumber) {
    return STATEMENT_NUMBER_LIST();
}

STATEMENT_NUMBER_LIST PKB::getStatementsThatHaveDescendants() {
    return STATEMENT_NUMBER_LIST();
}

STATEMENT_NUMBER_LIST PKB::getStatementsInSameBlock(STATEMENT_NUMBER s) {
    return STATEMENT_NUMBER_LIST();
}

} // namespace backend
