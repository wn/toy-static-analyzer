#pragma once

#include "DesignExtractor.h"
#include "PKB.h"
#include "Parser.h"

#include <iostream>
#include <string>
#include <vector>

typedef std::string PROCEDURE;
typedef std::vector<std::string> PROCEDURE_LIST;
typedef std::string VARIABLE;
typedef std::vector<std::string> VARIABLE_LIST;
typedef int STATEMENT_NUMBER;
typedef std::vector<STATEMENT_NUMBER> STATEMENT_NUMBER_LIST;

namespace backend {
class TNode;

class VarTable; // no need to #include "VarTable.h" as all I need is pointer

class PKB {
  public:
    explicit PKB(const TNode& ast);
    /* API for follows */
    // Given s, return all s' such that follow*(s', s).
    STATEMENT_NUMBER_LIST getPreFollows(STATEMENT_NUMBER s);
    // Given s, return all s' such that follow*(s, s').
    STATEMENT_NUMBER_LIST getPostFollows(STATEMENT_NUMBER s);

    /* -- MASS RETRIEVAL OF DESIGN ENTITIES -- */

    // Retrieves all statements in the SIMPLE program.
    // Prefer:
    // - STATEMENT_NUMBER_LIST getAllAssignmentStatementsThatMatch(PATTERN p);
    // - STATEMENT_NUMBER_LIST getAllAssignmentStatementsFor(VARIABLE v);
    // when pattern matching as they are optimized.
    STATEMENT_NUMBER_LIST getAllStatements();

    // Retrieves all variables used in the SIMPLE program.
    VARIABLE_LIST getAllVariables();

    // Retrieves all procedures defined in the SIMPLE program.
    PROCEDURE_LIST getAllProcedures();

    /* -- FOLLOWS / FOLLOWS* -- */

    // Retrieves all the statements that appear after the statement at
    // statementNumber, in the same "level" of the AST, in order.
    // i.e. a list of statements is returned such that for statement s,
    // Follows*(<statement at statementNumber>, s) holds true.
    //
    // Usage: To get all s1 such that Follows*(9, s1),
    // statements_after_statement_9 = getStatementsFollowedBy(9)
    //
    // Usage: To get s1 such that Follows(9, s1),
    // statements_after_statement_9 = getStatementsFollowedBy(9)
    // first_statement_after_statement_9 = statements_after_statement_9[0]
    STATEMENT_NUMBER_LIST getStatementsFollowedBy(STATEMENT_NUMBER s);
    // Get all statements that are followed by some statement.
    STATEMENT_NUMBER_LIST getStatementsFollowed();

    // Similarly, get all the statements that appear before.
    // More formally, for a given Statement s, return all s' such that Follow*(s, s').
    STATEMENT_NUMBER_LIST getStatementsThatFollows(STATEMENT_NUMBER s);
    // Get all statements that follow some statement.
    STATEMENT_NUMBER_LIST getStatementsFollows();


    /* -- PARENT / PARENT* -- */
    // Retrieves all the statements that (TODO(jeff) see this comment)
    // have the statement at `statementNumber` as a direct or indirect descendant.
    //
    // A list of statements is returned such that for statement s,
    // Parent*(s, <statement at statementNumber>) holds true.
    // We order the STATEMENT_LIST in order of proximity to the statement, i.e.
    // the first item in the list is the closest ancestor to the statementNumber.
    //
    // Usage: To get all s1 such that Parent*(s1, 9),
    // ancestors = getAncestors(9)
    //
    // Usage: To get the s1 such that Parent(s1, 9),
    // ancestors = getAncestors(9)
    // parent_of_9 = ancestors[0]
    STATEMENT_NUMBER_LIST getAncestors(STATEMENT_NUMBER statementNumber);
    STATEMENT_NUMBER_LIST getStatementsThatHaveAncestors();

    // Similarly, get all the statements that are descendants
    // of the statement at this statement number.
    STATEMENT_NUMBER_LIST getDescendants(STATEMENT_NUMBER statementNumber);
    STATEMENT_NUMBER_LIST getStatementsThatHaveDescendants();

  private:
    STATEMENT_NUMBER_LIST getStatementsInSameBlock(STATEMENT_NUMBER s);
};
} // namespace backend
