#include "PKB.h"
#include "TNode.h"

#include <algorithm>
#include <string>
#include <vector>

namespace qpbackend {
namespace qetest {
/**
 * Evaluation of queries involves retrieving answers from PKB.
 * For dependency injection in testing, a mock PKB API is used
 * where answers to relation or pattern inquiries are hardcoded
 */
class PKBMock : public backend::PKB {
  public:
    explicit PKBMock(int idx) : test_idx(idx) {
    }

    // the index of the source code used for testing
    // code 0: https://tinyurl.com/y32cfm6c -- for Follows & Follows*
    // code 1: nested structure {1, {{2, {3}}, {4, {5, 6}}}} -- for Parent & Parent*
    // code 2: VP5 in https://tinyurl.com/y2w76kw5 -- for Uses & Modifies & Next/Next*, Pattern
    // Assign code 3: code 4 in https://tinyurl.com/y2heef28 -- for
    // isRead/Print/Call/While/IfElse/Assign code 4: VP6 in https://tinyurl.com/y2w76kw5 -- for
    // Calls/Calls* code 5: VP8 in https://tinyurl.com/y2w76kw5 -- for while pattern
    int test_idx;

    bool isRead(STATEMENT_NUMBER s) const override;
    bool isPrint(STATEMENT_NUMBER s) const override;
    bool isCall(STATEMENT_NUMBER s) const override;
    bool isWhile(STATEMENT_NUMBER s) const override;
    bool isIfElse(STATEMENT_NUMBER s) const override;
    bool isAssign(STATEMENT_NUMBER s) const override;

    const STATEMENT_NUMBER_SET getCallStatementsWithProcedureName(PROCEDURE_NAME procedureName) const override;
    const PROCEDURE_NAME getProcedureNameFromCallStatement(STATEMENT_NUMBER callStatementNumber) const override;
    const STATEMENT_NUMBER_SET getReadStatementsWithVariableName(VARIABLE_NAME variableName) const override;
    const VARIABLE_NAME getVariableNameFromReadStatement(STATEMENT_NUMBER readStatementNumber) const override;
    const STATEMENT_NUMBER_SET getPrintStatementsWithVariableName(VARIABLE_NAME variableName) const override;
    const VARIABLE_NAME getVariableNameFromPrintStatement(STATEMENT_NUMBER printStatementNumber) const override;

    const STATEMENT_NUMBER_SET& getAllStatements() const override;
    const VARIABLE_NAME_LIST& getAllVariables() const override;
    const PROCEDURE_NAME_LIST& getAllProcedures() const override;
    const CONSTANT_NAME_SET& getAllConstants() const override;

    // FOLLOWS
    STATEMENT_NUMBER_SET getDirectFollow(STATEMENT_NUMBER s) const override;
    STATEMENT_NUMBER_SET getDirectFollowedBy(STATEMENT_NUMBER s) const override;
    STATEMENT_NUMBER_SET getStatementsFollowedBy(STATEMENT_NUMBER s) const override;
    STATEMENT_NUMBER_SET getAllStatementsThatAreFollowed() const override;
    STATEMENT_NUMBER_SET getStatementsThatFollows(STATEMENT_NUMBER s) const override;
    STATEMENT_NUMBER_SET getAllStatementsThatFollows() const override;

    // PARENT
    STATEMENT_NUMBER_SET getParent(STATEMENT_NUMBER statementNumber) const override;
    STATEMENT_NUMBER_SET getChildren(STATEMENT_NUMBER statementNumber) const override;
    STATEMENT_NUMBER_SET getAncestors(STATEMENT_NUMBER statementNumber) const override;
    STATEMENT_NUMBER_SET getStatementsThatHaveAncestors() const override;
    STATEMENT_NUMBER_SET getDescendants(STATEMENT_NUMBER statementNumber) const override;
    STATEMENT_NUMBER_SET getStatementsThatHaveDescendants() const override;

    STATEMENT_NUMBER_SET getStatementsThatUse(VARIABLE_NAME v) const override;
    STATEMENT_NUMBER_SET getStatementsThatUseSomeVariable() const override;
    PROCEDURE_NAME_LIST getProceduresThatUse(VARIABLE_NAME v) const override;
    PROCEDURE_NAME_LIST getProceduresThatUseSomeVariable() const override;
    VARIABLE_NAME_LIST getVariablesUsedIn(PROCEDURE_NAME p) const override;
    VARIABLE_NAME_LIST getVariablesUsedBySomeProcedure() const override;
    VARIABLE_NAME_LIST getVariablesUsedIn(STATEMENT_NUMBER s) const override;
    VARIABLE_NAME_LIST getVariablesUsedBySomeStatement() const override;

    STATEMENT_NUMBER_SET getStatementsThatModify(VARIABLE_NAME v) const override;
    STATEMENT_NUMBER_SET getStatementsThatModifySomeVariable() const override;
    PROCEDURE_NAME_LIST getProceduresThatModify(VARIABLE_NAME v) const override;
    PROCEDURE_NAME_LIST getProceduresThatModifySomeVariable() const override;
    VARIABLE_NAME_LIST getVariablesModifiedBy(PROCEDURE_NAME p) const override;
    VARIABLE_NAME_LIST getVariablesModifiedBySomeProcedure() const override;
    VARIABLE_NAME_LIST getVariablesModifiedBy(STATEMENT_NUMBER s) const override;
    VARIABLE_NAME_LIST getVariablesModifiedBySomeStatement() const override;

    PROCEDURE_NAME_SET getProcedureThatCalls(const VARIABLE_NAME& procedureName, bool isTransitive) const override;
    PROCEDURE_NAME_SET getProceduresCalledBy(const VARIABLE_NAME& procedureName, bool isTransitive) const override;
    const PROCEDURE_NAME_SET& getAllProceduresThatCallSomeProcedure() const override;
    const PROCEDURE_NAME_SET& getAllCalledProcedures() const override;

    STATEMENT_NUMBER_SET getNextStatementOf(STATEMENT_NUMBER statementNumber, bool isTransitive) const override;
    STATEMENT_NUMBER_SET getPreviousStatementOf(STATEMENT_NUMBER statementNumber, bool isTransitive) const override;

    STATEMENT_NUMBER_SET
    getAllAssignmentStatementsThatMatch(const std::string& assignee, const std::string& pattern, bool isSubExpr) const override;
    STATEMENT_NUMBER_SET getAllWhileStatementsThatMatch(const VARIABLE_NAME& variable,
                                                        const std::string& pattern,
                                                        bool isSubExpr) const override;
    STATEMENT_NUMBER_SET getAllIfElseStatementsThatMatch(const VARIABLE_NAME& variable,
                                                         const std::string& ifPattern,
                                                         bool ifPatternIsSubExpr,
                                                         const std::string& elsePattern,
                                                         bool elsePatternIsSubExpr) const override;
    const STATEMENT_NUMBER_SET& getAllStatementsWithNext() const override;
    const STATEMENT_NUMBER_SET& getAllStatementsWithPrev() const override;

    PROGRAM_LINE_SET getStatementsAffectedBy(PROGRAM_LINE statementNumber, bool isTransitive) const override;
    PROGRAM_LINE_SET getStatementsThatAffect(PROGRAM_LINE statementNumber, bool isTransitive) const override;
    const PROGRAM_LINE_SET& getAllStatementsThatAffect() const override;
    const PROGRAM_LINE_SET& getAllStatementsThatAreAffected() const override;
};

// For string representing two vectors
bool checkIfVectorOfStringMatch(const std::vector<std::string>& lst1, const std::vector<std::string>& lst2);
} // namespace qetest
} // namespace qpbackend
