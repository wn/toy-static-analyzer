#pragma once

#include <set>
#include <unordered_set>
#include <vector>

typedef std::string PROCEDURE_NAME;
typedef std::vector<PROCEDURE_NAME> PROCEDURE_NAME_LIST;
typedef std::unordered_set<std::string> PROCEDURE_NAME_SET;

typedef std::string VARIABLE_NAME;
typedef std::vector<VARIABLE_NAME> VARIABLE_NAME_LIST;
typedef std::unordered_set<std::string> VARIABLE_NAME_SET;

typedef int STATEMENT_NUMBER;
typedef std::unordered_set<STATEMENT_NUMBER> STATEMENT_NUMBER_SET;

typedef std::string CONSTANT_NAME;
typedef std::unordered_set<CONSTANT_NAME> CONSTANT_NAME_SET;


namespace backend {
class PKB {
  public:
    PKB() = default;
    /* -- MASS RETRIEVAL OF DESIGN ENTITIES -- */

    // Retrieves all statements in the SIMPLE program.
    virtual const STATEMENT_NUMBER_SET& getAllStatements() const = 0;

    // Retrieves all variables name used in the SIMPLE program.
    virtual const VARIABLE_NAME_LIST& getAllVariables() const = 0;

    // Retrieves all procedures name defined in the SIMPLE program.
    virtual const PROCEDURE_NAME_LIST& getAllProcedures() const = 0;

    // Retrieves all constants value defined in the SIMPLE program.
    virtual const CONSTANT_NAME_SET& getAllConstants() const = 0;

    /* -- Check type of statement number -- */
    virtual bool isRead(STATEMENT_NUMBER s) const = 0;
    virtual bool isPrint(STATEMENT_NUMBER s) const = 0;
    virtual bool isCall(STATEMENT_NUMBER s) const = 0;
    virtual bool isWhile(STATEMENT_NUMBER s) const = 0;
    virtual bool isIfElse(STATEMENT_NUMBER s) const = 0;
    virtual bool isAssign(STATEMENT_NUMBER s) const = 0;


    /* -- ATTRIBUTE-BASED RETRIEVAL * -- */
    virtual const STATEMENT_NUMBER_SET getCallStatementsWithProcedureName(PROCEDURE_NAME procedureName) const = 0;
    virtual const PROCEDURE_NAME
    getProcedureNameFromCallStatement(STATEMENT_NUMBER callStatementNumber) const = 0;

    virtual const STATEMENT_NUMBER_SET getReadStatementsWithVariableName(VARIABLE_NAME variableName) const = 0;
    virtual const VARIABLE_NAME getVariableNameFromReadStatement(STATEMENT_NUMBER readStatementNumber) const = 0;

    virtual const STATEMENT_NUMBER_SET getPrintStatementsWithVariableName(VARIABLE_NAME variableName) const = 0;
    virtual const VARIABLE_NAME
    getVariableNameFromPrintStatement(STATEMENT_NUMBER printStatementNumber) const = 0;

    /* -- FOLLOWS / FOLLOWS* -- */

    // Retrieves all the statements that appear after the statement at
    // statementNumber, in the same "level" of the AST, in order.
    // i.e. a list of statements is returned such that for statement s,
    // Follows*(<statement at statementNumber>, s) holds true.
    //
    // Usage: To get s1 such that Follows(s1, 9)
    // first_statement_before_statement_9 = getDirectFollow(9)
    // If the result is empty, there is no statement that directly follows s
    virtual STATEMENT_NUMBER_SET getDirectFollow(STATEMENT_NUMBER s) const = 0;
    // Usage: To get s1 such that Follows(9, s1)
    // first_statement_after_statement_9 = getDirectFollowedBy(9)
    // If the result is empty, there is no statement that is directly followed by s
    virtual STATEMENT_NUMBER_SET getDirectFollowedBy(STATEMENT_NUMBER s) const = 0;
    // Get all statements that are followed by statement s.
    virtual STATEMENT_NUMBER_SET getStatementsFollowedBy(STATEMENT_NUMBER s) const = 0;
    // Get all statements that are followed by some statement.
    virtual STATEMENT_NUMBER_SET getAllStatementsThatAreFollowed() const = 0;

    // Similarly, get all the statements that appear before.
    // More formally, for a given Statement s, return all s' such that Follow*(s, s').
    virtual STATEMENT_NUMBER_SET getStatementsThatFollows(STATEMENT_NUMBER s) const = 0;
    // Get all statements that follow some statement.
    virtual STATEMENT_NUMBER_SET getAllStatementsThatFollows() const = 0;


    /* -- PARENT / PARENT* -- */
    // Retrieves all the statements that (TODO(jeff) see this comment)
    // have the statement at `statementNumber` as a direct or indirect descendant.
    //
    // A list of statements is returned such that for statement s,
    // Parent*(s, <statement at statementNumber>) holds true.
    //
    // Usage: To get all s1 such that Parent*(s1, 9),
    // ancestors = getAncestors(9)
    //
    // Usage: To get the s1 such that Parent(s1, 9):
    // parent_of_9 = getParent(9)
    // Usage: To get the s1 such that Parent(9, s1):
    // children_of_9 = getChildren(9)
    virtual STATEMENT_NUMBER_SET getParent(STATEMENT_NUMBER statementNumber) const = 0;
    virtual STATEMENT_NUMBER_SET getChildren(STATEMENT_NUMBER statementNumber) const = 0;
    virtual STATEMENT_NUMBER_SET getAncestors(STATEMENT_NUMBER statementNumber) const = 0;
    virtual STATEMENT_NUMBER_SET getStatementsThatHaveAncestors() const = 0;

    // Similarly, get all the statements that are descendants
    // of the statement at this statement number.
    virtual STATEMENT_NUMBER_SET getDescendants(STATEMENT_NUMBER statementNumber) const = 0;
    virtual STATEMENT_NUMBER_SET getStatementsThatHaveDescendants() const = 0;

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
    virtual STATEMENT_NUMBER_SET getStatementsThatUse(VARIABLE_NAME v) const = 0;
    virtual STATEMENT_NUMBER_SET getStatementsThatUseSomeVariable() const = 0;

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
    virtual STATEMENT_NUMBER_SET getStatementsThatModify(VARIABLE_NAME v) const = 0;
    virtual STATEMENT_NUMBER_SET getStatementsThatModifySomeVariable() const = 0;


    virtual PROCEDURE_NAME_LIST getProceduresThatModify(VARIABLE_NAME v) const = 0;
    virtual PROCEDURE_NAME_LIST getProceduresThatModifySomeVariable() const = 0;

    // Procedure
    virtual VARIABLE_NAME_LIST getVariablesModifiedBy(PROCEDURE_NAME p) const = 0;
    virtual VARIABLE_NAME_LIST getVariablesModifiedBySomeProcedure() const = 0;

    // Statement
    virtual VARIABLE_NAME_LIST getVariablesModifiedBy(STATEMENT_NUMBER s) const = 0;
    virtual VARIABLE_NAME_LIST getVariablesModifiedBySomeStatement() const = 0;

    // -- CALLS -- */
    // For any procedures p and q:
    //    Calls (p, q) holds if procedure p directly calls q
    //    Calls* (p, q) holds if procedure p directly or indirectly calls q , that is:
    //        Calls (p, q) or
    //        Calls (p, p1) and Calls* (p1, q) for some procedure p1

    // all procedures that calls procedureName.
    virtual PROCEDURE_NAME_SET
    getProcedureThatCalls(const VARIABLE_NAME& procedureName, bool isTransitive) const = 0;
    // get all procedures that are called by procedureName.
    virtual PROCEDURE_NAME_SET
    getProceduresCalledBy(const VARIABLE_NAME& procedureName, bool isTransitive) const = 0;

    // -- NEXTS -- */
    // For two program lines n1 and n2 in the same procedure:
    //
    //    Next (n1, n2) holds if n2 can be executed immediately after n1 in some execution sequence
    //    Next* (n1, n2) holds if n2 can be executed after n1 in some execution sequence

    // all possible next statements that `statementNumber` can go to.
    virtual STATEMENT_NUMBER_SET
    getNextStatementOf(STATEMENT_NUMBER statementNumber, bool isTransitive) const = 0;
    // list of all statement numbers that goes to `statementNumber`
    virtual STATEMENT_NUMBER_SET
    getPreviousStatementOf(STATEMENT_NUMBER statementNumber, bool isTransitive) const = 0;

    virtual const STATEMENT_NUMBER_SET& getAllStatementsWithNext() const = 0;
    virtual const STATEMENT_NUMBER_SET& getAllStatementsWithPrev() const = 0;

    /* -- Patterns -- */
    // Get all assignment statements that matches the input pattern.
    // Example:
    //     pattern a(_, "_1+1_") -> getAllAssignmentStatementsThatMatch("", "1+1", true);
    //     pattern a("x", "_") -> getAllAssignmentStatementsThatMatch("x", "", true);
    //     pattern a("x", "1+1") -> getAllAssignmentStatementsThatMatch("x", "1+1", false)
    virtual STATEMENT_NUMBER_SET getAllAssignmentStatementsThatMatch(const std::string& assignee,
                                                                     const std::string& pattern,
                                                                     bool isSubExpr) const = 0;


    // TODO(weineng) this method currently do not take in any pattern. isSubExpr is always true.
    /**
     * Example:
     *   pattern w(_, "_") -> getAllWhileStatementsThatMatch("", "", true, "", true);
     *   pattern w("x", "_") -> getAllWhileStatementsThatMatch("x", "", true, "", true);
     * @param variable - variable in a condition. If variable is "_", we consider is as a wildcard.
     * @param pattern - expression that can be found in an assignment statement of the while-block
     * @param isSubExpr - whether pattern is a sub-expression
     * @return while statements that contain the variable and whose body contain an assign stmt that
     * matches the pattern
     */
    virtual STATEMENT_NUMBER_SET getAllWhileStatementsThatMatch(const VARIABLE_NAME& variable,
                                                                const std::string& pattern,
                                                                bool isSubExpr) const = 0;

    // TODO(weineng) this method currently do not take in any pattern. isSubExpr is always true.
    /**
     * Example:
     *   pattern ifs(_, "_") -> getAllIfElseStatementsThatMatch("", "", true, "", true);
     *   pattern ifs("x", "_") -> getAllIfElseStatementsThatMatch("x", "", true, "", true);
     * @param variable - variable in a condition. If variable is "_", we consider is as a wildcard.
     * @param ifPattern - expression that can be found in an assignment statement of the if-block
     * @param ifPatternIsSubExpr - whether if-pattern is a sub-expression
     * @param elsePattern - expression that can be found in an assignment statement of the else-block
     * @param elsePatternIsSubExpr - whether else-pattern is a sub-expression
     * @return if-else statements that contain the variable and has if and else block that matches
     * the pattern
     */
    virtual STATEMENT_NUMBER_SET getAllIfElseStatementsThatMatch(const VARIABLE_NAME& variable,
                                                                 const std::string& ifPattern,
                                                                 bool ifPatternIsSubExpr,
                                                                 const std::string& elsePattern,
                                                                 bool elsePatternIsSubExpr) const = 0;
};
} // namespace backend
