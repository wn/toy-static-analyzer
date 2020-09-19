#include "Query.h"

#include <algorithm>
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

const std::vector<std::pair<std::string, RelationType>> kRelationStringRelationTypePairs = {
    { "Follows", RelationType::FOLLOWS }, { "Follows*", RelationType::FOLLOWST },
    { "Parent", RelationType::PARENT },   { "Parent*", RelationType::PARENTT },
    { "Uses", RelationType::USES },       { "Modifies", RelationType::MODIFIES },
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

bool isRelationString(const std::string& string) {
    return std::any_of(kRelationStringRelationTypePairs.begin(), kRelationStringRelationTypePairs.end(),
                       [&](const std::pair<std::string, RelationType>& pair) {
                           return pair.first == string;
                       });
}

RelationType relationTypeFromString(const std::string& relationString) {
    for (const auto& relationStringRelationTypePair : kRelationStringRelationTypePairs) {
        if (relationStringRelationTypePair.first == relationString) {
            return relationStringRelationTypePair.second;
        }
    }
    throw std::invalid_argument("Error:relationTypeFromString: " + relationString + " does not map to any EntityType.");
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
