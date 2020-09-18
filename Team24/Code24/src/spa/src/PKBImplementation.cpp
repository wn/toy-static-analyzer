#include "PKBImplementation.h"

#include "DesignExtractor.h"
#include "Logger.h"

#include <unordered_map>
#include <vector>

namespace backend {

PKBImplementation::PKBImplementation(const TNode& ast) {
    logWord("PKB starting with ast");
    logLine(ast.toString());
    auto tNodeToStatementNumber = extractor::getTNodeToStatementNumber(ast);
    auto statementNumberToTNode = extractor::getStatementNumberToTNode(tNodeToStatementNumber);

    std::tie(followedFollowRelation, followFollowedRelation) = extractor::getFollowRelationship(ast);
    allStatementsThatFollows = extractor::getValuesInMap(followFollowedRelation);
    allStatementsThatAreFollowed = extractor::getValuesInMap(followedFollowRelation);
}

STATEMENT_NUMBER_LIST PKBImplementation::getAllStatements() const {
    return STATEMENT_NUMBER_LIST();
}

VARIABLE_LIST PKBImplementation::getAllVariables() const {
    return VARIABLE_LIST();
}

PROCEDURE_LIST PKBImplementation::getAllProcedures() const {
    return PROCEDURE_LIST();
}

STATEMENT_NUMBER_LIST PKBImplementation::getAncestors(STATEMENT_NUMBER s) const {
    return STATEMENT_NUMBER_LIST();
}

STATEMENT_NUMBER_LIST PKBImplementation::getStatementsThatHaveAncestors() const {
    return STATEMENT_NUMBER_LIST();
}

STATEMENT_NUMBER_LIST PKBImplementation::getDescendants(STATEMENT_NUMBER statementNumber) const {
    return STATEMENT_NUMBER_LIST();
}

STATEMENT_NUMBER_LIST PKBImplementation::getStatementsThatHaveDescendants() const {
    return STATEMENT_NUMBER_LIST();
}

// TODO(weineng) optimize in the future.
STATEMENT_NUMBER_LIST PKBImplementation::getStatementsThatFollows(STATEMENT_NUMBER s) const {
    STATEMENT_NUMBER_LIST result;
    auto it = followedFollowRelation.find(s);
    while (it != followedFollowRelation.end()) {
        int next = it->second;
        result.push_back(next);
        it = followedFollowRelation.find(next);
    }
    return result;
}

// TODO(weineng) optimize in the future.
STATEMENT_NUMBER_LIST PKBImplementation::getStatementsFollowedBy(STATEMENT_NUMBER s) const {
    STATEMENT_NUMBER_LIST result;
    auto it = followFollowedRelation.find(s);
    while (it != followFollowedRelation.end()) {
        int next = it->second;
        result.push_back(next);
        it = followFollowedRelation.find(next);
    }
    return result;
}

STATEMENT_NUMBER_LIST PKBImplementation::getAllStatementsThatFollows() const {
    return allStatementsThatFollows;
}

STATEMENT_NUMBER_LIST PKBImplementation::getAllStatementsThatAreFollowed() const {
    return allStatementsThatAreFollowed;
}

STATEMENT_NUMBER_LIST PKBImplementation::getPreFollows(STATEMENT_NUMBER s) const {
    return STATEMENT_NUMBER_LIST();
}

STATEMENT_NUMBER_LIST PKBImplementation::getPostFollows(STATEMENT_NUMBER s) const {
    return STATEMENT_NUMBER_LIST();
}

} // namespace backend
