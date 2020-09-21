#include "PKBImplementation.h"

#include "DesignExtractor.h"
#include "Logger.h"
#include "Parser.h"

#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace backend {

PKBImplementation::PKBImplementation(const TNode& ast) {
    logWord("PKB starting with ast");
    logLine(ast.toString());
    auto tNodeToStatementNumber = extractor::getTNodeToStatementNumber(ast);
    auto statementNumberToTNode = extractor::getStatementNumberToTNode(tNodeToStatementNumber);
    tNodeTypeToTNodesMap = extractor::getTNodeTypeToTNodes(ast);

    // Follow
    std::tie(followFollowedRelation, followedFollowRelation) = extractor::getFollowRelationship(ast);
    allStatementsThatFollows = extractor::getKeysInMap(followFollowedRelation);
    allStatementsThatAreFollowed = extractor::getKeysInMap(followedFollowRelation);

    // Parent
    std::tie(childrenParentRelation, parentChildrenRelation) = extractor::getParentRelationship(ast);
    allStatementsThatHaveAncestors = extractor::getKeysInMap(childrenParentRelation);
    allStatementsThatHaveDescendants = extractor::getKeysInMap(parentChildrenRelation);

    // Pattern
    patternsMap = extractor::getPatternsMap(tNodeTypeToTNodesMap[Assign], tNodeToStatementNumber);
}

STATEMENT_NUMBER_LIST PKBImplementation::getAllStatements() const {
    return STATEMENT_NUMBER_LIST();
}

VARIABLE_NAME_LIST PKBImplementation::getAllVariables() const {
    return VARIABLE_NAME_LIST();
}

PROCEDURE_NAME_LIST PKBImplementation::getAllProcedures() const {
    return PROCEDURE_NAME_LIST();
}

/** -------------------------- FOLLOWS ---------------------------- **/
// TODO(weineng) optimize in the future.
STATEMENT_NUMBER_LIST PKBImplementation::getStatementsThatFollows(STATEMENT_NUMBER s) const {
    return extractor::getVisitedPathFromStart(s, followedFollowRelation);
}

// TODO(weineng) optimize in the future.
STATEMENT_NUMBER_LIST PKBImplementation::getStatementsFollowedBy(STATEMENT_NUMBER s) const {
    return extractor::getVisitedPathFromStart(s, followFollowedRelation);
}

STATEMENT_NUMBER_LIST PKBImplementation::getAllStatementsThatFollows() const {
    return allStatementsThatFollows;
}

STATEMENT_NUMBER_LIST PKBImplementation::getAllStatementsThatAreFollowed() const {
    return allStatementsThatAreFollowed;
}

/** -------------------------- PARENTS ---------------------------- **/
STATEMENT_NUMBER_LIST PKBImplementation::getAncestors(STATEMENT_NUMBER s) const {
    return extractor::getVisitedPathFromStart(s, childrenParentRelation);
}

STATEMENT_NUMBER_LIST PKBImplementation::getStatementsThatHaveAncestors() const {
    return allStatementsThatHaveAncestors;
}

STATEMENT_NUMBER_LIST PKBImplementation::getDescendants(STATEMENT_NUMBER statementNumber) const {
    std::unordered_set<int> visited;
    std::vector<int> toVisit = { statementNumber };
    while (!toVisit.empty()) {
        int visiting = toVisit.back();
        toVisit.pop_back();
        if (visited.find(visiting) != visited.end()) {
            continue;
        }
        visited.insert(visiting);
        auto it = parentChildrenRelation.find(visiting);
        if (it == parentChildrenRelation.end()) {
            continue;
        }
        toVisit.insert(toVisit.end(), it->second.begin(), it->second.end());
    }
    visited.erase(statementNumber);
    return std::vector<int>(visited.begin(), visited.end());
}

STATEMENT_NUMBER_LIST PKBImplementation::getStatementsThatHaveDescendants() const {
    return allStatementsThatHaveDescendants;
}

/** -------------------------- USES ---------------------------- **/
STATEMENT_NUMBER_LIST PKBImplementation::getStatementsThatUse(VARIABLE_NAME v) const {
    return STATEMENT_NUMBER_LIST();
}
STATEMENT_NUMBER_LIST PKBImplementation::getStatementsThatUseSomeVariable() const {
    return STATEMENT_NUMBER_LIST();
}
PROCEDURE_NAME_LIST PKBImplementation::getProceduresThatUse(VARIABLE_NAME v) const {
    return PROCEDURE_NAME_LIST();
}
PROCEDURE_NAME_LIST PKBImplementation::getProceduresThatUseSomeVariable() const {
    return PROCEDURE_NAME_LIST();
}
VARIABLE_NAME_LIST PKBImplementation::getVariablesUsedIn(PROCEDURE_NAME p) const {
    return VARIABLE_NAME_LIST();
}
VARIABLE_NAME_LIST PKBImplementation::getVariablesUsedBySomeProcedure() const {
    return VARIABLE_NAME_LIST();
}
VARIABLE_NAME_LIST PKBImplementation::getVariablesUsedIn(STATEMENT_NUMBER s) const {
    return VARIABLE_NAME_LIST();
}
VARIABLE_NAME_LIST PKBImplementation::getVariablesUsedBySomeStatement() const {
    return VARIABLE_NAME_LIST();
}

/** -------------------------- MODIFIES ---------------------------- **/
STATEMENT_NUMBER_LIST PKBImplementation::getStatementsThatModify(VARIABLE_NAME v) const {
    return STATEMENT_NUMBER_LIST();
}
STATEMENT_NUMBER_LIST PKBImplementation::getStatementsThatModifySomeVariable() const {
    return STATEMENT_NUMBER_LIST();
}
PROCEDURE_NAME_LIST PKBImplementation::getProceduresThatModify(VARIABLE_NAME v) const {
    return PROCEDURE_NAME_LIST();
}
PROCEDURE_NAME_LIST PKBImplementation::getProceduresThatModifySomeVariable() const {
    return PROCEDURE_NAME_LIST();
}
VARIABLE_NAME_LIST PKBImplementation::getVariablesModifiedBy(PROCEDURE_NAME p) const {
    return VARIABLE_NAME_LIST();
}
VARIABLE_NAME_LIST PKBImplementation::getVariablesModifiedBySomeProcedure() const {
    return VARIABLE_NAME_LIST();
}
VARIABLE_NAME_LIST PKBImplementation::getVariablesModifiedBy(STATEMENT_NUMBER s) const {
    return VARIABLE_NAME_LIST();
}
VARIABLE_NAME_LIST PKBImplementation::getVariablesModifiedBySomeStatement() const {
    return VARIABLE_NAME_LIST();
}

STATEMENT_NUMBER_LIST
PKBImplementation::getAllAssignmentStatementsThatMatch(const std::string& assignee,
                                                       const std::string& pattern,
                                                       bool isSubExpr) const {
    if (pattern.empty()) {
        // TODO(https://github.com/nus-cs3203/team24-cp-spa-20s1/issues/191):
        // return modifies(_, v);
        return STATEMENT_NUMBER_LIST();
    }

    // Preprocess pattern using the parser, to set precedence.
    std::string searchPattern = Parser::parseExpr(pattern);
    if (patternsMap.find(searchPattern) == patternsMap.end()) {
        return {};
    }
    std::vector<std::tuple<std::string, STATEMENT_NUMBER, bool>> candidateResult = patternsMap.at(searchPattern);
    if (!assignee.empty()) {
        // remove results that have a different assignee
        auto res = std::remove_if(candidateResult.begin(), candidateResult.end(),
                                  [&](const std::tuple<std::string, STATEMENT_NUMBER, bool>& x) {
                                      return std::get<0>(x) != assignee;
                                  });
        candidateResult.erase(res, candidateResult.end());
    }
    if (!isSubExpr) {
        // Remove results that are sub expressions
        auto res = std::remove_if(candidateResult.begin(), candidateResult.end(),
                                  [&](const std::tuple<std::string, STATEMENT_NUMBER, bool>& x) {
                                      return std::get<2>(x);
                                  });
        candidateResult.erase(res, candidateResult.end());
    }

    STATEMENT_NUMBER_LIST result;
    for (auto tup : candidateResult) {
        result.push_back(std::get<1>(tup));
    }
    return result;
}

} // namespace backend
