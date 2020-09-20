#ifndef QUERY_H
#define QUERY_H

#include <string>
#include <unordered_map>
#include <vector>

namespace qpbackend {
enum EntityType { STMT, READ, PRINT, CALL, WHILE, IF, ASSIGN, VARIABLE, CONSTANT, PROCEDURE };

enum RelationType { FOLLOWS, FOLLOWST, PARENT, PARENTT, USES, MODIFIES };

typedef std::tuple<RelationType, std::string, std::string> RELATIONTUPLE;
typedef std::tuple<std::string, std::string, std::string> PATTERNTUPLE;

struct Query {
  public:
    std::unordered_map<std::string, EntityType> declarationMap;
    std::vector<std::string> synonymsToReturn;
    std::vector<std::tuple<RelationType, std::string, std::string>> suchThatClauses;
    std::vector<std::tuple<std::string, std::string, std::string>> patternClauses;

    explicit Query() : declarationMap(), suchThatClauses(), patternClauses(), synonymsToReturn(){};
    Query(const std::unordered_map<std::string, EntityType>& declarationMap,
          const std::vector<std::string>& synonymsToReturn,
          const std::vector<RELATIONTUPLE>& suchThatClauses,
          const std::vector<PATTERNTUPLE>& patternClauses);

    bool operator==(const Query& s) const;
    std::string toString() const;
};

// Allow for Query struct expansion in Catch framework's error message generation.
std::ostream& operator<<(std::ostream& os, Query const& value);

bool isEntityString(const std::string& string);

EntityType entityTypeFromString(const std::string& entityString);

bool isRelationString(const std::string& string);

RelationType relationTypeFromString(const std::string& relationString);

} // namespace qpbackend
#endif // QUERY_H
