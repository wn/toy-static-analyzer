#include "QueryEvaluator.h"

#include "Logger.h"

#include <stdexcept>
#include <unordered_set>
#include <utility>

#define DEBUG 0

namespace qpbackend {
namespace queryevaluator {
std::string QueryEvaluator::evaluateQuery(Query query) {
    SingleQueryEvaluator sqe(query);
    return sqe.evaluateQuery(pkb);
}

// helper structure to map such-that clause arguments to SubRelationType
SRT_LOOKUP_TABLE SingleQueryEvaluator::srt_table = generateSrtTable();

std::string SingleQueryEvaluator::evaluateQuery(const backend::PKB* pkb) {
    // initialize the table of candidates
    if (hasEvaluationCompleted) {
        handleError("same single query evaluator should not be called twice");
    }
    if (query.synonymsToReturn.size() == 0) {
        handleError("no selected, invalid query");
    }

    for (const auto& requested : query.synonymsToReturn) {
        initializeSynonym(pkb, requested);
    }

    // process all such that clauses
    for (const auto& clause : query.suchThatClauses) {
        if (hasClauseFailed) {
            break;
        }
        hasClauseFailed = !(evaluateSuchThatClause(pkb, clause));
    }

    // process all patterns
    for (const auto& clause : query.patternClauses) {
        if (hasClauseFailed) {
            break;
        }
        hasClauseFailed = !(evaluatePatternClause(pkb, clause));
    }

    // prepare output
    std::string result = produceResult();
    hasEvaluationCompleted = true;
    return result;
}

/**
 * convert evaluation result to a string
 */
std::string SingleQueryEvaluator::produceResult() {
    std::string result = "";

    // for basic requirement
    if (query.synonymsToReturn.size() == 1) {
        std::string inquired = query.synonymsToReturn[0];
        if (!hasClauseFailed) {
            for (const auto& candidate : synonym_candidates[inquired]) {
                if (result.size() > 0) {
                    result += ", ";
                }
                result += candidate;
            }
        }
    }

    return result;
};

/**
 * check if a name is a synonym's name and initialized.
 * if it's a synonym but not in the synonym_candidates, call initialization method
 * @param pkb : the pkb to look up for synonym's candidate values.
 * @param synonym : the name of th synonym. i.e., for synonym x, "x" is its name.
 */
void SingleQueryEvaluator::initializeSynonym(const backend::PKB* pkb, std::string const& synonymName) {
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
        synonym_candidates[synonymName] = pkb->getAllVariables();
    } else if (entityType == PROCEDURE) {
        synonym_candidates[synonymName] = pkb->getAllProcedures();
    } else if (entityType == CONSTANT) {
        synonym_candidates[synonymName] = {}; // TODO: call PKB for constants
    } else {
        STATEMENT_NUMBER_LIST result = pkb->getAllStatements();
        if (entityType != STMT) {
            auto unary_predictor = [](STATEMENT_NUMBER x) { return true; };
            /* TODO: update the PKB.h accordingly
            switch (et) {
            case READ:
                unary_predictor = [](STATEMENT_NUMBER x) { return !(pkb->isRead(x)); };
                break;
            case PRINT:
                unary_predictor = [](STATEMENT_NUMBER x) { return !(pkb->isPrint(x)); };
                break;
            case CALL:
                unary_predictor = [](STATEMENT_NUMBER x) { return !(pkb->isCall(x)); };
                break;
            case WHILE:
                unary_predictor = [](STATEMENT_NUMBER x) { return !(pkb->isWhile(x)); };
                break;
            case IF:
                unary_predictor = [](STATEMENT_NUMBER x) { return !(pkb->isIf(x)); };
                break;
            case ASSIGN:
                unary_predictor = [](STATEMENT_NUMBER x) { return !(pkb->isAssign(x)); };
                break;
            default:
                handleError("invalid entity type"); // TODO: handle invalid entity type
                return;
            } */
            result.erase(std::remove_if(result.begin(), result.end(), unary_predictor), result.end());
        }
        synonym_candidates[synonymName] = castToStrVector<STATEMENT_NUMBER>(result);
    }
}

/**
 * evaluate single such-that clause
 * @param pkb
 * @param suchThatClause
 * @return return false if (i) semantic errors encountered (ii) no result found
 */
bool SingleQueryEvaluator::evaluateSuchThatClause(const backend::PKB* pkb,
                                                  const std::tuple<RelationType, std::string, std::string>& suchThatClause) {
    RelationType rt = std::get<0>(suchThatClause);
    std::string arg1 = std::get<1>(suchThatClause);
    std::string arg2 = std::get<2>(suchThatClause);

    ArgType arg_type_1 = getArgType(pkb, arg1);
    ArgType arg_type_2 = getArgType(pkb, arg2);
    if ((arg_type_1 == INVALID_ARG) || (arg_type_2 == INVALID_ARG) || (arg_type_1 == EXPR) ||
        (arg_type_2 == EXPR)) {
        handleError("invalid argument type for such-that clause: " + arg1 + ", " + arg2);
        return false;
    }

    SubRelationType srt;
    try {
        srt = srt_table[rt][arg_type_1][arg_type_2];
    } catch (const std::out_of_range& oor) {
        handleError("Evaluation for the relation type not implemented.");
        srt = INVALID;
    }

    if (srt == INVALID) {
        handleError("invalid argument type for this relation: " + arg1 + ", " + arg2);
        return false;
    }

    if (isSynonym(pkb, arg1)) {
        switch (arg_type_2) {
        case STMT_SYNONYM:
        case VAR_SYNONYM:
        case PROC_SYNONYM:
        case CONST_SYNONYM:
            return evaluateSynonymSynonym(pkb, srt, arg1, arg2);
        case NUM_ENTITY:
            return evaluateEntitySynonym(pkb, srt, arg2, arg1); // swap the arguments as the called method required
        case NAME_ENTITY:
            return evaluateEntitySynonym(pkb, srt, arg2, arg1); // swap the arguments as the called method required
        case WILDCARD:
            return evaluateSynonymWildcard(pkb, srt, arg1);
        default:
            handleError("invalid arg2 type: " + arg2);
            return false;
        }
    } else if (arg_type_1 == NUM_ENTITY || arg_type_1 == NAME_ENTITY) {
        switch (arg_type_2) {
        case STMT_SYNONYM:
        case VAR_SYNONYM:
        case PROC_SYNONYM:
        case CONST_SYNONYM:
            return evaluateEntitySynonym(pkb, srt, arg1, arg2);
        case NUM_ENTITY:
            return evaluateEntityEntity(pkb, srt, arg1, arg2);
        case NAME_ENTITY:
            return evaluateEntityEntity(pkb, srt, arg1, arg2);
        case WILDCARD:
            return evaluateEntityWildcard(pkb, srt, arg1);
        default:
            handleError("invalid arg2 type: " + arg2);
            return false;
        }
    } else if (arg_type_1 == WILDCARD) {
        switch (arg_type_2) {
        case STMT_SYNONYM:
        case VAR_SYNONYM:
        case PROC_SYNONYM:
        case CONST_SYNONYM:
            return evaluateSynonymWildcard(pkb, srt, arg2);
        case NUM_ENTITY:
            return evaluateEntityWildcard(pkb, srt, arg2);
        case NAME_ENTITY:
            return evaluateEntityWildcard(pkb, srt, arg2);
        case WILDCARD:
            return evaluateWildcardWildcard(pkb, srt);
        default:
            handleError("invalid arg2 type: " + arg2);
            return false;
        }
    } else {
        handleError("invalid arg1 type: " + arg1);
        return false;
    }
}

// TODO: implement pattern clause evaluation
/**
 * evaluate pattern clause
 * @param pkb : the pkb to evaluate the clause against
 * @param patternClause : the pattern clause to evaluate
 * @return false if the pattern cannot hold true for any statements, otherwise true
 */
bool SingleQueryEvaluator::evaluatePatternClause(const backend::PKB* pkb,
                                                 const std::tuple<std::string, std::string, std::string>& patternClause) {
    return false;
}

/**
 * evaluate the clause against a pair of synonyms
 * after evaluation, update two synonyms' candidate list
 * @param pkb
 * @param subRelationType
 * @param arg1 : the name of first synonym
 * @param arg2 : the name of second synonym
 * @return : false if any synonym's candidate value list got empty. otherwise true
 */
bool SingleQueryEvaluator::evaluateSynonymSynonym(const backend::PKB* pkb,
                                                  SubRelationType subRelationType,
                                                  std::string const& arg1,
                                                  std::string const& arg2) {
    std::vector<std::string> candidates_1 = synonym_candidates[arg1];
    std::vector<std::string> candidates_2 = synonym_candidates[arg2];

    if (candidates_1.empty() || candidates_2.empty()) {
        return false;
    }

    // helper data structure
    std::vector<bool> arg1_mask(candidates_1.size(), false);
    std::unordered_set<std::string> arg2_set;

    // call PKB to retrive answers to find possible pairs
    for (std::size_t i = 0; i < candidates_1.size(); i++) {
        std::string c1 = candidates_1[i];
        std::vector<std::string> c1_result = inquirePKBForRelation(pkb, subRelationType, c1);
        std::vector<std::string> c2_list = vectorIntersection<std::string>(candidates_2, c1_result);
        if (!c2_list.empty()) {
            arg1_mask[i] = true;
            arg2_set.insert(c2_list.begin(), c2_list.end());
        }
    }

    // filter the candidates by pairwise constraints
    bool flag_1 = false;
    std::vector<std::string> result1;
    for (size_t i = 0; i < arg1_mask.size(); i++) {
        if (arg1_mask[i]) {
            flag_1 = true;
            result1.push_back(candidates_1[i]);
        }
    }
    synonym_candidates[arg1] = std::move(result1);

    bool flag_2 = !arg2_set.empty();
    synonym_candidates[arg2] = std::vector<std::string>(arg2_set.begin(), arg2_set.end());

    // TODO: consider the pairwise constraint for advanced requirement
    return flag_1 && flag_2;
}

/**
 * evaluate the clause against an entity and a synonym
 * after evaluation, update the candidate value list of the synonym
 * @param pkb
 * @param subRelationType
 * @param arg1 : an entity--stetment number or procedure name or variable name
 * @param arg2 : the name of a synonym
 * @return false if no candidates of synonym makes the relation hold, otherwise true
 */
bool SingleQueryEvaluator::evaluateEntitySynonym(const backend::PKB* pkb,
                                                 SubRelationType subRelationType,
                                                 std::string const& arg1,
                                                 std::string const& arg2) {
    std::vector<std::string> arg1_result = inquirePKBForRelation(pkb, subRelationType, arg1);
    synonym_candidates[arg2] = vectorIntersection<std::string>(synonym_candidates[arg2], arg1_result);
    return !(synonym_candidates[arg2]).empty();
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
    std::vector<std::string> arg1_result = inquirePKBForRelation(pkb, subRelationType, arg1);
    return isFoundInVector<std::string>(arg1_result, arg2);
}

/**
 * evaluate the clause against a synonym and a wildcard (placeholde '_')
 * after the evaluation, update the synonm's candidate list
 * @param pkb
 * @param subRelationType
 * @param arg : the name of the synonym
 * @return false if the synonm's candidate list gets empty
 */
bool SingleQueryEvaluator::evaluateSynonymWildcard(const backend::PKB* pkb,
                                                   SubRelationType subRelationType,
                                                   std::string const& arg) {
    std::vector<std::string> result = inquirePKBForRelationWildcard(pkb, subRelationType);
    synonym_candidates[arg] = vectorIntersection<std::string>(synonym_candidates[arg], result);
    return !(synonym_candidates[arg].empty());
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
    std::vector<std::string> result = inquirePKBForRelationWildcard(pkb, subRelationType);
    return isFoundInVector<std::string>(result, arg);
}

/**
 * evaluate a pair of wildcard
 * @param pkb
 * @param subRelationType
 * @return false if no such relations exist in the source code, otherwise true
 */
bool SingleQueryEvaluator::evaluateWildcardWildcard(const backend::PKB* pkb, SubRelationType subRelationType) {
    std::vector<std::string> result = inquirePKBForRelationWildcard(pkb, subRelationType);
    return !(result.empty());
}

/**
 * call PKB API methods to retrieve answer for the given relation and argument
 * @param pkb
 * @param subRelationType
 * @param arg : an entity--stetment number or procedure name or variable name
 * @return the list of values that together with the given entity make the relation hold
 */
std::vector<std::string> SingleQueryEvaluator::inquirePKBForRelation(const backend::PKB* pkb,
                                                                     SubRelationType subRelationType,
                                                                     std::string const& arg) {
    std::vector<std::string> result;
    STATEMENT_NUMBER_LIST stmts;
    switch (subRelationType) {
    case PREFOLLOWS:
        stmts = pkb->getStatementsFollowedBy(std::stoi(arg));
        if (!stmts.empty()) {
            result.push_back(std::to_string(stmts[0]));
        }
        break;
    case POSTFOLLOWS:
        stmts = pkb->getStatementsThatFollows(std::stoi(arg));
        if (!stmts.empty()) {
            result.push_back(std::to_string(stmts.back()));
        }
        break;
    case PREFOLLOWST:
        stmts = pkb->getStatementsFollowedBy(std::stoi(arg));
        result = castToStrVector<STATEMENT_NUMBER>(stmts);
        break;
    case POSTFOLLOWST:
        stmts = pkb->getStatementsThatFollows(std::stoi(arg));
        result = castToStrVector<STATEMENT_NUMBER>(stmts);
        break;
    case PREPARENT:
        stmts = pkb->getDescendants(std::stoi(arg));
        for (const auto& child : stmts) {
            if (std::stoi(arg) == (pkb->getAncestors(child)).at(0)) {
                result.push_back(std::to_string(child));
            }
        }
        break;
    case POSTPARENT:
        stmts = pkb->getAncestors(std::stoi(arg));
        if (!stmts.empty()) {
            result.push_back(std::to_string(stmts[0]));
        }
        break;
    case PREPARENTT:
        stmts = pkb->getDescendants(std::stoi(arg));
        result = castToStrVector<STATEMENT_NUMBER>(stmts);
        break;
    case POSTPARENTT:
        stmts = pkb->getAncestors(std::stoi(arg));
        result = castToStrVector<STATEMENT_NUMBER>(stmts);
        break;
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
std::vector<std::string>
SingleQueryEvaluator::inquirePKBForRelationWildcard(const backend::PKB* pkb, SubRelationType subRelationType) {
    std::vector<std::string> result;
    STATEMENT_NUMBER_LIST stmts;
    switch (subRelationType) {
    case PREFOLLOWS_WILD:
        stmts = pkb->getAllStatementsThatAreFollowed();
        result = castToStrVector<STATEMENT_NUMBER>(stmts);
        break;
    case POSTFOLLOWS_WILD:
        stmts = pkb->getAllStatementsThatFollows();
        result = castToStrVector<STATEMENT_NUMBER>(stmts);
        break;
    case PREPARENT_WILD:
        stmts = pkb->getStatementsThatHaveDescendants();
        result = castToStrVector<STATEMENT_NUMBER>(stmts);
        break;
    case POSTPARENT_WILD:
        stmts = pkb->getStatementsThatHaveAncestors();
        result = castToStrVector<STATEMENT_NUMBER>(stmts);
        break;
    default:
        handleError("unknown sub-relation type");
    }
    return result;
}

// TODO: implement error handling other than logging
/**
 * handle semantic error, exit evaluation properly
 */
void SingleQueryEvaluator::handleError(std::string const& msg) {
    logLine(msg);
}

/**
 * check if a string is the name of the synonym
 * @param pkb : used for synonym candidates initialization
 * @param str : the string to test
 * @return true if it's the name of a synonym, otherwis fakse
 */
bool SingleQueryEvaluator::isSynonym(const backend::PKB* pkb, std::string const& str) {
    initializeSynonym(pkb, str);
    return (synonym_candidates.find(str) != synonym_candidates.end());
}

/**
 * detrmine the type of an argument used in the clauses
 * @param pkb
 * @param arg
 * @return the type of the argument
 */
ArgType SingleQueryEvaluator::getArgType(const backend::PKB* pkb, std::string const& arg) {
    if (isWildCard(arg)) {
        return WILDCARD;
    } else if (isSynonym(pkb, arg)) {
        switch (query.declarationMap[arg]) {
        case VARIABLE:
            return VAR_SYNONYM;
        case PROCEDURE:
            return PROC_SYNONYM;
        case CONSTANT:
            return CONST_SYNONYM;
        default:
            return STMT_SYNONYM;
        }
    } else if (isPosInt(arg)) {
        return NUM_ENTITY;
    } else if (isName(arg)) {
        return NAME_ENTITY;
    } else if (isExpression(arg)) {
        return EXPR;
    } else {
        return INVALID_ARG;
    }
}

/**
 * cast vector to vector of strings
 */
template <typename T> std::vector<std::string> castToStrVector(const std::vector<T>& v) {
    std::vector<std::string> result;
    for (const auto& element : v) {
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
