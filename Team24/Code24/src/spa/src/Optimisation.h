#pragma once

#include "QPTypes.h"

namespace qpbackend {
namespace optimisation {

// Note: if any is invalid: return empty vector.
std::vector<std::vector<CLAUSE_LIST>>
optimizeQueries(const CLAUSE_LIST& clauses, const RETURN_CANDIDATE_LIST& returnCandidate);
} // namespace optimisation
} // namespace qpbackend
