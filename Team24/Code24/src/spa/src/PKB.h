#pragma once

#include <TNode.h>
#include <string>
#include <vector>

typedef std::string PROCEDURE;
typedef std::vector<std::string> PROCEDURE_LIST;
typedef std::string VARIABLE;
typedef std::vector<std::string> VARIABLE_LIST;
typedef int STATEMENT_NUMBER;
typedef std::vector<STATEMENT_NUMBER> STATEMENT_NUMBER_LIST;

namespace backend {
class PKB {
  public:
    /* -- MASS RETRIEVAL OF DESIGN ENTITIES -- */

    // Retrieves all statements in the SIMPLE program.
    // Prefer:
    // - STATEMENT_NUMBER_LIST getAllAssignmentStatementsThatMatch(PATTERN p);
    // - STATEMENT_NUMBER_LIST getAllAssignmentStatementsFor(VARIABLE v);
    // when pattern matching as they are optimized.
    virtual STATEMENT_NUMBER_LIST getAllStatements() const = 0;

    // Retrieves all variables used in the SIMPLE program.
    virtual VARIABLE_LIST getAllVariables() const = 0;

    // Retrieves all procedures defined in the SIMPLE program.
    virtual PROCEDURE_LIST getAllProcedures() const = 0;

    /* -- FOLLOWS / FOLLOWS* -- */

    // Retrieves all the statements that appear after the statement at
    // statementNumber, in the same "level" of the AST, in order.
    // i.e. a list of statements is returned such that for statement s,
    // Follows*(<statement at statementNumber>, s) holds true.
    //
    // Usage: To get s1 such that Follows(9, s1),
    // statements_after_statement_9 = getStatementsFollowedBy(9)
    // first_statement_after_statement_9 = statements_after_statement_9[0]
    virtual STATEMENT_NUMBER_LIST getStatementsFollowedBy(STATEMENT_NUMBER s) const = 0;
    // Get all statements that are followed by some statement.
    virtual STATEMENT_NUMBER_LIST getAllStatementsThatAreFollowed() const = 0;

    // Similarly, get all the statements that appear before.
    // More formally, for a given Statement s, return all s' such that Follow*(s, s').
    virtual STATEMENT_NUMBER_LIST getStatementsThatFollows(STATEMENT_NUMBER s) const = 0;
    // Get all statements that follow some statement.
    virtual STATEMENT_NUMBER_LIST getAllStatementsThatFollows() const = 0;


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
    virtual STATEMENT_NUMBER_LIST getAncestors(STATEMENT_NUMBER statementNumber) const = 0;
    virtual STATEMENT_NUMBER_LIST getStatementsThatHaveAncestors() const = 0;

    // Similarly, get all the statements that are descendants
    // of the statement at this statement number.
    virtual STATEMENT_NUMBER_LIST getDescendants(STATEMENT_NUMBER statementNumber) const = 0;
    virtual STATEMENT_NUMBER_LIST getStatementsThatHaveDescendants() const = 0;
};
} // namespace backend
