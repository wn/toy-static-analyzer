#include <Lexer.h>
#include <fstream>
#include <iostream>
#include <stdio.h>
#include <string>
#include <vector>

#include "PKB.h"
#include "TNode.h"

int PKB::setProcToAST(PROC p, TNode* r) {
    return 0;
}

TNode* PKB::getRootAST(PROC p) {
    return nullptr;
}

STATEMENTS PKB::getPreFollows(STATEMENT s) {
    return STATEMENTS();
}

STATEMENTS PKB::getPostFollows(STATEMENT s) {
    return STATEMENTS();
}

bool PKB::isFollow(STATEMENT s1, STATEMENT s2) {
    return false;
}

STATEMENTS PKB::getParents(STATEMENT s) {
    return STATEMENTS();
}

STATEMENTS PKB::getChildren(STATEMENT s) {
    return STATEMENTS();
}

bool PKB::isParent(STATEMENT s1, STATEMENT s2) {
    return false;
}

STATEMENTS PKB::getBlockStatementsNumber(STATEMENT s) {
    return STATEMENTS();
}
