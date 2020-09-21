#include "PKB.h"
#include "TNode.h"

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
    // link to the code 0: https://tinyurl.com/y32cfm6c
    int test_idx;

    STATEMENT_NUMBER_LIST getPreFollows(STATEMENT_NUMBER s) const;
    STATEMENT_NUMBER_LIST getPostFollows(STATEMENT_NUMBER s) const;

    STATEMENT_NUMBER_LIST getAllStatements() const;
    VARIABLE_LIST getAllVariables() const;
    PROCEDURE_LIST getAllProcedures() const;

    // FOLLOWS
    STATEMENT_NUMBER_LIST getStatementsFollowedBy(STATEMENT_NUMBER s) const;
    STATEMENT_NUMBER_LIST getAllStatementsThatAreFollowed() const;
    STATEMENT_NUMBER_LIST getStatementsThatFollows(STATEMENT_NUMBER s) const;
    STATEMENT_NUMBER_LIST getAllStatementsThatFollows() const;

    // PARENT
    STATEMENT_NUMBER_LIST getAncestors(STATEMENT_NUMBER statementNumber) const;
    STATEMENT_NUMBER_LIST getStatementsThatHaveAncestors() const;
    STATEMENT_NUMBER_LIST getDescendants(STATEMENT_NUMBER statementNumber) const;
    STATEMENT_NUMBER_LIST getStatementsThatHaveDescendants() const;

    STATEMENT_NUMBER_LIST getStatementsThatUse(VARIABLE_NAME v) const;
    STATEMENT_NUMBER_LIST getStatementsThatUseSomeVariable() const;
    PROCEDURE_LIST getProceduresThatUse(STATEMENT_NUMBER s) const;
    PROCEDURE_LIST getProceduresThatUseSomeVariable() const;
    VARIABLE_LIST getVariablesUsedIn(PROCEDURE_NAME p) const;
    VARIABLE_LIST getVariablesUsedBySomeProcedure() const;
    VARIABLE_LIST getVariablesUsedIn(STATEMENT_NUMBER s) const;
    VARIABLE_LIST getVariablesUsedBySomeStatement() const;

    STATEMENT_NUMBER_LIST getStatementsThatModify(VARIABLE_NAME v) const;
    STATEMENT_NUMBER_LIST getStatementsThatModifySomeVariable() const;
    PROCEDURE_LIST getProceduresThatModify(VARIABLE_NAME v) const;
    PROCEDURE_LIST getProceduresThatModifySomeVariable() const;
    VARIABLE_LIST getVariablesModifiedBy(PROCEDURE_NAME p) const;
    VARIABLE_LIST getVariablesModifiedBySomeProcedure() const;
    VARIABLE_LIST getVariablesModifiedBy(STATEMENT_NUMBER s) const;
    VARIABLE_LIST getVariablesModifiedBySomeStatement() const;
};
} // namespace qetest
} // namespace qpbackend
