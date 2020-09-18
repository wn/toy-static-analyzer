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
std::vector<int> getValuesInMap(const std::unordered_map<int, int>& followedFollowRelation);
} // namespace extractor
} // namespace backend
