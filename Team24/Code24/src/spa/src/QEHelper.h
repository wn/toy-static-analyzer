#ifndef QEHELPER_H
#define QEHELPER_H

#include "Query.h"

#include <string>
#include <unordered_map>

namespace qpbackend {
namespace queryevaluator {
// helper relation type derived from given relation type, used for evaluation
enum SubRelationType {
    PREFOLLOWS, // check upon a1, get a2 that Follows(a1, a2)
    POSTFOLLOWS, // check upon a2, get a1 that Follows(a1, a2)
    PREFOLLOWST, // check upon a1, get a2 that Follows*(a1, a2)
    POSTFOLLOWST, // check upon a2, get a1 that Follows*(a1, a2)
    PREFOLLOWS_WILD, // check upon a1, get Follows(a1, _)
    POSTFOLLOWS_WILD, // check upon a2, get Follows(_, a2)
    PREPARENT, // check upon a1, get a2 that Parents(a1, a2)
    POSTPARENT, // check upon a2, get a1 that Parents(a1, a2)
    PREPARENTT, // check upon a1, get a2 that Parents*(a1, a2)
    POSTPARENTT, // check upon a2, get a1 that Parents*(a1, a2)
    PREPARENT_WILD, // check upon a1, get Parents(a1, _)
    POSTPARENT_WILD, // check upon a2, get Parents(_, a2)
    PREUSESS, // check upon s (stmt), get v that Uses(s, v)
    POSTUSESS, // check upon v, get s (stmt) that Uses(s, v)
    PREUSESP, // check upon p (procedure), get v that Uses(p, v)
    POSTUSESP, // check upon v, get p (procedure) that Uses(p, v)
    USES_WILDCARD, // check upon s (stmt), Uses(s, v)
    USEP_WILDCARD, // check upon p (procedure), Uses(p, v)
    PREMODIFIESS, // check upon s (stmt), get v that Modifies(s, v)
    POSTMODIFIESS, // check upon v, get s (stmt) that Modifies(s, v)
    PREMODIFIESP, // check upon p (procedure), get v that Modifies(p, v)
    POSTMODIFIESP, // check upon v, get p (procedure) that Modifes(p, v)
    MODIFIESS_WILDCARD, // check upon s (stmt), Modifies(s, v)
    MODIFIESP_WILDCARD, // check upon p (procedure), Modifies(p, v)
    ASSIGN_PATTERN, // evaluate pattern a(v, ...) or a("v", ...)
    INVALID // no suitable subrelation to evaluate
};

// 2nd level SubRelation table: table mapping ArgType to SubRelationType>
typedef std::unordered_map<int, SubRelationType> SEC_SRT_TABLE;
// 1st level SubRelation table: table mapping ArgType to SEC_SRT_TABLE
typedef std::unordered_map<int, SEC_SRT_TABLE> FIR_SRT_TABLE;
// SubRelation Table: table mapping relation to FIR_SRT_TABLE
typedef std::unordered_map<int, FIR_SRT_TABLE> SRT_LOOKUP_TABLE;

bool isWildCard(const std::string& str); // check if the argument is a wildcard
bool isPosInt(const std::string& str); // check if the argument is a positive integer
bool isName(const std::string& str); // check if the argument is the name of a variable or procedure

// extract quoted part of a string
// if it's not quoted, return the original string
std::string extractQuotedStr(const std::string& str);

// extract the content of pattern string
// return {isValidPattern, patternContent, isSubExpr}
std::tuple<bool, std::string, bool> extractPatternExpr(const std::string& str);

SRT_LOOKUP_TABLE generateSrtTable(); // generate a mapping from relation and argument types to SubRelationType

} // namespace queryevaluator
} // namespace qpbackend

#endif // QEHELPER_H
