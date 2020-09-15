#ifndef QUERY_H
#define QUERY_H

#include <string>
#include <unordered_map>
#include <vector>

namespace qpbackend {
enum EntityType { STMT, READ, PRINT, CALL, WHILE, IF, ASSIGN, VARIABLE, CONSTANT, PROCEDURE };

enum RelationType { FOLLOWS, FOLLOWST, PARENT, PARENTT, USES, MODIFIES };

struct Query {
  public:
    std::unordered_map<std::string, EntityType> declarationMap;
    std::vector<std::string> synonymsToReturn;
    std::vector<std::tuple<RelationType, std::string, std::string>> suchThatClauses;
    std::vector<std::tuple<std::string, std::string, std::string>> patternClauses;

    explicit Query() : declarationMap(), suchThatClauses(), patternClauses(), synonymsToReturn(){};
};

} // namespace qpbackend

#endif // QUERY_H
