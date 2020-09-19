#pragma once

#include "PKB.h"
#include "TNode.h"

#include <unordered_map>

/**
typedef std::string PROCEDURE;
typedef std::vector<std::string> PROCEDURE_LIST;
typedef std::string VARIABLE;
typedef std::vector<std::string> VARIABLE_LIST;
typedef int STATEMENT_NUMBER;
typedef std::vector<STATEMENT_NUMBER> STATEMENT_NUMBER_LIST;
**/

namespace backend {
class PKBImplementation : virtual public backend::PKB {
  public:
    explicit PKBImplementation(const TNode& ast);

    STATEMENT_NUMBER_LIST getAllStatements() const;
    VARIABLE_LIST getAllVariables() const;
    PROCEDURE_LIST getAllProcedures() const;

    STATEMENT_NUMBER_LIST getStatementsFollowedBy(STATEMENT_NUMBER s) const;
    STATEMENT_NUMBER_LIST getAllStatementsThatFollows() const;
    STATEMENT_NUMBER_LIST getStatementsThatFollows(STATEMENT_NUMBER s) const;
    STATEMENT_NUMBER_LIST getAllStatementsThatAreFollowed() const;

    STATEMENT_NUMBER_LIST getAncestors(STATEMENT_NUMBER statementNumber) const;
    STATEMENT_NUMBER_LIST getStatementsThatHaveAncestors() const;
    STATEMENT_NUMBER_LIST getDescendants(STATEMENT_NUMBER statementNumber) const;
    STATEMENT_NUMBER_LIST getStatementsThatHaveDescendants() const;

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
