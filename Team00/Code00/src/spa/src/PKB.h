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
    bool isFollow(STATEMENT s1, STATEMENT s2, bool glob = false);

    /* API for parents */
    STATEMENTS getAncestors(STATEMENT s);
    STATEMENTS getDescendents(STATEMENT s);
    // Check if s1 is an ancestor of s2
    bool isAncestor(STATEMENT s1, STATEMENT s2, bool glob = false);

  private:
    // get all statements in the same block as s.
    STATEMENTS getBlockStatementsNumber(STATEMENT s);
};
} // namespace backend
