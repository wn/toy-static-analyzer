#include "Query.h"

#include <map>
#include <stdexcept>
#include <string>

namespace qpbackend {

const std::map<std::string, EntityType> gEntityStringToTypeMap = {
    { "stmt", EntityType::STMT },         { "read", EntityType::READ },
    { "print", EntityType::PRINT },       { "call", EntityType::CALL },
    { "while", EntityType::WHILE },       { "if", EntityType::IF },
    { "assign", EntityType::ASSIGN },     { "variable", EntityType::VARIABLE },
    { "constant", EntityType::CONSTANT }, { "procedure", EntityType::PROCEDURE },
};

EntityType entityTypeFromString(const std::string& entityString) {
    auto result = gEntityStringToTypeMap.find(entityString);
    if (result == gEntityStringToTypeMap.end()) {
        throw std::invalid_argument(entityString + " does not map to any EntityType.");
    }
    return result->second;
}

bool Query::operator==(const Query& s) const {
    return declarationMap == s.declarationMap && synonymsToReturn == s.synonymsToReturn &&
           suchThatClauses == s.suchThatClauses && patternClauses == s.patternClauses;
}


Query::Query(const std::unordered_map<std::string, EntityType>& declarationMap,
             const std::vector<std::string>& synonymsToReturn,
             const std::vector<RELATIONTUPLE>& suchThatClauses,
             const std::vector<PATTERNTUPLE>& patternClauses) {
    this->declarationMap = declarationMap;
    this->synonymsToReturn = synonymsToReturn;
    this->suchThatClauses = suchThatClauses;
    this->patternClauses = patternClauses;
}

} // namespace qpbackend
