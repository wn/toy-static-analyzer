#include "QEHelper.h"

#include <algorithm>

namespace qpbackend {
namespace queryevaluator {
bool isWildCard(std::string const& str) {
    return str == "_";
}

bool isPosInt(const std::string& str) {
    return !str.empty() && (str[0] != '0') && std::find_if(str.begin(), str.end(), [](unsigned char c) {
                                                  return !std::isdigit(c);
                                              }) == str.end();
}

bool isName(const std::string& str) {
    if (str.size() < 3) {
        return false;
    } else if (!((str.front() == '\"') && (str.back() == '\"'))) {
        return false;
    } else {
        std::string substr = str.substr(1, str.size() - 2);
        return isalpha(substr.front()) && std::find_if(substr.begin(), substr.end(), [](unsigned char c) {
                                              return !(std::isdigit(c) || std::isalpha(c));
                                          }) == substr.end();
    }
}

std::tuple<bool, std::string, bool> extractPatternExpr(const std::string& str) {
    // wildcard case
    if (isWildCard(str)) {
        return { true, "", true };
    }
    // subexpression case
    if (str.size() > 4 && str[0] == '_' && str[1] == '\"' && str[str.size() - 1] == '_' &&
        str[str.size() - 2] == '\"') {
        return { true, str.substr(2, str.size() - 4), true };
    }
    // non-subexpression case
    if (str.size() > 2 && str[0] == '\"' && str[str.size() - 1] == '\"') {
        return { true, str.substr(1, str.size() - 2), false };
    }
    // unrecognized pattern
    return { false, "", false };
}

std::string extractQuotedStr(const std::string& str) {
    if (str.size() < 3 || str.front() != '\"' || str.back() != '\"') {
        return str;
    } else {
        return str.substr(1, str.size() - 2);
    }
}

SRT_LOOKUP_TABLE generateSrtTable() {
    SRT_LOOKUP_TABLE srt_table = {
        { FOLLOWS,
          { { STMT_SYNONYM, { { STMT_SYNONYM, PREFOLLOWS }, { NUM_ENTITY, POSTFOLLOWS }, { WILDCARD, PREFOLLOWS_WILD } } },
            { NUM_ENTITY, { { STMT_SYNONYM, PREFOLLOWS }, { NUM_ENTITY, PREFOLLOWS }, { WILDCARD, PREFOLLOWS_WILD } } },
            { WILDCARD, { { STMT_SYNONYM, POSTFOLLOWS_WILD }, { NUM_ENTITY, POSTFOLLOWS_WILD }, { WILDCARD, PREFOLLOWS_WILD } } } } },
        { FOLLOWST,
          { { STMT_SYNONYM, { { STMT_SYNONYM, PREFOLLOWST }, { NUM_ENTITY, POSTFOLLOWST }, { WILDCARD, PREFOLLOWS_WILD } } },
            { NUM_ENTITY, { { STMT_SYNONYM, PREFOLLOWST }, { NUM_ENTITY, PREFOLLOWST }, { WILDCARD, PREFOLLOWS_WILD } } },
            { WILDCARD, { { STMT_SYNONYM, POSTFOLLOWS_WILD }, { NUM_ENTITY, POSTFOLLOWS_WILD }, { WILDCARD, PREFOLLOWS_WILD } } } } },
        { PARENT,
          { { STMT_SYNONYM, { { STMT_SYNONYM, PREPARENT }, { NUM_ENTITY, POSTPARENT }, { WILDCARD, PREPARENT_WILD } } },
            { NUM_ENTITY, { { STMT_SYNONYM, PREPARENT }, { NUM_ENTITY, PREPARENT }, { WILDCARD, PREPARENT_WILD } } },
            { WILDCARD, { { STMT_SYNONYM, POSTPARENT_WILD }, { NUM_ENTITY, POSTPARENT_WILD }, { WILDCARD, PREPARENT_WILD } } } } },
        { PARENTT,
          { { STMT_SYNONYM, { { STMT_SYNONYM, PREPARENTT }, { NUM_ENTITY, POSTPARENTT }, { WILDCARD, PREPARENT_WILD } } },
            { NUM_ENTITY, { { STMT_SYNONYM, PREPARENTT }, { NUM_ENTITY, PREPARENTT }, { WILDCARD, PREPARENT_WILD } } },
            { WILDCARD, { { STMT_SYNONYM, POSTPARENT_WILD }, { NUM_ENTITY, POSTPARENT_WILD }, { WILDCARD, PREPARENT_WILD } } } } },
        { USES,
          { { STMT_SYNONYM, { { VAR_SYNONYM, PREUSESS }, { NAME_ENTITY, POSTUSESS }, { WILDCARD, USES_WILDCARD } } },
            { PROC_SYNONYM, { { VAR_SYNONYM, PREUSESP }, { NAME_ENTITY, POSTUSESP }, { WILDCARD, USEP_WILDCARD } } },
            { NUM_ENTITY, { { VAR_SYNONYM, PREUSESS }, { NAME_ENTITY, PREUSESS }, { WILDCARD, USES_WILDCARD } } },
            { NAME_ENTITY, { { VAR_SYNONYM, PREUSESP }, { NAME_ENTITY, PREUSESP }, { WILDCARD, USEP_WILDCARD } } } } },
        { MODIFIES,
          { { STMT_SYNONYM, { { VAR_SYNONYM, PREMODIFIESS }, { NAME_ENTITY, POSTMODIFIESS }, { WILDCARD, MODIFIESS_WILDCARD } } },
            { PROC_SYNONYM, { { VAR_SYNONYM, PREMODIFIESP }, { NAME_ENTITY, POSTMODIFIESP }, { WILDCARD, MODIFIESP_WILDCARD } } },
            { NUM_ENTITY, { { VAR_SYNONYM, PREMODIFIESS }, { NAME_ENTITY, PREMODIFIESS }, { WILDCARD, MODIFIESS_WILDCARD } } },
            { NAME_ENTITY, { { VAR_SYNONYM, PREMODIFIESP }, { NAME_ENTITY, PREMODIFIESP }, { WILDCARD, MODIFIESP_WILDCARD } } } } },
        { NEXT,
          { { STMT_SYNONYM, { { STMT_SYNONYM, PRENEXT }, { NUM_ENTITY, POSTNEXT }, { WILDCARD, PRENEXT_WILD } } },
            { NUM_ENTITY, { { STMT_SYNONYM, PRENEXT }, { NUM_ENTITY, PRENEXT }, { WILDCARD, PRENEXT_WILD } } },
            { WILDCARD, { { STMT_SYNONYM, POSTNEXT_WILD }, { NUM_ENTITY, POSTNEXT_WILD }, { WILDCARD, PRENEXT_WILD } } } } },
        { NEXTT,
          { { STMT_SYNONYM, { { STMT_SYNONYM, PRENEXTT }, { NUM_ENTITY, POSTNEXTT }, { WILDCARD, PRENEXT_WILD } } },
            { NUM_ENTITY, { { STMT_SYNONYM, PRENEXTT }, { NUM_ENTITY, PRENEXTT }, { WILDCARD, PRENEXT_WILD } } },
            { WILDCARD, { { STMT_SYNONYM, POSTNEXT_WILD }, { NUM_ENTITY, POSTNEXT_WILD }, { WILDCARD, PRENEXT_WILD } } } } },
        { ASSIGN_PATTERN_EXACT,
          { { VAR_SYNONYM, { { STMT_SYNONYM, ASSIGN_PATTERN_EXACT_SRT } } },
            { NAME_ENTITY, { { STMT_SYNONYM, ASSIGN_PATTERN_EXACT_SRT } } },
            { WILDCARD, { { STMT_SYNONYM, ASSIGN_PATTERN_EXACT_SRT } } } } },
        { ASSIGN_PATTERN_SUB_EXPR,
          { { VAR_SYNONYM, { { STMT_SYNONYM, ASSIGN_PATTERN_SUBEXPR_SRT } } },
            { NAME_ENTITY, { { STMT_SYNONYM, ASSIGN_PATTERN_SUBEXPR_SRT } } },
            { WILDCARD, { { STMT_SYNONYM, ASSIGN_PATTERN_SUBEXPR_SRT } } } } },
        { ASSIGN_PATTERN_WILDCARD,
          { { VAR_SYNONYM, { { STMT_SYNONYM, ASSIGN_PATTERN_WILDCARD_SRT } } },
            { NAME_ENTITY, { { STMT_SYNONYM, ASSIGN_PATTERN_WILDCARD_SRT } } },
            { WILDCARD, { { STMT_SYNONYM, ASSIGN_PATTERN_WILDCARD_SRT } } } } }
    };

    return srt_table;
}
} // namespace queryevaluator
} // namespace qpbackend
