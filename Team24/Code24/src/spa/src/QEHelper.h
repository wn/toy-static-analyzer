#ifndef QEHELPER_H
#define QEHELPER_H

#include "Query.h"

#include <string>

namespace qpbackend {
namespace queryevaluator {
// helper relation type derived from given relation type, used for evaluation
enum SubRelationType {
    PREFOLLOWS, // check a1, Follows(a1, a2)
    POSTFOLLOWS, // check a2, Follows(a1, a2)
    PREFOLLOWST, // check a1, Follows*(a1, a2)
    POSTFOLLOWST, // check a2, Follows*(a1, a2)
    PREFOLLOWS_WILD, // check a1, Follows(a1, _)
    POSTFOLLOWS_WILD, // check a2, Follows(_, a2)
    PREPARENT, // check a1, Parents(a1, a2)
    POSTPARENT, // check a2, Parents(a1, a2)
    PREPARENTT, // check a1, Parents*(a1, a2)
    POSTPARENTT, // check a2, Parents*(a1, a2)
    PREPARENT_WILD, // check a1, Parents(a1, _)
    POSTPARENT_WILD, // check a2, Parents(_, a2)
    INVALID // no suitable subrelation to evaluate
};

// type of argument used in relation, used for evaluation
enum ArgType {
    STMT_SYNONYM, // synonym name of a statement synonym
    VAR_SYNONYM, // synonym name of a variable synonym
    PROC_SYNONYM, // synonym name of a procedure synonym
    CONST_SYNONYM, // synonym name of a procedure synonym
    NAME_ENTITY, // name of variable or procedure, e.g. "\"centroidX\"" "\"main\""
    NUM_ENTITY, // constant number or statement number of line number, e.g. "42"
    EXPR, // expression used in pattern, e.g. "_\"x+y*z\"_"
    WILDCARD, // placeholder sign, e.g. "_"
    INVALID_ARG // invalid argument, not accepted for evaluation
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
// TODO : implement pattern expression check
bool isExpression(const std::string& str); // check if the argument is a pattern expression

SRT_LOOKUP_TABLE generateSrtTable(); // generate a mapping from relation and argument types to SubRelationType

} // namespace queryevaluator
} // namespace qpbackend

#endif // QEHELPER_H
