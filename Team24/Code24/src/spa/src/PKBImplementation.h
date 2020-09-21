#pragma once

#include "PKB.h"
#include "TNode.h"

#include <unordered_map>

/**
typedef std::string PROCEDURE_NAME;
typedef std::vector<std::string> PROCEDURE_LIST;
typedef std::string VARIABLE_NAME;
typedef std::vector<std::string> VARIABLE_LIST;
typedef int STATEMENT_NUMBER;
typedef std::vector<STATEMENT_NUMBER> STATEMENT_NUMBER_LIST;
**/

namespace backend {
class PKBImplementation : virtual public backend::PKB {
  public:
    explicit PKBImplementation(const TNode& ast);
    STATEMENT_NUMBER_LIST getAllStatements() const override;
    VARIABLE_LIST getAllVariables() const override;
    PROCEDURE_LIST getAllProcedures() const override;

    STATEMENT_NUMBER_LIST getStatementsFollowedBy(STATEMENT_NUMBER s) const override;
    STATEMENT_NUMBER_LIST getAllStatementsThatFollows() const override;
    STATEMENT_NUMBER_LIST getStatementsThatFollows(STATEMENT_NUMBER s) const override;
    STATEMENT_NUMBER_LIST getAllStatementsThatAreFollowed() const override;

    STATEMENT_NUMBER_LIST getAncestors(STATEMENT_NUMBER statementNumber) const override;
    STATEMENT_NUMBER_LIST getStatementsThatHaveAncestors() const override;
    STATEMENT_NUMBER_LIST getDescendants(STATEMENT_NUMBER statementNumber) const override;
    STATEMENT_NUMBER_LIST getStatementsThatHaveDescendants() const override;

    STATEMENT_NUMBER_LIST getStatementsThatUse(VARIABLE_NAME v) const override;
    STATEMENT_NUMBER_LIST getStatementsThatUseSomeVariable() const override;
    PROCEDURE_LIST getProceduresThatUse(STATEMENT_NUMBER s) const override;
    PROCEDURE_LIST getProceduresThatUseSomeVariable() const override;
    VARIABLE_LIST getVariablesUsedIn(PROCEDURE_NAME p) const override;
    VARIABLE_LIST getVariablesUsedBySomeProcedure() const override;
    VARIABLE_LIST getVariablesUsedIn(STATEMENT_NUMBER s) const override;
    VARIABLE_LIST getVariablesUsedBySomeStatement() const override;

    STATEMENT_NUMBER_LIST getStatementsThatModify(VARIABLE_NAME v) const override;
    STATEMENT_NUMBER_LIST getStatementsThatModifySomeVariable() const override;
    PROCEDURE_LIST getProceduresThatModify(VARIABLE_NAME v) const override;
    PROCEDURE_LIST getProceduresThatModifySomeVariable() const override;
    VARIABLE_LIST getVariablesModifiedBy(PROCEDURE_NAME p) const override;
    VARIABLE_LIST getVariablesModifiedBySomeProcedure() const override;
    VARIABLE_LIST getVariablesModifiedBy(STATEMENT_NUMBER s) const override;
    VARIABLE_LIST getVariablesModifiedBySomeStatement() const override;

  private:
    // Follows helper:
    // for k, v in map, follow(v, k).
    std::unordered_map<STATEMENT_NUMBER, STATEMENT_NUMBER> followedFollowRelation;
    // for k, v in map, follow(k, v).
    std::unordered_map<STATEMENT_NUMBER, STATEMENT_NUMBER> followFollowedRelation;
    // Stmt list is private to prevent modification.
    STATEMENT_NUMBER_LIST allStatementsThatFollows;
    STATEMENT_NUMBER_LIST allStatementsThatAreFollowed;

    // Parent helper:
    // for k, v in map, parent(k, j) for j in v
    std::unordered_map<STATEMENT_NUMBER, STATEMENT_NUMBER_LIST> parentChildrenRelation;
    // for k, v in map, parent(v, k).
    std::unordered_map<STATEMENT_NUMBER, STATEMENT_NUMBER> childrenParentRelation;
    // Stmt list is private to prevent modification.
    STATEMENT_NUMBER_LIST allStatementsThatHaveAncestors;
    STATEMENT_NUMBER_LIST allStatementsThatHaveDescendants;
};
} // namespace backend
