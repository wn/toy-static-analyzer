#pragma once

#include "TNode.h"

#include <unordered_map>


namespace backend {
namespace extractor {

std::unordered_map<TNode, int> getTNodeToStatementNumber(const TNode& ast);
std::unordered_map<int, TNode> getStatementNumberToTNode(const std::unordered_map<TNode, int>& tNodeToStatementNumber);

} // namespace extractor
} // namespace backend
