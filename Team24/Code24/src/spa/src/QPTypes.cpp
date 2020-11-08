#include "QPTypes.h"

#include <iostream>
#include <sstream>
#include <string>

namespace qpbackend {
std::string prettyPrintReturnType(ReturnType returnType) {
    switch (returnType) {
    case PROC_NAME: {
        return "PROC_NAME";
    }
    case VAR_NAME: {
        return "VAR_NAME";
    }
    case CONST_VALUE: {
        return "CONST_VALUE";
    }
    case STMT_NO: {
        return "STMT_NO";
    }
    case DEFAULT_VAL: {
        return "DEFAULT_VAL";
    }
    case BOOLEAN: {
        return "BOOLEAN";
    }
    case INVALID_RETURN_TYPE: {
        return "INVALID_RETURN_TYPE";
    }
    }
}

std::string prettyReturnCandidate(const RETURN_CANDIDATE& returnCandidate) {
    std::string prettyString = "< ";
    prettyString += prettyPrintReturnType(returnCandidate.first);
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

std::string prettyPrintAttrArg(const ATTR_ARG& attrArg) {
    std::stringstream stringstream;
    stringstream << "<" << prettyPrintArgType(std::get<0>(attrArg)) << ", "
                 << prettyPrintReturnType(std::get<1>(attrArg)) << ", " << std::get<2>(attrArg) << '>';
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
            return SYNONYM_SYNONYM;
        case NUM_ENTITY:
        case NAME_ENTITY:
            return SYNONYM_ENTITY;
        case WILDCARD:
            return SYNONYM_WILDCARD;
        default:
            return INVALID_2;
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
            return ENTITY_SYNONYM;
        case NUM_ENTITY:
        case NAME_ENTITY:
            return ENTITY_ENTITY;
        case WILDCARD:
            return ENTITY_WILDCARD;
        default:
            return INVALID_2;
        }
    } else if (arg_type_1 == WILDCARD) {
        switch (arg_type_2) {
        case STMT_SYNONYM:
        case VAR_SYNONYM:
        case PROC_SYNONYM:
        case CONST_SYNONYM:
            return WILDCARD_SYNONYM;
        case NUM_ENTITY:
        case NAME_ENTITY:
            return WILDCARD_ENTITY;
        case WILDCARD:
            return WILDCARD_WILDCARD;
        default:
            return INVALID_2;
        }
    } else {
        return INVALID_1;
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
    case NEXTBIP:
        return "NextBip";
    case NEXTBIPT:
        return "NextBip*";
    case AFFECTSBIP:
        return "AffectsBip";
    case AFFECTSBIPT:
        return "AffectsBip*";
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

std::string prettyPrintWithClause(const WITH_CLAUSE& withClause) {
    return "WITH: (" + prettyPrintAttrArg(std::get<0>(withClause)) + ", " +
           prettyPrintAttrArg(std::get<1>(withClause)) + ")";
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
