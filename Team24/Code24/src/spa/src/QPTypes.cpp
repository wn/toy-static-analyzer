#include "QPTypes.h"

#include <iostream>
#include <sstream>
#include <string>

namespace qpbackend {
std::string prettyReturnCandidate(const RETURN_CANDIDATE& returnCandidate) {
    std::string prettyString = "< ";
    switch (returnCandidate.first) {
    case PROC_PROC_NAME: {
        prettyString += "PROC_PROC_NAME";
        break;
    }
    case CALL_PROC_NAME: {
        prettyString += "CALL_PROC_NAME";
        break;
    }
    case VAR_VAR_NAME: {
        prettyString += "VAR_VAR_NAME";
        break;
    }
    case READ_VAR_NAME: {
        prettyString += "READ_VAR_NAME";
        break;
    }
    case PRINT_VAR_NAME: {
        prettyString += "PRINT_VAR_NAME";
        break;
    }
    case CONSTANT_VALUE: {
        prettyString += "CONSTANT_VALUE";
        break;
    }
    case STMT_STMT_NO: {
        prettyString += "STMT_STMT_NO";
        break;
    }
    case READ_STMT_NO: {
        prettyString += "READ_STMT_NO";
        break;
    }
    case PRINT_STMT_NO: {
        prettyString += "PRINT_STMT_NO";
        break;
    }
    case CALL_STMT_NO: {
        prettyString += "CALL_STMT_NO";
        break;
    }
    case WHILE_STMT_NO: {
        prettyString += "WHILE_STMT_NO";
        break;
    }
    case IF_STMT_NO: {
        prettyString += "IF_STMT_NO";
        break;
    }
    case ASSIGN_STMT_NO: {
        prettyString += "ASSIGN_STMT_NO";
        break;
    }
    case PROG_LINE: {
        prettyString += "PROG_LINE";
        break;
    }
    case BOOLEAN: {
        prettyString += "BOOLEAN";
        break;
    }
    case INVALID_RETURN_TYPE: {
        prettyString += "INVALID_RETURN_TYPE";
        break;
    }
    }
    prettyString += ", ";
    prettyString += returnCandidate.second;
    prettyString += " >";
    return prettyString;
}

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
    case CALL_TO_PROC_SYNONYM:
        return "CALL_TO_PROC_SYNONYM";
    case READ_TO_VAR_SYNONYM:
        return "READ_TO_VAR_SYNONYM";
    case PRINT_TO_VAR_SYNONYM:
        return "PRINT_TO_VAR_SYNONYM";
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
        arg_type_1 == CONST_SYNONYM || arg_type_1 == CALL_TO_PROC_SYNONYM ||
        arg_type_1 == READ_TO_VAR_SYNONYM || arg_type_1 == PRINT_TO_VAR_SYNONYM) {
        switch (arg_type_2) {
        case STMT_SYNONYM:
        case VAR_SYNONYM:
        case PROC_SYNONYM:
        case CONST_SYNONYM:
        case CALL_TO_PROC_SYNONYM:
        case READ_TO_VAR_SYNONYM:
        case PRINT_TO_VAR_SYNONYM:
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
        case CALL_TO_PROC_SYNONYM:
        case READ_TO_VAR_SYNONYM:
        case PRINT_TO_VAR_SYNONYM:
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
std::string prettyPrintClauseType(ClauseType t) {
    switch (t) {
    case FOLLOWS:
        return "Follows";
    case FOLLOWST:
        return "Follows*";
    case PARENT:
        return "Parent";
    case PARENTT:
        return "Parent*";
    case USES:
        return "Uses";
    case MODIFIES:
        return "Modifies";
    case CALLS:
        return "Calls";
    case CALLST:
        return "Calls*";
    case NEXT:
        return "Next";
    case NEXTT:
        return "Next*";
    case AFFECTS:
        return "Affects";
    case AFFECTST:
        return "Affects*";
    case ASSIGN_PATTERN_EXACT:
        return "ASSIGN_PATTERN_EXACT";
    case ASSIGN_PATTERN_SUB_EXPR:
        return "ASSIGN_PATTERN_SUB_EXPR";
    case ASSIGN_PATTERN_WILDCARD:
        return "ASSIGN_PATTERN_WILDCARD";
    case IF_PATTERN:
        return "IF_PATTERN";
    case WHILE_PATTERN:
        return "WHILE_PATTERN";
    case WITH:
        return "WITH";
    case INVALID_CLAUSE_TYPE:
        return "INVALID_CLAUSE_TYPE";
    };
}
std::string prettyPrintCLAUSE(const CLAUSE& clause) {
    std::string pattern = std::get<3>(clause);
    return prettyPrintClauseType(std::get<0>(clause)) + "(" + prettyPrintArg(std::get<1>(clause)) +
           ", " + prettyPrintArg(std::get<2>(clause)) + (pattern.empty() ? "" : ", " + pattern) + ")";
}

std::ostream& operator<<(std::ostream& os, const CLAUSE& value) {
    os << prettyPrintCLAUSE(value);
    return os;
}

std::ostream& operator<<(std::ostream& os, const CLAUSE_LIST& value) {
    if (value.size() == 0) {
        return os;
    }
    for (int i = 0; i < value.size() - 1; ++i) {
        os << value[i] << ", ";
    }
    os << value.back();
    return os;
}

std::ostream& operator<<(std::ostream& os, const std::vector<std::vector<CLAUSE_LIST>>& value) {
    for (int i = 0; i < value.size(); ++i) {
        os << "Group " << i + 1 << std::endl;
        const std::vector<CLAUSE_LIST> v = value[i];
        if (v.size() == 0) {
            continue;
        }
        for (int j = 0; j < v.size() - 1; ++j) {
            os << j + 1 << ": " << v[j] << std::endl;
        }
        os << v.size() << ": " << v.back() << std::endl << std::endl;
    }
    return os;
}

} // namespace qpbackend
