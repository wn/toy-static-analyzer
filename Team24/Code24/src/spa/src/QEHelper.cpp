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

SRT_LOOKUP_TABLE generateSrtTable() {
    SRT_LOOKUP_TABLE srt_table = {
        { FOLLOWS,
          { { SYNONYM, { { SYNONYM, PREFOLLOWS }, { NUM_ENTITY, POSTFOLLOWS }, { NAME_ENTITY, INVALID }, { WILDCARD, PREFOLLOWS_WILD } } },
            { NUM_ENTITY, { { SYNONYM, PREFOLLOWS }, { NUM_ENTITY, PREFOLLOWS }, { NAME_ENTITY, INVALID }, { WILDCARD, PREFOLLOWS_WILD } } },
            { NAME_ENTITY, { { SYNONYM, INVALID }, { NUM_ENTITY, INVALID }, { NAME_ENTITY, INVALID }, { WILDCARD, INVALID } } },
            { WILDCARD,
              { { SYNONYM, POSTFOLLOWS_WILD }, { NUM_ENTITY, POSTFOLLOWS_WILD }, { NAME_ENTITY, INVALID }, { WILDCARD, PREFOLLOWS_WILD } } } } },
        { FOLLOWST,
          { { SYNONYM, { { SYNONYM, PREFOLLOWST }, { NUM_ENTITY, POSTFOLLOWST }, { NAME_ENTITY, INVALID }, { WILDCARD, PREFOLLOWS_WILD } } },
            { NUM_ENTITY, { { SYNONYM, PREFOLLOWST }, { NUM_ENTITY, PREFOLLOWST }, { NAME_ENTITY, INVALID }, { WILDCARD, PREFOLLOWS_WILD } } },
            { NAME_ENTITY, { { SYNONYM, INVALID }, { NUM_ENTITY, INVALID }, { NAME_ENTITY, INVALID }, { WILDCARD, INVALID } } },
            { WILDCARD,
              { { SYNONYM, POSTFOLLOWS_WILD }, { NUM_ENTITY, POSTFOLLOWS_WILD }, { NAME_ENTITY, INVALID }, { WILDCARD, PREFOLLOWS_WILD } } } } }
    };

    return srt_table;
}
} // namespace queryevaluator
} // namespace qpbackend
