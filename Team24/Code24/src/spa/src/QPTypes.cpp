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

} // namespace qpbackend
