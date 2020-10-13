#ifndef QUERYSHIMS_H
#define QUERYSHIMS_H
#include "QPTypes.h"

namespace qpbackend {
const RETURN_CANDIDATE_LIST convertToReturnValues(const std::vector<std::string>& nameValues,
                                                  const qpbackend::DECLARATION_MAP& declarationMap);
} // namespace qpbackend

#endif // QUERYSHIMS_H
