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

bool isNumArg(ArgType argType) {
    return argType == STMT_SYNONYM || argType == CONST_SYNONYM || argType == NUM_ENTITY;
}

bool isNameArg(ArgType argType) {
    return argType == VAR_SYNONYM || argType == PROC_SYNONYM || argType == NAME_ENTITY ||
           argType == CALL_TO_PROC_SYNONYM || argType == READ_TO_VAR_SYNONYM || argType == PRINT_TO_VAR_SYNONYM;
}

bool needAttrConversion(ArgType argType) {
    return argType == CALL_TO_PROC_SYNONYM || argType == READ_TO_VAR_SYNONYM || argType == PRINT_TO_VAR_SYNONYM;
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
          { { STMT_SYNONYM, { { STMT_SYNONYM, POSTFOLLOWS }, { NUM_ENTITY, POSTFOLLOWS }, { WILDCARD, PREFOLLOWS_WILD } } },
            { NUM_ENTITY, { { STMT_SYNONYM, PREFOLLOWS }, { NUM_ENTITY, PREFOLLOWS }, { WILDCARD, PREFOLLOWS_WILD } } },
            { WILDCARD, { { STMT_SYNONYM, POSTFOLLOWS_WILD }, { NUM_ENTITY, POSTFOLLOWS_WILD }, { WILDCARD, PREFOLLOWS_WILD } } } } },
        { FOLLOWST,
          { { STMT_SYNONYM, { { STMT_SYNONYM, POSTFOLLOWST }, { NUM_ENTITY, POSTFOLLOWST }, { WILDCARD, PREFOLLOWS_WILD } } },
            { NUM_ENTITY, { { STMT_SYNONYM, PREFOLLOWST }, { NUM_ENTITY, PREFOLLOWST }, { WILDCARD, PREFOLLOWS_WILD } } },
            { WILDCARD, { { STMT_SYNONYM, POSTFOLLOWS_WILD }, { NUM_ENTITY, POSTFOLLOWS_WILD }, { WILDCARD, PREFOLLOWS_WILD } } } } },
        { PARENT,
          { { STMT_SYNONYM, { { STMT_SYNONYM, POSTPARENT }, { NUM_ENTITY, POSTPARENT }, { WILDCARD, PREPARENT_WILD } } },
            { NUM_ENTITY, { { STMT_SYNONYM, PREPARENT }, { NUM_ENTITY, PREPARENT }, { WILDCARD, PREPARENT_WILD } } },
            { WILDCARD, { { STMT_SYNONYM, POSTPARENT_WILD }, { NUM_ENTITY, POSTPARENT_WILD }, { WILDCARD, PREPARENT_WILD } } } } },
        { PARENTT,
          { { STMT_SYNONYM, { { STMT_SYNONYM, POSTPARENTT }, { NUM_ENTITY, POSTPARENTT }, { WILDCARD, PREPARENT_WILD } } },
            { NUM_ENTITY, { { STMT_SYNONYM, PREPARENTT }, { NUM_ENTITY, PREPARENTT }, { WILDCARD, PREPARENT_WILD } } },
            { WILDCARD, { { STMT_SYNONYM, POSTPARENT_WILD }, { NUM_ENTITY, POSTPARENT_WILD }, { WILDCARD, PREPARENT_WILD } } } } },
        { USES,
          { { STMT_SYNONYM, { { VAR_SYNONYM, POSTUSESS }, { NAME_ENTITY, POSTUSESS }, { WILDCARD, USES_WILDCARD } } },
            { PROC_SYNONYM, { { VAR_SYNONYM, POSTUSESP }, { NAME_ENTITY, POSTUSESP }, { WILDCARD, USEP_WILDCARD } } },
            { NUM_ENTITY, { { VAR_SYNONYM, PREUSESS }, { NAME_ENTITY, PREUSESS }, { WILDCARD, USES_WILDCARD } } },
            { NAME_ENTITY, { { VAR_SYNONYM, PREUSESP }, { NAME_ENTITY, PREUSESP }, { WILDCARD, USEP_WILDCARD } } } } },
        { MODIFIES,
          { { STMT_SYNONYM, { { VAR_SYNONYM, POSTMODIFIESS }, { NAME_ENTITY, POSTMODIFIESS }, { WILDCARD, MODIFIESS_WILDCARD } } },
            { PROC_SYNONYM, { { VAR_SYNONYM, POSTMODIFIESP }, { NAME_ENTITY, POSTMODIFIESP }, { WILDCARD, MODIFIESP_WILDCARD } } },
            { NUM_ENTITY, { { VAR_SYNONYM, PREMODIFIESS }, { NAME_ENTITY, PREMODIFIESS }, { WILDCARD, MODIFIESS_WILDCARD } } },
            { NAME_ENTITY, { { VAR_SYNONYM, PREMODIFIESP }, { NAME_ENTITY, PREMODIFIESP }, { WILDCARD, MODIFIESP_WILDCARD } } } } },
        { NEXT,
          { { STMT_SYNONYM, { { STMT_SYNONYM, POSTNEXT }, { NUM_ENTITY, POSTNEXT }, { WILDCARD, PRENEXT_WILD } } },
            { NUM_ENTITY, { { STMT_SYNONYM, PRENEXT }, { NUM_ENTITY, PRENEXT }, { WILDCARD, PRENEXT_WILD } } },
            { WILDCARD, { { STMT_SYNONYM, POSTNEXT_WILD }, { NUM_ENTITY, POSTNEXT_WILD }, { WILDCARD, PRENEXT_WILD } } } } },
        { NEXTT,
          { { STMT_SYNONYM, { { STMT_SYNONYM, POSTNEXTT }, { NUM_ENTITY, POSTNEXTT }, { WILDCARD, PRENEXT_WILD } } },
            { NUM_ENTITY, { { STMT_SYNONYM, PRENEXTT }, { NUM_ENTITY, PRENEXTT }, { WILDCARD, PRENEXT_WILD } } },
            { WILDCARD, { { STMT_SYNONYM, POSTNEXT_WILD }, { NUM_ENTITY, POSTNEXT_WILD }, { WILDCARD, PRENEXT_WILD } } } } },
        { AFFECTS,
          { { STMT_SYNONYM, { { STMT_SYNONYM, POSTAFFECTS }, { NUM_ENTITY, POSTAFFECTS }, { WILDCARD, PREAFFECTS_WILD } } },
            { NUM_ENTITY, { { STMT_SYNONYM, PREAFFECTS }, { NUM_ENTITY, PREAFFECTS }, { WILDCARD, PREAFFECTS_WILD } } },
            { WILDCARD, { { STMT_SYNONYM, POSTAFFECTS_WILD }, { NUM_ENTITY, POSTAFFECTS_WILD }, { WILDCARD, PREAFFECTS_WILD } } } } },
        { AFFECTST,
          { { STMT_SYNONYM, { { STMT_SYNONYM, POSTAFFECTST }, { NUM_ENTITY, POSTAFFECTST }, { WILDCARD, PREAFFECTS_WILD } } },
            { NUM_ENTITY, { { STMT_SYNONYM, PREAFFECTST }, { NUM_ENTITY, PREAFFECTST }, { WILDCARD, PREAFFECTS_WILD } } },
            { WILDCARD, { { STMT_SYNONYM, POSTAFFECTS_WILD }, { NUM_ENTITY, POSTAFFECTS_WILD }, { WILDCARD, PREAFFECTS_WILD } } } } },
        { CALLS,
          { { PROC_SYNONYM, { { PROC_SYNONYM, POSTCALLS }, { NAME_ENTITY, POSTCALLS }, { WILDCARD, PRECALL_WILD } } },
            { NAME_ENTITY, { { PROC_SYNONYM, PRECALLS }, { NAME_ENTITY, PRECALLS }, { WILDCARD, PRECALL_WILD } } },
            { WILDCARD, { { PROC_SYNONYM, POSTCALL_WILD }, { NAME_ENTITY, POSTCALL_WILD }, { WILDCARD, PRECALL_WILD } } } } },
        { CALLST,
          { { PROC_SYNONYM, { { PROC_SYNONYM, POSTCALLST }, { NAME_ENTITY, POSTCALLST }, { WILDCARD, PRECALL_WILD } } },
            { NAME_ENTITY, { { PROC_SYNONYM, PRECALLST }, { NAME_ENTITY, PRECALLST }, { WILDCARD, PRECALL_WILD } } },
            { WILDCARD, { { PROC_SYNONYM, POSTCALL_WILD }, { NAME_ENTITY, POSTCALL_WILD }, { WILDCARD, PRECALL_WILD } } } } },
        { ASSIGN_PATTERN_EXACT,
          { { STMT_SYNONYM,
              { { VAR_SYNONYM, ASSIGN_PATTERN_EXACT_SRT },
                { NAME_ENTITY, ASSIGN_PATTERN_EXACT_SRT },
                { WILDCARD, ASSIGN_PATTERN_EXACT_SRT } } } } },
        { ASSIGN_PATTERN_SUB_EXPR,
          { { STMT_SYNONYM,
              { { VAR_SYNONYM, ASSIGN_PATTERN_SUBEXPR_SRT },
                { NAME_ENTITY, ASSIGN_PATTERN_SUBEXPR_SRT },
                { WILDCARD, ASSIGN_PATTERN_SUBEXPR_SRT } } } } },
        { ASSIGN_PATTERN_WILDCARD,
          { { STMT_SYNONYM,
              { { VAR_SYNONYM, ASSIGN_PATTERN_WILDCARD_SRT },
                { NAME_ENTITY, ASSIGN_PATTERN_WILDCARD_SRT },
                { WILDCARD, ASSIGN_PATTERN_WILDCARD_SRT } } } } },
        { IF_PATTERN,
          { { STMT_SYNONYM, { { VAR_SYNONYM, IF_PATTERN_SRT }, { NAME_ENTITY, IF_PATTERN_SRT }, { WILDCARD, IF_PATTERN_SRT } } } } },
        { WHILE_PATTERN,
          { { STMT_SYNONYM, { { VAR_SYNONYM, WHILE_PATTERN_SRT }, { NAME_ENTITY, WHILE_PATTERN_SRT }, { WILDCARD, WHILE_PATTERN_SRT } } } } }
    };

    return srt_table;
}

ATTR_CONVERT_TABLE generateAttrConvertTable() {
    ATTR_CONVERT_TABLE table = {
        { PROG_LINE, { { DEFAULT_VAL, STMT_SYNONYM } } },
        { STMT, { { DEFAULT_VAL, STMT_SYNONYM }, { STMT_NO, STMT_SYNONYM } } },
        { READ, { { DEFAULT_VAL, STMT_SYNONYM }, { STMT_NO, STMT_SYNONYM }, { VAR_NAME, READ_TO_VAR_SYNONYM } } },
        { PRINT, { { DEFAULT_VAL, STMT_SYNONYM }, { STMT_NO, STMT_SYNONYM }, { VAR_NAME, PRINT_TO_VAR_SYNONYM } } },
        { CALL, { { DEFAULT_VAL, STMT_SYNONYM }, { STMT_NO, STMT_SYNONYM }, { PROC_NAME, CALL_TO_PROC_SYNONYM } } },
        { WHILE, { { DEFAULT_VAL, STMT_SYNONYM }, { STMT_NO, STMT_SYNONYM } } },
        { IF, { { DEFAULT_VAL, STMT_SYNONYM }, { STMT_NO, STMT_SYNONYM } } },
        { ASSIGN, { { DEFAULT_VAL, STMT_SYNONYM }, { STMT_NO, STMT_SYNONYM } } },
        { VARIABLE, { { DEFAULT_VAL, VAR_SYNONYM }, { VAR_NAME, VAR_SYNONYM } } },
        { CONSTANT, { { DEFAULT_VAL, CONST_SYNONYM }, { CONST_VALUE, CONST_SYNONYM } } },
        { PROCEDURE, { { DEFAULT_VAL, PROC_SYNONYM }, { PROC_NAME, PROC_SYNONYM } } }
    };

    return table;
}

std::string assignSynonymToAttribute(std::string syn, ReturnType rt) {
    return syn + "_" + std::to_string(rt);
}

std::string tupleToStr(const std::vector<std::string>& t) {
    if (t.size() == 0) {
        return "";
    }
    if (t.size() == 1) {
        return t[0];
    }

    std::string result = t[0];
    for (size_t idx = 1; idx < t.size(); idx++) {
        result += " ";
        result += t[idx];
    }
    return result;
}


} // namespace queryevaluator
} // namespace qpbackend
