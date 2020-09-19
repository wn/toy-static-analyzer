#include "Query.h"

#include <map>
#include <stdexcept>
#include <string>

namespace qpbackend {

const std::map<std::string, EntityType> kEntityStringToTypeMap = {
    { "stmt", EntityType::STMT },         { "read", EntityType::READ },
    { "print", EntityType::PRINT },       { "call", EntityType::CALL },
    { "while", EntityType::WHILE },       { "if", EntityType::IF },
    { "assign", EntityType::ASSIGN },     { "variable", EntityType::VARIABLE },
    { "constant", EntityType::CONSTANT }, { "procedure", EntityType::PROCEDURE },
};

bool isEntityString(const std::string& string) {
    return kEntityStringToTypeMap.find(string) != kEntityStringToTypeMap.end();
}

EntityType entityTypeFromString(const std::string& entityString) {
    auto result = kEntityStringToTypeMap.find(entityString);
    if (result == kEntityStringToTypeMap.end()) {
        throw std::invalid_argument("Error:entityTypeFromString: " + entityString + " does not map to any EntityType.");
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

// TODO(https://github.com/nus-cs3203/team24-cp-spa-20s1/issues/157):
// Complete this implementation for better debugging.
std::string Query::toString() const {
    std::string s = "Query {\nDeclaration map:";

    for (const auto& kv : declarationMap) {
        s += kv.first + " ";
    }

    s += "\nSynonyms to return:";
    for (const auto& synonym : synonymsToReturn) {
        s += synonym + " ";
    }
    s += "\n}";
    return s;
}


} // namespace qpbackend
