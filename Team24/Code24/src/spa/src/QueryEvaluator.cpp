#include "QueryEvaluator.h"

#include "Logger.h"
#include "PKB.h"
#include "QEHelper.h"
#include "QPTypes.h"
#include "Query.h"

#include <algorithm>
#include <functional>
#include <iostream>
#include <stdexcept>
#include <unordered_set>
#include <utility>

namespace qpbackend {
namespace queryevaluator {
std::vector<std::string> QueryEvaluator::evaluateQuery(Query query) {
    SingleQueryEvaluator sqe(query);
    return sqe.evaluateQuery(pkb);
}

// helper structure to map such-that clause arguments to SubRelationType
SRT_LOOKUP_TABLE SingleQueryEvaluator::srt_table = generateSrtTable();

std::vector<std::string> SingleQueryEvaluator::evaluateQuery(const backend::PKB* pkb) {
    // initialize the table of candidates
    if (hasEvaluationCompleted) {
        handleError("same single query evaluator should not be called twice");
    }

    if (query.returnCandidates.size() == 0) {
        handleError("no selected, invalid query");
    }

    for (const auto& requested : query.returnCandidates) {
        initializeIfSynonym(pkb, requested.second);
    }

    // sort and group clauses
    std::vector<std::vector<CLAUSE_LIST>> clauses = getClausesSortedAndGrouped(pkb);

    // evaluate clauses
    for (const auto& group : clauses) {
        if (hasClauseFailed) {
            break;
        }
        ResultTable stGroupRT;
        for (const auto& subgroup : group) {
            if (hasClauseFailed) {
                break;
            }
            ResultTable subGroupRT;
            for (const auto& clause : subgroup) {
                if (hasClauseFailed) {
                    break;
                }
                hasClauseFailed = hasClauseFailed || !(evaluateClause(pkb, clause, subGroupRT));
            }
            hasClauseFailed = hasClauseFailed || !stGroupRT.mergeTable(std::move(subGroupRT));
            updateSynonymsWithResultTable(stGroupRT);
        }
        hasClauseFailed = hasClauseFailed || !resultTable.mergeTable(std::move(stGroupRT));
        updateSynonymsWithResultTable(resultTable);
    }

    // prepare output
    hasEvaluationCompleted = true;
    return produceResult();
}

/**
 * convert evaluation result to a string
 */
std::vector<std::string> SingleQueryEvaluator::produceResult() {
    // for basic requirement
    if (query.returnCandidates.size() == 1) {
        std::string inquired = query.returnCandidates[0].second;
        if (!hasClauseFailed) {
            return synonym_candidates[inquired];
        }
    }

    return std::vector<std::string>();
};

/**
 * check if a name is a synonym's name and initialized.
 * if it's a synonym but not in the synonym_candidates, call initialization method
 * @param pkb : the pkb to look up for synonym's candidate values.
 * @param synonym : the name of th synonym. i.e., for synonym x, "x" is its name.
 */
void SingleQueryEvaluator::initializeIfSynonym(const backend::PKB* pkb, std::string const& synonymName) {
    if ((query.declarationMap.find(synonymName) != query.declarationMap.end()) &&
        (synonym_candidates.find(synonymName) == synonym_candidates.end())) {
        initializeCandidate(pkb, synonymName, query.declarationMap[synonymName]);
    }
}

/**
 * store the name in the synonym_candidates and intialize its liss of candidate values.
 * @param pkb : the pkb to look for candidate synonyms
 * @param synonym : the synonym's name
 * @param entityType : the type of the synonym in declaration
 */
void SingleQueryEvaluator::initializeCandidate(const backend::PKB* pkb,
                                               std::string const& synonymName,
                                               EntityType entityType) {
    // initialize the possible values of all synonyms
    if (entityType == VARIABLE) {
        VARIABLE_NAME_LIST variables = pkb->getAllVariables();
        synonym_candidates[synonymName] = std::vector<std::string>(variables.begin(), variables.end());
    } else if (entityType == PROCEDURE) {
        PROCEDURE_NAME_LIST procs = pkb->getAllProcedures();
        synonym_candidates[synonymName] = std::vector<PROCEDURE_NAME>(procs.begin(), procs.end());
    } else if (entityType == CONSTANT) {
        CONSTANT_NAME_SET consts = pkb->getAllConstants();
        std::vector<std::string> constStrs;
        std::copy(consts.begin(), consts.end(), std::back_inserter(constStrs));
        synonym_candidates[synonymName] = constStrs;
    } else {
        std::map<EntityType, std::function<bool(STATEMENT_NUMBER)>> entityTypeToUnaryPredictor = {
            { ASSIGN, [pkb](STATEMENT_NUMBER x) { return pkb->isAssign(x); } },
            { CALL, [pkb](STATEMENT_NUMBER x) { return pkb->isCall(x); } },
            { IF, [pkb](STATEMENT_NUMBER x) { return pkb->isIfElse(x); } },
            { PRINT, [pkb](STATEMENT_NUMBER x) { return pkb->isPrint(x); } },
            { READ, [pkb](STATEMENT_NUMBER x) { return pkb->isRead(x); } },
            { STMT, [pkb](STATEMENT_NUMBER x) { return true; } },
            { WHILE, [pkb](STATEMENT_NUMBER x) { return pkb->isWhile(x); } },
        };
        if (entityTypeToUnaryPredictor.find(entityType) == entityTypeToUnaryPredictor.end()) {
            handleError("invalid entity type");
            return;
        }
        // Only keep statements that fulfill the predicate
        STATEMENT_NUMBER_SET allStatements = pkb->getAllStatements();
        STATEMENT_NUMBER_SET result;
        auto predicate = entityTypeToUnaryPredictor[entityType];
        for (STATEMENT_NUMBER statementNumber : allStatements) {
            if (predicate(statementNumber)) {
                result.insert(statementNumber);
            }
        }

        synonym_candidates[synonymName] = castToStrVector<>(result);
    }
}

/**
 * evaluate single clause
 * @param pkb
 * @param clause
 * @param groupResultTable: IRT of the group the clause belongs to
 * @return return false if (i) semantic errors encountered (ii) no result found
 */
bool SingleQueryEvaluator::evaluateClause(const backend::PKB* pkb, const CLAUSE& clause, ResultTable& groupResultTable) {
    ClauseType rt = std::get<0>(clause);

    ArgType arg_type_1 = std::get<1>(clause).first;
    ArgType arg_type_2 = std::get<2>(clause).first;

    std::string arg1 = std::get<1>(clause).second;
    std::string arg2 = std::get<2>(clause).second;

    const std::string& patternStr = std::get<3>(clause);

    if ((arg_type_1 == INVALID_ARG) || (arg_type_2 == INVALID_ARG)) {
        handleError("invalid argument type for the clause: " + arg1 + ", " + arg2);
        return false;
    }

    SubRelationType srt = INVALID;
    try {
        srt = srt_table.at(rt).at(arg_type_1).at(arg_type_2);
    } catch (const std::out_of_range& oor) {
        handleError("Evaluation for the relation type not implemented.");
    }

    if (srt == INVALID) {
        handleError("invalid argument type for this relation: " + arg1 + ", " + arg2);
        return false;
    }

    // Initializes arguments if they point to declared synonyms.
    initializeIfSynonym(pkb, arg1);
    initializeIfSynonym(pkb, arg2);

    ClauseArgsType argsType = getClauseArgsType(arg_type_1, arg_type_2);

    switch (argsType) {
    case SynonymSynonym:
        return evaluateSynonymSynonym(pkb, srt, arg2, arg1, patternStr, groupResultTable);
    case SynonymEntity:
        return evaluateEntitySynonym(pkb, srt, arg2, arg1, patternStr,
                                     groupResultTable); // swap the arguments as the called method required
    case SynonymWildcard:
        return evaluateSynonymWildcard(pkb, srt, arg1, patternStr, groupResultTable);
    case EntitySynonym:
        return evaluateEntitySynonym(pkb, srt, arg1, arg2, patternStr, groupResultTable);
    case EntityEntity:
        return evaluateEntityEntity(pkb, srt, arg1, arg2);
    case EntityWildcard:
        return evaluateEntityWildcard(pkb, srt, arg1);
    case WildcardSynonym:
        return evaluateSynonymWildcard(pkb, srt, arg2, patternStr, groupResultTable);
    case WildcardEntity:
        return evaluateEntityWildcard(pkb, srt, arg2);
    case WildcardWildcard:
        return evaluateWildcardWildcard(pkb, srt);
    case Invalid1:
        handleError("invalid arg1 type: " + arg1);
        return false;
    case Invalid2:
        handleError("invalid arg2 type: " + arg2);
        return false;
    default:
        handleError("Clauses argument is not implemented yet.");
        return false;
    }
}

/**
 * evaluate the clause against a pair of synonyms
 * after evaluation, update two synonyms' candidate list
 * @param pkb
 * @param subRelationType
 * @param arg1 : the name of first synonym
 * @param arg2 : the name of second synonym
 * @param groupResultTable: the result table of the group the clause belongs to
 * @param patternStr: the pattern string
 * @return : false if any synonym's candidate value list got empty. otherwise true
 */
bool SingleQueryEvaluator::evaluateSynonymSynonym(const backend::PKB* pkb,
                                                  SubRelationType subRelationType,
                                                  std::string const& arg1,
                                                  std::string const& arg2,
                                                  std::string const& patternStr,
                                                  ResultTable& groupResultTable) {
    // TODO(https://github.com/nus-cs3203/team24-cp-spa-20s1/issues/246)
    // note that relation between the synonym and itself is not allowed
    // however, this does not hold for Next, fix it for the advanced
    bool isSelfRelation = (arg1 == arg2);
    std::vector<std::string> candidates_1 = synonym_candidates[arg1];
    std::vector<std::string> candidates_2 = synonym_candidates[arg2];

    if (candidates_1.empty() || candidates_2.empty()) {
        return false;
    }

    // check all pairs
    std::unordered_set<std::string> singleEntity;
    std::unordered_set<std::vector<std::string>, StringVectorHash> pairs;
    for (const auto& c1 : candidates_1) {
        std::vector<std::string> c1_result;
        c1_result = inquirePKBForRelationOrPattern(pkb, subRelationType, c1, patternStr);
        if (isSelfRelation) {
            if (isFoundInVector<std::string>(c1_result, c1)) {
                singleEntity.insert(c1);
            }
        } else {
            for (const auto& c2 : candidates_2) {
                if (isFoundInVector<std::string>(c1_result, c2)) {
                    pairs.insert({ c1, c2 });
                }
            }
        }
    }

    // update IRT table
    if (isSelfRelation) {
        ResultTable newRT(arg1, singleEntity);
        groupResultTable.mergeTable(std::move(newRT));
    } else {
        ResultTable newRT({ arg1, arg2 }, pairs);
        groupResultTable.mergeTable(std::move(newRT));
    }
    updateSynonymsWithResultTable(groupResultTable);
    return !groupResultTable.isEmpty();
}

/**
 * evaluate the clause against an entity and a synonym
 * after evaluation, update the candidate value list of the synonym
 * @param pkb
 * @param subRelationType
 * @param arg1 : an entity--stetment number or procedure name or variable name
 * @param arg2 : the name of a synonym
 * @param groupResultTable: the IRT table of the group the clause belongs to
 * @param patternStr : the pattern string
 * @return false if no candidates of synonym makes the relation hold, otherwise true
 */
bool SingleQueryEvaluator::evaluateEntitySynonym(const backend::PKB* pkb,
                                                 SubRelationType subRelationType,
                                                 std::string const& arg1,
                                                 std::string const& arg2,
                                                 std::string const& patternStr,
                                                 ResultTable& groupResultTable) {
    std::vector<std::string> arg1_result;
    arg1_result = inquirePKBForRelationOrPattern(pkb, subRelationType, arg1, patternStr);

    std::vector<std::string> result = vectorIntersection<>(arg1_result, synonym_candidates[arg2]);
    std::unordered_set<std::string> resultSet(result.begin(), result.end());
    ResultTable newRT(arg2, resultSet);
    groupResultTable.mergeTable(std::move(newRT));
    updateSynonymsWithResultTable(groupResultTable);
    return !groupResultTable.isEmpty();
}

/**
 * evaluate the clause
 * @param pkb
 * @param subRelationType
 * @param arg1 : an entity--stetment number or procedure name or variable name
 * @param arg2 : an entity--stetment number or procedure name or variable name
 * @return false if the relation does not hold for two entities, otherwise true
 */
bool SingleQueryEvaluator::evaluateEntityEntity(const backend::PKB* pkb,
                                                SubRelationType subRelationType,
                                                std::string const& arg1,
                                                std::string const& arg2) {
    std::vector<std::string> arg1_result = inquirePKBForRelationOrPattern(pkb, subRelationType, arg1, "");
    return isFoundInVector<std::string>(arg1_result, arg2);
}

/**
 * evaluate the clause against a synonym and a wildcard (placeholde '_')
 * after the evaluation, update the synonm's candidate list
 * @param pkb
 * @param subRelationType
 * @param arg : the name of the synonym
 * @param groupResultTable: the IRT table of the group the clause belongs to
 * @return false if the synonm's candidate list gets empty
 */
bool SingleQueryEvaluator::evaluateSynonymWildcard(const backend::PKB* pkb,
                                                   SubRelationType subRelationType,
                                                   std::string const& arg,
                                                   std::string const& patternStr,
                                                   ResultTable& groupResultTable) {
    std::vector<std::string> inquired_result = inquirePKBForRelationWildcard(pkb, subRelationType, patternStr);
    std::vector<std::string> result = vectorIntersection(inquired_result, synonym_candidates[arg]);
    std::unordered_set<std::string> resultSet(result.begin(), result.end());
    ResultTable newRT(arg, resultSet);
    groupResultTable.mergeTable(std::move(newRT));
    updateSynonymsWithResultTable(groupResultTable);
    return !groupResultTable.isEmpty();
}

/**
 * evaluate the clause against an entity and a wildcard (place hol)
 * @param pkb
 * @param subRelationType
 * @param arg : an entity--stetment number or procedure name or variable name
 * @return false if the entity cannot fulfill the relation anyway, otherwise true
 */
bool SingleQueryEvaluator::evaluateEntityWildcard(const backend::PKB* pkb,
                                                  SubRelationType subRelationType,
                                                  std::string const& arg) {
    std::vector<std::string> result = inquirePKBForRelationWildcard(pkb, subRelationType, "");
    return isFoundInVector<std::string>(result, arg);
}

/**
 * evaluate a pair of wildcard
 * @param pkb
 * @param subRelationType
 * @return false if no such relations exist in the source code, otherwise true
 */
bool SingleQueryEvaluator::evaluateWildcardWildcard(const backend::PKB* pkb, SubRelationType subRelationType) {
    std::vector<std::string> result = inquirePKBForRelationWildcard(pkb, subRelationType, "");
    return !(result.empty());
}

/**
 * call PKB API methods to retrieve answer for the given relation and argument
 * @param pkb
 * @param subRelationType
 * @param arg : an entity--stetment number or procedure name or variable name
 * @return the list of values that together with the given entity make the relation hold
 */
std::vector<std::string> SingleQueryEvaluator::inquirePKBForRelationOrPattern(const backend::PKB* pkb,
                                                                              SubRelationType subRelationType,
                                                                              const std::string& arg,
                                                                              const std::string& patternStr) {
    std::vector<std::string> result;
    STATEMENT_NUMBER_SET stmts;
    PROCEDURE_NAME_SET procs;
    VARIABLE_NAME_LIST vars;
    switch (subRelationType) {
    case PREFOLLOWS:
        stmts = pkb->getDirectFollow(std::stoi(arg));
        result = castToStrVector<>(stmts);
        break;
    case POSTFOLLOWS:
        stmts = pkb->getDirectFollowedBy(std::stoi(arg));
        result = castToStrVector<>(stmts);
        break;
    case PREFOLLOWST:
        stmts = pkb->getStatementsThatFollows(std::stoi(arg));
        result = castToStrVector<>(stmts);
        break;
    case POSTFOLLOWST:
        stmts = pkb->getStatementsFollowedBy(std::stoi(arg));
        result = castToStrVector<>(stmts);
        break;
    case PREPARENT:
        stmts = pkb->getChildren(std::stoi(arg));
        result = castToStrVector<>(stmts);
        break;
    case POSTPARENT:
        stmts = pkb->getParent(std::stoi(arg));
        result = castToStrVector<>(stmts);
        break;
    case PREPARENTT:
        stmts = pkb->getDescendants(std::stoi(arg));
        result = castToStrVector<>(stmts);
        break;
    case POSTPARENTT:
        stmts = pkb->getAncestors(std::stoi(arg));
        result = castToStrVector<>(stmts);
        break;
    case PREUSESS: {
        VARIABLE_NAME_LIST variables = pkb->getVariablesUsedIn(std::stoi(arg));
        result = std::vector<VARIABLE_NAME>(variables.begin(), variables.end());
        break;
    }
    case POSTUSESS:
        stmts = pkb->getStatementsThatUse(arg);
        result = castToStrVector<>(stmts);
        break;
    case PREUSESP: {
        VARIABLE_NAME_LIST variables = pkb->getVariablesUsedIn(arg);
        result = std::vector<VARIABLE_NAME>(variables.begin(), variables.end());
        break;
    }
    case POSTUSESP: {
        auto procs = pkb->getProceduresThatUse(arg);
        result = std::vector<PROCEDURE_NAME>(procs.begin(), procs.end());
        break;
    }
    case PREMODIFIESS: {
        VARIABLE_NAME_LIST variables = pkb->getVariablesModifiedBy(std::stoi(arg));
        result = std::vector<VARIABLE_NAME>(variables.begin(), variables.end());
        break;
    }
    case POSTMODIFIESS:
        stmts = pkb->getStatementsThatModify(arg);
        result = castToStrVector<>(stmts);
        break;
    case PREMODIFIESP: {
        VARIABLE_NAME_LIST variables = pkb->getVariablesModifiedBy(arg);
        result = std::vector<VARIABLE_NAME>(variables.begin(), variables.end());
        break;
    }
    case POSTMODIFIESP: {
        auto procs = pkb->getProceduresThatModify(arg);
        result = std::vector<PROCEDURE_NAME>(procs.begin(), procs.end());
        break;
    }
    case PRENEXT: {
        stmts = pkb->getNextStatementOf(std::stoi(arg), false);
        result = castToStrVector<>(stmts);
        break;
    }
    case POSTNEXT: {
        stmts = pkb->getPreviousStatementOf(std::stoi(arg), false);
        result = castToStrVector<>(stmts);
        break;
    }
    case PRENEXTT: {
        stmts = pkb->getNextStatementOf(std::stoi(arg), true);
        result = castToStrVector<>(stmts);
        break;
    }
    case POSTNEXTT: {
        stmts = pkb->getPreviousStatementOf(std::stoi(arg), true);
        result = castToStrVector<>(stmts);
        break;
    }
    case PRECALLS: {
        procs = pkb->getProceduresCalledBy(arg, false);
        result = std::vector<PROCEDURE_NAME>(procs.begin(), procs.end());
        break;
    }
    case POSTCALLS: {
        procs = pkb->getProcedureThatCalls(arg, false);
        result = std::vector<PROCEDURE_NAME>(procs.begin(), procs.end());
        break;
    }
    case PRECALLST: {
        procs = pkb->getProceduresCalledBy(arg, true);
        result = std::vector<PROCEDURE_NAME>(procs.begin(), procs.end());
        break;
    }
    case POSTCALLST: {
        procs = pkb->getProcedureThatCalls(arg, true);
        result = std::vector<PROCEDURE_NAME>(procs.begin(), procs.end());
        break;
    }
    case ASSIGN_PATTERN_EXACT_SRT: {
        stmts = pkb->getAllAssignmentStatementsThatMatch(arg, patternStr, false);
        result = castToStrVector<>(stmts);
        break;
    }
    case ASSIGN_PATTERN_SUBEXPR_SRT: {
        stmts = pkb->getAllAssignmentStatementsThatMatch(arg, patternStr, true);
        result = castToStrVector<>(stmts);
        break;
    }
    case ASSIGN_PATTERN_WILDCARD_SRT: {
        stmts = pkb->getAllAssignmentStatementsThatMatch(arg, "", true);
        result = castToStrVector<>(stmts);
        break;
    }
    default:
        handleError("unknown sub-relation type");
    }
    return result;
}

/**
 * call PKB API methods to retrieve answer for the given relation
 * @param pkb
 * @param subRelationType
 * @return the list of values that make the relation hold
 */
std::vector<std::string> SingleQueryEvaluator::inquirePKBForRelationWildcard(const backend::PKB* pkb,
                                                                             SubRelationType subRelationType,
                                                                             const std::string& patternStr) {
    std::vector<std::string> result;
    STATEMENT_NUMBER_SET stmts;
    PROCEDURE_NAME_SET procs;
    switch (subRelationType) {
    case PREFOLLOWS_WILD:
        stmts = pkb->getAllStatementsThatAreFollowed();
        result = castToStrVector<>(stmts);
        break;
    case POSTFOLLOWS_WILD:
        stmts = pkb->getAllStatementsThatFollows();
        result = castToStrVector<>(stmts);
        break;
    case PREPARENT_WILD:
        stmts = pkb->getStatementsThatHaveDescendants();
        result = castToStrVector<>(stmts);
        break;
    case POSTPARENT_WILD:
        stmts = pkb->getStatementsThatHaveAncestors();
        result = castToStrVector<>(stmts);
        break;
    case USES_WILDCARD:
        stmts = pkb->getStatementsThatUseSomeVariable();
        result = castToStrVector<>(stmts);
        break;
    case USEP_WILDCARD: {
        auto procs = pkb->getProceduresThatUseSomeVariable();
        result = std::vector<PROCEDURE_NAME>(procs.begin(), procs.end());
        break;
    }
    case MODIFIESS_WILDCARD:
        stmts = pkb->getStatementsThatModifySomeVariable();
        result = castToStrVector<>(stmts);
        break;
    case MODIFIESP_WILDCARD: {
        auto procs = pkb->getProceduresThatModifySomeVariable();
        result = std::vector<PROCEDURE_NAME>(procs.begin(), procs.end());
        break;
    }
    case PRENEXT_WILD:
        stmts = pkb->getAllStatementsWithNext();
        result = castToStrVector<>(stmts);
        break;
    case POSTNEXT_WILD:
        stmts = pkb->getAllStatementsWithPrev();
        result = castToStrVector<>(stmts);
        break;
    case PRECALL_WILD:
        procs = pkb->getAllProceduresThatCallSomeProcedure();
        result = std::vector<PROCEDURE_NAME>(procs.begin(), procs.end());
        break;
    case POSTCALL_WILD:
        procs = pkb->getAllCalledProcedures();
        result = std::vector<PROCEDURE_NAME>(procs.begin(), procs.end());
        break;
    case ASSIGN_PATTERN_EXACT_SRT: {
        stmts = pkb->getAllAssignmentStatementsThatMatch("_", patternStr, false);
        result = castToStrVector<>(stmts);
        break;
    }
    case ASSIGN_PATTERN_SUBEXPR_SRT: {
        stmts = pkb->getAllAssignmentStatementsThatMatch("_", patternStr, true);
        result = castToStrVector<>(stmts);
        break;
    }
    case ASSIGN_PATTERN_WILDCARD_SRT: {
        stmts = pkb->getAllAssignmentStatementsThatMatch("_", "", true);
        result = castToStrVector<>(stmts);
        break;
    }
    default:
        handleError("unknown sub-relation type");
    }
    return result;
}

std::vector<std::vector<CLAUSE_LIST>> SingleQueryEvaluator::getClausesSortedAndGrouped(const backend::PKB* pkb) {
    // TODO(https://github.com/nus-cs3203/team24-cp-spa-20s1/issues/271)
    // remove construction of CLAUSE into QPP after refactoring Query struct
    std::vector<CLAUSE_LIST> clauses;
    for (const auto& relationClause : query.suchThatClauses) {
        CLAUSE clause = { std::get<0>(relationClause), std::get<1>(relationClause),
                          std::get<2>(relationClause), "" };
        clauses.push_back({ clause });
    }

    CLAUSE invalidClause = { INVALID_CLAUSE_TYPE, { INVALID_ARG, "" }, { INVALID_ARG, "" }, "" };

    for (const auto& patternClause : query.patternClauses) {
        clauses.push_back({ patternClause });
    }

    // TODO(https://github.com/nus-cs3203/team24-cp-spa-20s1/issues/266)
    // sort and group the clauses
    return { clauses };
}

/**
 * update synonym candidates with a given Intermediate Result Table
 * @param table : the IRT used
 */
void SingleQueryEvaluator::updateSynonymsWithResultTable(ResultTable& table) {
    for (const auto& p : synonym_candidates) {
        table.updateSynonymValueVector(p.first, synonym_candidates[p.first]);
    }
}

// TODO(https://github.com/nus-cs3203/team24-cp-spa-20s1/issues/281)
// implement error handling other than logging
/**
 * handle semantic error, exit evaluation properly
 */
void SingleQueryEvaluator::handleError(std::string const& msg) {
    logLine(msg);
}

/**
 * check if a string is the name of a synonym
 * @param pkb : used for synonym candidates initialization
 * @param str : the string to test
 * @return true if it's the name of a synonym, otherwis false
 */
bool SingleQueryEvaluator::isSynonym(std::string const& str) {
    return (query.declarationMap.find(str) != query.declarationMap.end());
}

/**
 * cast vector to vector of strings
 */
template <typename T> std::vector<std::string> castToStrVector(const std::vector<T>& vect) {
    std::vector<std::string> result;
    for (const auto& element : vect) {
        result.push_back(std::to_string(element));
    }
    return result;
}

/**
 * cast set to vector of strings
 */
template <typename T> std::vector<std::string> castToStrVector(const std::unordered_set<T>& s) {
    std::vector<std::string> result;
    for (const auto& element : s) {
        result.push_back(std::to_string(element));
    }
    return result;
}

/**
 * check if vector v contains argument arg
 * @tparam T
 * @param v
 * @param arg
 * @return return true if contained, otherwise false
 */
template <typename T> bool isFoundInVector(const std::vector<T>& v, T arg) {
    return std::find(v.begin(), v.end(), arg) != v.end();
}

/**
 * return the intersection of two vectors lst1 and lst2
 * @tparam T
 * @param lst1 : the first vector
 * @param lst2 : the second vector
 * @return : return the intersection of two vectors
 */
template <typename T>
std::vector<T> vectorIntersection(const std::vector<T>& lst1, const std::vector<T>& lst2) {
    std::vector<T> result;
    std::unordered_set<T> lst1_set(lst1.begin(), lst1.end());
    for (const auto& element : lst2) {
        if (lst1_set.find(element) != lst1_set.end()) {
            result.push_back(element);
        }
    }
    return result;
}
} // namespace queryevaluator
} // namespace qpbackend
