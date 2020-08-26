#pragma once

#include <iostream>
#include <stdio.h>
#include <string>
#include <vector>

typedef short PROC;
typedef int STATEMENT;
typedef std::vector<STATEMENT> STATEMENTS;

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
    bool isFollow(STATEMENT s1, STATEMENT s2);

    /* API for parents */
    STATEMENTS getParents(STATEMENT s);
    STATEMENTS getChildren(STATEMENT s);
    // Check if s1 is a parent of s2
    bool isParent(STATEMENT s1, STATEMENT s2);

  private:
    // get all statements in the same block as s.
    STATEMENTS getBlockStatementsNumber(STATEMENT s);
};