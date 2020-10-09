#ifndef QPTYPES_H
#define QPTYPES_H

#include <string>
#include <tuple>
#include <unordered_map>
#include <utility> // std::pair
#include <vector>

namespace qpbackend {

enum EntityType { STMT, READ, PRINT, CALL, WHILE, IF, ASSIGN, VARIABLE, CONSTANT, PROCEDURE };

enum ClauseType {
    // Relation Types
    FOLLOWS,
    FOLLOWST,
    PARENT,
    PARENTT,
    USES,
    MODIFIES,
    CALLS,
    CALLST,
    NEXT,
    NEXTT,
    AFFECTS,
    AFFECTST,
    // Pattern
    ASSIGN_PATTERN_EXACT,
    ASSIGN_PATTERN_SUB_EXPR,
    ASSIGN_PATTERN_WILDCARD,
    IF_PATTERN,
    WHILE_PATTERN,
    // With
    WITH,
    // Invalid clause type, not accepted for evaluation
    INVALID_CLAUSE_TYPE
};

enum ReturnType {
    PROC_PROC_NAME,
    CALL_PROC_NAME,
    VAR_VAR_NAME,
    READ_VAR_NAME,
    PRINT_VAR_NAME,
    CONSTANT_VALUE,
    STMT_STMT_NO,
    READ_STMT_NO,
    PRINT_STMT_NO,
    CALL_STMT_NO,
    WHILE_STMT_NO,
    IF_STMT_NO,
    ASSIGN_STMT_NO,
    // Implicit elem types
    PROG_LINE, // prog_line pl; Select pl ...
    BOOLEAN, // `Select BOOLEAN`
    // Invalid return type, not accepted for evaluation
    INVALID_RETURN_TYPE
};

// type of argument used in relation, used for evaluation
enum ArgType {
    // synonym name of a statement synonym
    // READ, IF, CALL, WHILE, ASSIGN, PRINT are all regarded as subtypes
    // of STMT synonym.
    STMT_SYNONYM,
    VAR_SYNONYM, // synonym name of a variable synonym
    PROC_SYNONYM, // synonym name of a procedure synonym
    CONST_SYNONYM, // synonym name of a constant synonym
    NAME_ENTITY, // name of variable or procedure, e.g. "\"centroidX\"" "\"main\""
    NUM_ENTITY, // constant number or statement number of line number, e.g. "42"
    WILDCARD, // placeholder sign, e.g. "_"
    INVALID_ARG // invalid argument, not accepted for evaluation
};

typedef std::unordered_map<std::string, EntityType> DECLARATION_MAP;
typedef std::pair<ArgType, std::string /*argValue*/> ARG;
typedef std::tuple<ClauseType, ARG, ARG> RELATIONTUPLE;
typedef std::tuple<ClauseType, ARG, ARG, std::string /*expr*/> CLAUSE;
typedef CLAUSE PATTERNTUPLE;
typedef std::vector<CLAUSE> CLAUSE_LIST;
typedef std::pair<ReturnType, std::string /*synonymName*/> RETURN_CANDIDATE;
typedef std::vector<RETURN_CANDIDATE> RETURN_CANDIDATE_LIST;
// Legacy typedefs
typedef std::tuple<std::string /* pattern synonym */, std::string /* var */, std::string /* expression spec */> LEGACY_PATTERN_TUPLE;

std::string prettyPrintArg(const ARG& arg);
std::string prettyPrintArgType(ArgType argType);
} // namespace qpbackend

#endif // QPTYPES_H
