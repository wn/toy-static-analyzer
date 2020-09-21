#pragma once

#include "PKB.h"
#include "TNode.h"

#include <set>
#include <unordered_map>

/**
typedef std::string PROCEDURE_NAME;
typedef std::vector<std::string> PROCEDURE_NAME_LIST;
typedef std::string VARIABLE_NAME;
typedef std::vector<std::string> VARIABLE_NAME_LIST;
typedef int STATEMENT_NUMBER;
typedef std::vector<STATEMENT_NUMBER> STATEMENT_NUMBER_LIST;
**/

typedef std::set<std::string> PROCEDURE_NAME_SET;
typedef std::set<std::string> VARIABLE_NAME_SET;
typedef std::set<STATEMENT_NUMBER> STATEMENT_NUMBER_SET;

namespace backend {
class PKBImplementation : virtual public backend::PKB {
  public:
    explicit PKBImplementation(const TNode& ast);
    STATEMENT_NUMBER_LIST getAllStatements() const override;
    VARIABLE_NAME_LIST getAllVariables() const override;
    PROCEDURE_NAME_LIST getAllProcedures() const override;

    bool isRead(STATEMENT_NUMBER s) const override;
    bool isPrint(STATEMENT_NUMBER s) const override;
    bool isCall(STATEMENT_NUMBER s) const override;
    bool isWhile(STATEMENT_NUMBER s) const override;
    bool isIfElse(STATEMENT_NUMBER s) const override;
    bool isAssign(STATEMENT_NUMBER s) const override;

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
    PROCEDURE_NAME_LIST getProceduresThatUse(VARIABLE_NAME v) const override;
    PROCEDURE_NAME_LIST getProceduresThatUseSomeVariable() const override;
    VARIABLE_NAME_LIST getVariablesUsedIn(PROCEDURE_NAME p) const override;
    VARIABLE_NAME_LIST getVariablesUsedBySomeProcedure() const override;
    VARIABLE_NAME_LIST getVariablesUsedIn(STATEMENT_NUMBER s) const override;
    VARIABLE_NAME_LIST getVariablesUsedBySomeStatement() const override;

    STATEMENT_NUMBER_LIST getStatementsThatModify(VARIABLE_NAME v) const override;
    STATEMENT_NUMBER_LIST getStatementsThatModifySomeVariable() const override;
    PROCEDURE_NAME_LIST getProceduresThatModify(VARIABLE_NAME v) const override;
    PROCEDURE_NAME_LIST getProceduresThatModifySomeVariable() const override;
    VARIABLE_NAME_LIST getVariablesModifiedBy(PROCEDURE_NAME p) const override;
    VARIABLE_NAME_LIST getVariablesModifiedBySomeProcedure() const override;
    VARIABLE_NAME_LIST getVariablesModifiedBy(STATEMENT_NUMBER s) const override;
    VARIABLE_NAME_LIST getVariablesModifiedBySomeStatement() const override;

    // Pattern
    STATEMENT_NUMBER_LIST
    getAllAssignmentStatementsThatMatch(const std::string& assignee, const std::string& pattern, bool isSubExpr) const override;

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


    // Uses helper:
    std::unordered_map<VARIABLE_NAME, STATEMENT_NUMBER_SET> variableToStatementsThatUseIt;
    STATEMENT_NUMBER_SET allStatementsThatUseSomeVariable;

    std::unordered_map<VARIABLE_NAME, PROCEDURE_NAME_SET> variableToProceduresThatUseIt;
    PROCEDURE_NAME_SET allProceduresThatThatUseSomeVariable;

    std::unordered_map<PROCEDURE_NAME, VARIABLE_NAME_SET> procedureToUsedVariables;
    VARIABLE_NAME_SET allVariablesUsedBySomeProcedure;

    std::unordered_map<STATEMENT_NUMBER, VARIABLE_NAME_SET> statementToUsedVariables;
    VARIABLE_NAME_SET allVariablesUsedBySomeStatement;

    // Pattern helper:
    std::unordered_map<std::string, std::vector<std::tuple<std::string, STATEMENT_NUMBER, bool>>> patternsMap;

    // Performance booster fields:
    std::unordered_map<TNodeType, std::vector<const TNode*>, EnumClassHash> tNodeTypeToTNodesMap;
    std::unordered_map<int, TNodeType> statementNumberToTNodeType;
};
} // namespace backend
