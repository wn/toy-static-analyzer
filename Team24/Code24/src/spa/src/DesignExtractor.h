#pragma once

#include "TNode.h"

#include <unordered_map>
#include <unordered_set>

namespace backend {
namespace extractor {

/**
 * Returns the Uses mapping of the program.
 * @param tNodeTypeToTNodes
 * @return a Map where the TNode keys are in the domain of the Uses relation, and they map to
 * variables used by the TNode.
 */
std::unordered_map<const TNode*, std::unordered_set<std::string>>
getUsesMapping(std::unordered_map<TNodeType, std::vector<const TNode*>, EnumClassHash>& tNodeTypeToTNodes);

/**
 * Returns the Modifies mapping of the program.
 * @param tNodeTypeToTNodes
 * @return a Map where the TNode keys are in the domain of the Modifies relation, and they map to
 * variables modified by the TNode.
 */
std::unordered_map<const TNode*, std::unordered_set<std::string>>
getModifiesMapping(std::unordered_map<TNodeType, std::vector<const TNode*>, EnumClassHash>& tNodeTypeToTNodes);

std::unordered_map<const TNode*, std::unordered_set<const TNode*>>
getProcedureToCallees(const std::unordered_map<TNodeType, std::vector<const TNode*>, EnumClassHash>& tNodeTypeToTNodes);

const TNode* getProcedureFromProcedureName(
const std::string& procedureName,
const std::unordered_map<TNodeType, std::vector<const TNode*>, EnumClassHash>& tNodeTypeToTNodes);

std::unordered_map<const TNode*, int> getTNodeToStatementNumber(const TNode& ast);
std::unordered_map<int, const TNode*>
getStatementNumberToTNode(const std::unordered_map<const TNode*, int>& tNodeToStatementNumber);
std::unordered_map<TNodeType, std::vector<const TNode*>, EnumClassHash> getTNodeTypeToTNodes(const TNode& ast);

bool isValidSimpleProgram(const TNode& ast);

// Return a pair of mapping {{follower : followed}, {followed, follower}}.
std::pair<std::unordered_map<int, int>, std::unordered_map<int, int>> getFollowRelationship(const TNode& ast);
// Return a pair of mapping {{child : parent}, {parent, children}}.
std::pair<std::unordered_map<int, int>, std::unordered_map<int, std::vector<int>>>
getParentRelationship(const TNode& ast);

template <typename T> std::vector<int> getKeysInMap(const std::unordered_map<int, T>& map) {
    std::vector<int> result;
    for (auto it = map.begin(); it != map.end(); ++it) {
        result.push_back(it->first);
    }
    return result;
}

std::vector<int> getVisitedPathFromStart(int start, const std::unordered_map<int, int>& relation);

/**
 * Get a mapping of pattern to {assignee, stmtNumber, isSubExpr}
 *
 * Note:
 * - isSubExpr - boolean of whether the visiting tNode is a sub expression
 * - stmtNo - the statement number of the assignee and expression.
 * - assignee - for each assign statement in the AST, the assignee is the variable on
 *   the left of the assignment statement. i.e. the "a" in a = 1 + 1;
 */
std::unordered_map<std::string, std::vector<std::tuple<std::string, int, bool>>>
getPatternsMap(const std::vector<const TNode*>& assignTNodes,
               const std::unordered_map<const TNode*, int>& tNodeToStatementNumber);

std::unordered_map<int, TNodeType>
getStatementNumberToTNodeTypeMap(const std::unordered_map<int, const TNode*>& statementNumberToTNode);
} // namespace extractor
} // namespace backend
