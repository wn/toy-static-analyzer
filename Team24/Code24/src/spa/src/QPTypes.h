#ifndef QPTYPES_H
#define QPTYPES_H

#include <string>
#include <tuple>
#include <utility> // std::pair
#include <vector>

namespace qpbackend {

enum EntityType { STMT, READ, PRINT, CALL, WHILE, IF, ASSIGN, VARIABLE, CONSTANT, PROCEDURE };

enum ClauseType { FOLLOWS, FOLLOWST, PARENT, PARENTT, USES, MODIFIES };

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

typedef std::pair<ArgType, std::string /*argValue*/> ARG;
typedef std::tuple<ClauseType, ARG, ARG> RELATIONTUPLE;
typedef std::tuple<std::string, std::string, std::string> PATTERNTUPLE;
typedef std::tuple<ClauseType, ARG, ARG, std::string /*expr*/> CLAUSE;
typedef std::vector<CLAUSE> CLAUSE_LIST;
typedef std::pair<ReturnType, std::string /*synonymName*/> RETURN_CANDIDATE;
typedef std::vector<RETURN_CANDIDATE> RETURN_CANDIDATE_LIST;

std::string prettyPrintArgType(ArgType argType);
} // namespace qpbackend

#endif // QPTYPES_H
