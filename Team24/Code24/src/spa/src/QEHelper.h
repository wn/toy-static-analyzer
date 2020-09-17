#ifndef QEHELPER_H
#define QEHELPER_H

#include "PKB.h"
#include "Query.h"

#include <algorithm>
#include <string>
#include <utility>
#include <vector>

namespace qpbackend {
namespace queryevaluator {
// methods to check string argument
bool isWildCard(const std::string& str);
bool isStmt(const std::string& str);
bool isPosInt(const std::string& str);

// helper functions for vector operations
template <typename T> std::vector<std::string> vector_str_cast(const std::vector<T>& v);
template <typename T> bool vector_contain(const std::vector<T>& v, T arg);
template <typename T>
std::vector<T> vector_intersection(const std::vector<T>& lst1, const std::vector<T>& lst2);
} // namespace queryevaluator
} // namespace qpbackend

#endif // QEHELPER_H
