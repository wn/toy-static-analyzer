#pragma once

#include "TNode.h"

#include <unordered_map>

namespace backend {
namespace extractor {

std::unordered_map<TNode, int> getTNodeToStatementNumber(const TNode& ast);
std::unordered_map<int, TNode> getStatementNumberToTNode(const std::unordered_map<TNode, int>& tNodeToStatementNumber);
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
} // namespace extractor
} // namespace backend
