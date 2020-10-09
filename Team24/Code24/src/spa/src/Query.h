#ifndef QUERY_H
#define QUERY_H

#include "QPTypes.h"

#include <string>
#include <vector>

namespace qpbackend {

struct Query {
  public:
    DECLARATION_MAP declarationMap;
    std::vector<std::string> synonymsToReturn;
    std::vector<RELATIONTUPLE> suchThatClauses;
    std::vector<PATTERNTUPLE> patternClauses;

    explicit Query() : declarationMap(), synonymsToReturn(), suchThatClauses(), patternClauses(){};
    Query(const DECLARATION_MAP& declarationMap,
          const std::vector<std::string>& synonymsToReturn,
          const std::vector<RELATIONTUPLE>& suchThatClauses,
          const CLAUSE_LIST& patternClauses);

    bool operator==(const Query& s) const;
    std::string toString() const;
};

// Allow for Query struct expansion in Catch framework's error message generation.
std::ostream& operator<<(std::ostream& os, Query const& value);

bool isEntityString(const std::string& string);

EntityType entityTypeFromString(const std::string& entityString);

bool isRelationClauseString(const std::string& string);

ClauseType relationClauseTypeFromString(const std::string& relationClauseString);

} // namespace qpbackend
#endif // QUERY_H
