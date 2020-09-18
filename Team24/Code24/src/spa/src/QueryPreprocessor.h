#pragma once

#include "Lexer.h"
#include "Query.h"

#include <vector>

namespace querypreprocessor {

typedef backend::lexer::Token TOKEN;
typedef std::vector<TOKEN> TOKENS;

/**
 * Parses tokens of a QPL query into a Query struct for easier processing.
 *
 * @return A Query struct representing the valid QPL query. If the QPL query is invalid, return an
 * empty Query struct.
 */
qpbackend::Query parseTokens(const TOKENS& tokens);

} // namespace querypreprocessor
