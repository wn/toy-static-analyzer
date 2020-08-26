#pragma once

#include <iostream>
#include <stdio.h>
#include <string>
#include <vector>

typedef short PROC;
typedef int STATEMENT;
typedef std::vector<STATEMENT> STATEMENTS;

namespace backend {
class TNode;

class VarTable; // no need to #include "VarTable.h" as all I need is pointer

class PKB {
  public:
    static VarTable* varTable;
    static int setProcToAST(PROC p, TNode* r);
    static TNode* getRootAST(PROC p);

    /* API for follows */
    // Given s, return all s' such that follow*(s', s).
    STATEMENTS getPreFollows(STATEMENT s);
    // Given s, return all s' such that follow*(s, s').
    STATEMENTS getPostFollows(STATEMENT s);

    /* API for parents */
    STATEMENTS getAncestors(STATEMENT s);
    STATEMENTS getDescendents(STATEMENT s);

  private:
    // get all statements in the same block as s.
    STATEMENTS getBlockStatementsNumber(STATEMENT s);
};
} // namespace backend
