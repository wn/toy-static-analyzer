#pragma once

#include "Parser.h"

namespace backend {
namespace testhelpers {

Parser GenerateParserFromTokens(const std::string& expr);
TNode generateProgramNodeFromStatement(const std::string& name, const TNode& node);
TNode generateMockAssignNode();
TNode generateProgramNodeFromCondition(const TNode& node);

} // namespace testhelpers
} // namespace backend