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

std::unordered_map<const TNode*, std::vector<const TNode*>>
getProcedureToCallers(std::unordered_map<TNodeType, std::vector<const TNode*>, EnumClassHash>& tNodeTypeToTNodes);

const TNode*
getProcedureFromProcedureName(const std::string& procedureName,
                              std::unordered_map<TNodeType, std::vector<const TNode*>, EnumClassHash>& tNodeTypeToTNodes);

/**
 * Returns one possible Topological ordering of the "called-by" graph.
 *
 * The call graph of a SIMPLE program is defined as the procedures being the vertices,
 * and a directed edge indicating that one procedure calls another. The edge (p,q) means that
 * procedure p contains at least one statement "call q;".
 *
 * The "called by" graph is the reverse graph of the call graph, meaning that the edge (q,p)
 * exists instead of (p,q). This edge indicates that q was _called by_ p.
 *
 * @param tNodeTypeToTNodes
 * @return a topological ordering of the "called-by" graph. The guarantees that when iterating
 * through procedure k, all procedures called by procedure k have been iterated over.
 */
std::vector<const TNode*> getTopologicalOrderingOfCalledByGraph(
std::unordered_map<TNodeType, std::vector<const TNode*>, EnumClassHash>& tNodeTypeToTNodes);

std::unordered_map<const TNode*, int> getTNodeToStatementNumber(const TNode& ast);
std::unordered_map<int, const TNode*>
getStatementNumberToTNode(const std::unordered_map<const TNode*, int>& tNodeToStatementNumber);
std::unordered_map<TNodeType, std::vector<const TNode*>, EnumClassHash> getTNodeTypeToTNodes(const TNode& ast);

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
} // namespace extractor
} // namespace backend
