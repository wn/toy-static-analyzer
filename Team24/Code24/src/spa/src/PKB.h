#pragma once

#include <string>
#include <vector>

typedef std::string PROCEDURE_NAME;
typedef std::vector<std::string> PROCEDURE_NAME_LIST;
typedef std::string VARIABLE_NAME;
typedef std::vector<std::string> VARIABLE_NAME_LIST;
typedef int STATEMENT_NUMBER;
typedef std::vector<STATEMENT_NUMBER> STATEMENT_NUMBER_LIST;

namespace backend {
class PKB {
  public:
    /* -- MASS RETRIEVAL OF DESIGN ENTITIES -- */

    // Retrieves all statements in the SIMPLE program.
    // Prefer:
    // - STATEMENT_NUMBER_LIST getAllAssignmentStatementsThatMatch(PATTERN p);
    // - STATEMENT_NUMBER_LIST getAllAssignmentStatementsFor(VARIABLE_NAME v);
    // when pattern matching as they are optimized.
    virtual STATEMENT_NUMBER_LIST getAllStatements() const = 0;

    // Retrieves all variables used in the SIMPLE program.
    virtual VARIABLE_NAME_LIST getAllVariables() const = 0;

    // Retrieves all procedures defined in the SIMPLE program.
    virtual PROCEDURE_NAME_LIST getAllProcedures() const = 0;

    /* -- Check type of statement number -- */
    virtual bool isRead(STATEMENT_NUMBER s) const = 0;
    virtual bool isPrint(STATEMENT_NUMBER s) const = 0;
    virtual bool isCall(STATEMENT_NUMBER s) const = 0;
    virtual bool isWhile(STATEMENT_NUMBER s) const = 0;
    virtual bool isIfElse(STATEMENT_NUMBER s) const = 0;
    virtual bool isAssign(STATEMENT_NUMBER s) const = 0;

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

    /* -- USES -- */
    // Get all statements that Uses v
    // Example query:
    //     variable v; select v such that uses(_,v);
    // Possible query plan:
    //     allVariables = getAllVariables();
    //     return [v for v in allVariables if len(getStatementsThatUse(v)) > 0]
    //
    // Example query:
    //     stmt s; select s such that uses(s,"v");
    // Possible query plan:
    //     return getStatementsThatUse("v")
    virtual STATEMENT_NUMBER_LIST getStatementsThatUse(VARIABLE_NAME v) const = 0;
    virtual STATEMENT_NUMBER_LIST getStatementsThatUseSomeVariable() const = 0;

    // Get all procedure that Uses v
    virtual PROCEDURE_NAME_LIST getProceduresThatUse(VARIABLE_NAME v) const = 0;
    virtual PROCEDURE_NAME_LIST getProceduresThatUseSomeVariable() const = 0;

    // Get all variables "v" such that Procedure p Uses v
    virtual VARIABLE_NAME_LIST getVariablesUsedIn(PROCEDURE_NAME p) const = 0;
    virtual VARIABLE_NAME_LIST getVariablesUsedBySomeProcedure() const = 0;

    // Get all variables "v" such that Statement s Uses v
    virtual VARIABLE_NAME_LIST getVariablesUsedIn(STATEMENT_NUMBER s) const = 0;
    virtual VARIABLE_NAME_LIST getVariablesUsedBySomeStatement() const = 0;

    /* -- MODIFIES -- */
    // Get all statements that modify v
    // Example query:
    //     variable v; select v such that Modify();
    // Possible query plan:
    //     allVariables = getAllVariables();
    //     return [v for v in allVariables if len(getStatementsThatUse(v)) > 0]
    //
    // Example query:
    //     stmt s; select s such that uses(s,"v");
    // Possible query plan:
    //     return getStatementsThatUse("v")
    virtual STATEMENT_NUMBER_LIST getStatementsThatModify(VARIABLE_NAME v) const = 0;
    virtual STATEMENT_NUMBER_LIST getStatementsThatModifySomeVariable() const = 0;


    virtual PROCEDURE_NAME_LIST getProceduresThatModify(VARIABLE_NAME v) const = 0;
    virtual PROCEDURE_NAME_LIST getProceduresThatModifySomeVariable() const = 0;

    // Procedure
    virtual VARIABLE_NAME_LIST getVariablesModifiedBy(PROCEDURE_NAME p) const = 0;
    virtual VARIABLE_NAME_LIST getVariablesModifiedBySomeProcedure() const = 0;

    // Statement
    virtual VARIABLE_NAME_LIST getVariablesModifiedBy(STATEMENT_NUMBER s) const = 0;
    virtual VARIABLE_NAME_LIST getVariablesModifiedBySomeStatement() const = 0;

    /* -- Patterns -- */
    // Get all statements that matches the input pattern.
    // Example:
    //     pattern a(_, "_1+1_") -> getAllAssignmentStatementsThatMatch("", "1+1", true);
    //     pattern a("x", "_") -> getAllAssignmentStatementsThatMatch("x", "", true);
    //     pattern a("x", "1+1") -> getAllAssignmentStatementsThatMatch("x", "1+1", false)
    virtual STATEMENT_NUMBER_LIST getAllAssignmentStatementsThatMatch(const std::string& assignee,
                                                                      const std::string& pattern,
                                                                      bool isSubExpr) const = 0;
};
} // namespace backend
