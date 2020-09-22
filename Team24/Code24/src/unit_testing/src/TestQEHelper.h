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
    // code 2: VP5 in https://tinyurl.com/y2w76kw5 -- for Uses & Modifies
    // code 3: code 4 in https://tinyurl.com/y2heef28 -- for isRead/Print/Call/While/IfElse/Assign
    int test_idx;

    STATEMENT_NUMBER_LIST getPreFollows(STATEMENT_NUMBER s) const;
    STATEMENT_NUMBER_LIST getPostFollows(STATEMENT_NUMBER s) const;

    bool isRead(STATEMENT_NUMBER s) const;
    bool isPrint(STATEMENT_NUMBER s) const;
    bool isCall(STATEMENT_NUMBER s) const;
    bool isWhile(STATEMENT_NUMBER s) const;
    bool isIfElse(STATEMENT_NUMBER s) const;
    bool isAssign(STATEMENT_NUMBER s) const;

    const STATEMENT_NUMBER_LIST& getAllStatements() const;
    const VARIABLE_NAME_LIST& getAllVariables() const;
    const PROCEDURE_NAME_LIST& getAllProcedures() const;
    const CONSTANT_NAME_SET& getAllConstants() const;

    // FOLLOWS
    STATEMENT_NUMBER_LIST getDirectFollow(STATEMENT_NUMBER s) const;
    STATEMENT_NUMBER_LIST getDirectFollowedBy(STATEMENT_NUMBER s) const;
    STATEMENT_NUMBER_LIST getStatementsFollowedBy(STATEMENT_NUMBER s) const;
    STATEMENT_NUMBER_LIST getAllStatementsThatAreFollowed() const;
    STATEMENT_NUMBER_LIST getStatementsThatFollows(STATEMENT_NUMBER s) const;
    STATEMENT_NUMBER_LIST getAllStatementsThatFollows() const;

    // PARENT
    STATEMENT_NUMBER_LIST getParent(STATEMENT_NUMBER statementNumber) const;
    STATEMENT_NUMBER_LIST getChildren(STATEMENT_NUMBER statementNumber) const;
    STATEMENT_NUMBER_LIST getAncestors(STATEMENT_NUMBER statementNumber) const;
    STATEMENT_NUMBER_LIST getStatementsThatHaveAncestors() const;
    STATEMENT_NUMBER_LIST getDescendants(STATEMENT_NUMBER statementNumber) const;
    STATEMENT_NUMBER_LIST getStatementsThatHaveDescendants() const;

    STATEMENT_NUMBER_LIST getStatementsThatUse(VARIABLE_NAME v) const;
    STATEMENT_NUMBER_LIST getStatementsThatUseSomeVariable() const;
    PROCEDURE_NAME_LIST getProceduresThatUse(VARIABLE_NAME v) const;
    PROCEDURE_NAME_LIST getProceduresThatUseSomeVariable() const;
    VARIABLE_NAME_LIST getVariablesUsedIn(PROCEDURE_NAME p) const;
    VARIABLE_NAME_LIST getVariablesUsedBySomeProcedure() const;
    VARIABLE_NAME_LIST getVariablesUsedIn(STATEMENT_NUMBER s) const;
    VARIABLE_NAME_LIST getVariablesUsedBySomeStatement() const;

    STATEMENT_NUMBER_LIST getStatementsThatModify(VARIABLE_NAME v) const;
    STATEMENT_NUMBER_LIST getStatementsThatModifySomeVariable() const;
    PROCEDURE_NAME_LIST getProceduresThatModify(VARIABLE_NAME v) const;
    PROCEDURE_NAME_LIST getProceduresThatModifySomeVariable() const;
    VARIABLE_NAME_LIST getVariablesModifiedBy(PROCEDURE_NAME p) const;
    VARIABLE_NAME_LIST getVariablesModifiedBySomeProcedure() const;
    VARIABLE_NAME_LIST getVariablesModifiedBy(STATEMENT_NUMBER s) const;
    VARIABLE_NAME_LIST getVariablesModifiedBySomeStatement() const;

    STATEMENT_NUMBER_LIST
    getAllAssignmentStatementsThatMatch(const std::string& assignee, const std::string& pattern, bool isSubExpr) const;
};

// For string representing two vectors
bool checkIfVectorOfStringMatch(const std::vector<std::string>& lst1, const std::vector<std::string>& lst2);
} // namespace qetest
} // namespace qpbackend
