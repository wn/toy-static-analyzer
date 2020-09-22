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

// TODO: confirm the format of expression
bool isExpression(const std::string& str) {
    return false;
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
            { NAME_ENTITY, { { VAR_SYNONYM, PREUSESP }, { NAME_ENTITY, PREUSESP }, { WILDCARD, USEP_WILDCARD } } } } }
    };

    return srt_table;
}
} // namespace queryevaluator
} // namespace qpbackend
