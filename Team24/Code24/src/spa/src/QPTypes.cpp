#include "QPTypes.h"

#include <sstream>
#include <string>

namespace qpbackend {
std::string prettyPrintArgType(ArgType argType) {
    switch (argType) {
    case STMT_SYNONYM:
        return "STMT_SYNONYM";
    case VAR_SYNONYM:
        return "VAR_SYNONYM";
    case PROC_SYNONYM:
        return "PROC_SYNONYM";
    case CONST_SYNONYM:
        return "CONST_SYNONYM";
    case NAME_ENTITY:
        return "NAME_ENTITY";
    case NUM_ENTITY:
        return "NUM_ENTITY";
    case WILDCARD:
        return "WILDCARD";
    case INVALID_ARG:
        return "INVALID_ARG";
    }
}

std::string prettyPrintArg(const ARG& arg) {
    std::stringstream stringstream;
    stringstream << "<" << prettyPrintArgType(arg.first) << ", " << arg.second << '>';
    return stringstream.str();
}

ClauseArgsType getClauseArgsType(ArgType arg_type_1, ArgType arg_type_2) {
    if (arg_type_1 == STMT_SYNONYM || arg_type_1 == VAR_SYNONYM || arg_type_1 == PROC_SYNONYM ||
        arg_type_1 == CONST_SYNONYM) {
        switch (arg_type_2) {
        case STMT_SYNONYM:
        case VAR_SYNONYM:
        case PROC_SYNONYM:
        case CONST_SYNONYM:
            return SynonymSynonym;
        case NUM_ENTITY:
        case NAME_ENTITY:
            return SynonymEntity;
        case WILDCARD:
            return SynonymWildcard;
        default:
            return Invalid2;
        }
    } else if (arg_type_1 == NUM_ENTITY || arg_type_1 == NAME_ENTITY) {
        switch (arg_type_2) {
        case STMT_SYNONYM:
        case VAR_SYNONYM:
        case PROC_SYNONYM:
        case CONST_SYNONYM:
            return EntitySynonym;
        case NUM_ENTITY:
        case NAME_ENTITY:
            return EntityEntity;
        case WILDCARD:
            return EntityWildcard;
        default:
            return Invalid2;
        }
    } else if (arg_type_1 == WILDCARD) {
        switch (arg_type_2) {
        case STMT_SYNONYM:
        case VAR_SYNONYM:
        case PROC_SYNONYM:
        case CONST_SYNONYM:
            return WildcardSynonym;
        case NUM_ENTITY:
        case NAME_ENTITY:
            return WildcardEntity;
        case WILDCARD:
            return WildcardWildcard;
        default:
            return Invalid2;
        }
    } else {
        return Invalid1;
    }
}

} // namespace qpbackend
