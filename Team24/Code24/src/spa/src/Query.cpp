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
} // namespace qpbackend
