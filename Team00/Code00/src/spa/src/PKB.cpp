#include <Lexer.h>
#include <fstream>
#include <iostream>
#include <stdio.h>
#include <string>
#include <vector>

#include "PKB.h"
#include "TNode.h"

namespace backend {
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

STATEMENTS PKB::getAncestors(STATEMENT s) {
    return STATEMENTS();
}

STATEMENTS PKB::getDescendents(STATEMENT s) {
    return STATEMENTS();
}

STATEMENTS PKB::getBlockStatementsNumber(STATEMENT s) {
    return STATEMENTS();
}
} // namespace backend
