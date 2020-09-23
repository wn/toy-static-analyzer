#include "QueryPreprocessor.h"

#include "Lexer.h"
#include "Logger.h"
#include "Query.h"

#include <sstream>
#include <tuple>
#include <utility>
#include <vector>

// Constants
const std::string kQppErrorPrefix = "Log[Error-QueryPreprocessor]: ";
const std::string kQppLogWarnPrefix = "Log[WARN-QueryPreprocessor]: ";
const std::string kQppLogInfoPrefix = "Log[INFO-QueryPreprocessor]: ";
const std::string kPatternKeyword = "pattern";

namespace querypreprocessor {

// Declarations.

class State;
// <state of parser, isValidState>
typedef std::pair<State, bool> STATESTATUSPAIR;
// <state of parser, string representation of parsed token(s), isValidState>
typedef std::tuple<State, std::string, bool> STATE_RESULT_STATUS_TRIPLE;
void throwIfTokenDoesNotHaveExpectedTokenType(backend::lexer::TokenType expectedTokenType, const TOKEN& token);
static qpbackend::EntityType getEntityTypeFromToken(const TOKEN& token);
State parseSelect(State state);
State parseDeclarations(State state);
STATESTATUSPAIR parseSingleDeclaration(State state);
bool isValidDeclarationDelimiter(const TOKEN& token);
State parseFilteringClauses(State state);
// Declarations for such that clauses
STATESTATUSPAIR parseSingleSuchThatClause(State state);
STATESTATUSPAIR parseRelRef(State state);
STATESTATUSPAIR parseRelationStmtStmt(State state, qpbackend::RelationType relationType);
STATESTATUSPAIR parseRelationStmtEntOrEntEnt(State state, qpbackend::RelationType relationType);
std::string getStmtRefStringValue(const TOKEN& token);
bool isStmtRefToken(const TOKEN& token);
STATE_RESULT_STATUS_TRIPLE parseEntRef(State state);
// Declarations for patten clauses
STATESTATUSPAIR parseSinglePatternClause(State state);
STATE_RESULT_STATUS_TRIPLE parseExpressionSpec(State state);
bool isSynAssignToken(const TOKEN& token, State& state);

// Helper methods

void throwIfTokenDoesNotHaveExpectedTokenType(backend::lexer::TokenType expectedTokenType, const TOKEN& token) {
    if (token.type != expectedTokenType) {
        throw std::runtime_error(kQppErrorPrefix + "throwIfTokenDoesNotHaveExpectedTokenType: Expected a " +
                                 backend::lexer::prettyPrintType(expectedTokenType) + " token, instead received a " +
                                 backend::lexer::prettyPrintType(token.type) + " token");
    }
}

qpbackend::EntityType getEntityTypeFromToken(const TOKEN& token) {
    throwIfTokenDoesNotHaveExpectedTokenType(backend::lexer::TokenType::NAME, token);
    return qpbackend::entityTypeFromString(token.nameValue);
}

/**
 * The state of the query preprocessor.
 *
 * This abstracts away all the logic in manipulating the QPL query tokens and Query struct directly.
 * The State will also throw errors or exceptions once it has detected that it is in an invalid
 * state.
 */
class State {
  private:
    qpbackend::Query query;
    TOKENS tokens;
    unsigned int tokenPos{ 0 };
    void logTokenAt(unsigned int tokenPos, std::string methodName) {
        std::stringstream s;
        const TOKEN& token = tokens.at(tokenPos);
        s << kQppLogInfoPrefix << methodName << " Token Position: " << std::to_string(tokenPos)
          << "| value:" << token.nameValue << token.integerValue
          << " type:" << backend::lexer::prettyPrintType(token.type);
        logLine(s.str());
    }

  public:
    State() = default;
    explicit State(const TOKENS& tokens) {
        this->tokens = tokens;
    }

    // Copy getter(s)

    qpbackend::Query getQuery() {
        return query;
    }

    // Tokens manipulation

    TOKEN peekToken() {
        if (!hasTokensLeftToParse()) {
            throw std::runtime_error(kQppErrorPrefix +
                                     "State::peekToken: There are no more tokens left to peek.");
        }
        logTokenAt(tokenPos, "peekToken");
        return tokens[tokenPos];
    }

    backend::lexer::Token popToken() {
        if (!hasTokensLeftToParse()) {
            throw std::runtime_error(kQppErrorPrefix +
                                     "State::popToken: QueryPreprocessor has "
                                     "not successfully parsed a Query yet, "
                                     "but has run out of tokens to parse.\n" +
                                     query.toString());
        }
        TOKEN tokenToReturn = tokens[tokenPos];
        logTokenAt(tokenPos, "popToken");
        tokenPos += 1;
        return tokenToReturn;
    }

    backend::lexer::Token popUntilNonWhitespaceToken() {
        TOKEN token = popToken();
        while (token.type == backend::lexer::WHITESPACE) {
            token = popToken();
        }
        return token;
    }

    /**
     * If no next non whitespace token exists, it will pop all the tokens till there are no tokens
     * left to pop.
     */
    void popToNextNonWhitespaceToken() {
        while (hasTokensLeftToParse() && peekToken().type == backend::lexer::WHITESPACE) {
            popToken();
        }
    }

    bool popIfCurrentTokenIsWhitespaceToken() {
        if (!hasTokensLeftToParse() || peekToken().type != backend::lexer::WHITESPACE) {
            return false;
        }
        popToken();
        return true;
    }

    bool hasTokensLeftToParse() {
        return tokenPos < tokens.size();
    }

    // Query struct manipulation

    void addSynonymToQueryDeclarationMap(qpbackend::EntityType entityType, const TOKEN& token) {
        throwIfTokenDoesNotHaveExpectedTokenType(backend::lexer::TokenType::NAME, token);
        if (query.declarationMap.find(token.nameValue) != query.declarationMap.end()) {
            throw std::runtime_error(kQppErrorPrefix + "State::addSynonymToQueryDeclarationMap: Synonym " +
                                     token.nameValue + " has already been declared.");
        }

        query.declarationMap.insert(std::pair<std::string, qpbackend::EntityType>(token.nameValue, entityType));
    }

    void addSynonymToReturn(const TOKEN& token) {
        throwIfTokenDoesNotHaveExpectedTokenType(backend::lexer::TokenType::NAME, token);
        if (query.declarationMap.find(token.nameValue) == query.declarationMap.end()) {
            throw std::runtime_error(kQppErrorPrefix + "State::addSynonymToReturn: Cannot return values for synonym " +
                                     token.nameValue + " as it has not been declared.");
        }

        query.synonymsToReturn.push_back(token.nameValue);
    }

    void addSuchThatClause(qpbackend::RelationType relationType, std::string arg1, std::string arg2) {
        query.suchThatClauses.emplace_back(relationType, arg1, arg2);
    }

    void addPatternClause(std::string assignmentSynonym, std::string variableName, std::string expressionSpec) {
        query.patternClauses.emplace_back(assignmentSynonym, variableName, expressionSpec);
    }
};

// Parser / Business logic methods

/**
 * select-cl : declaration* ‘Select’ synonym ([ suchthat-cl ] | [ pattern-cl ])*
 */
State parseSelect(State state) {
    state = parseDeclarations(state);
    logLine(kQppLogInfoPrefix + "parseSelect: Query state after parsing declaration*" +
            state.getQuery().toString());
    const TOKEN& selectToken = state.popUntilNonWhitespaceToken();
    if (selectToken.type != backend::lexer::NAME || selectToken.nameValue != "Select") {
        // Irrecoverable syntax error, only 'Select' tokens come after declaration*. There is no
        // way to backtrack.
        throw std::runtime_error(kQppErrorPrefix +
                                 "parseSelect: Encountered "
                                 "\"" +
                                 selectToken.nameValue +
                                 "\""
                                 " while parsing, when \"Select\" is expected instead.");
    }

    const TOKEN& synonymToken = state.popUntilNonWhitespaceToken();
    state.addSynonymToReturn(synonymToken);
    if (!state.hasTokensLeftToParse()) {
        return state;
    }
    return parseFilteringClauses(state);
}

/**
 * declaration*
 *
 * Optimistically parse for declarations until an invalid state is reached. When that happens,
 * return the the most recent valid state.
 */
State parseDeclarations(State state) {
    bool isValidState;
    State tempState = state;
    std::tie(tempState, isValidState) = parseSingleDeclaration(state);

    // Optimistically parse for declarations until an invalid state is reached. Only update the
    // `state` with the newly obtained `tempState` if it is valid.
    while (isValidState) {
        state = tempState;
        std::tie(tempState, isValidState) = parseSingleDeclaration(state);
    }
    return state;
}


/**
 * declaration : design-entity synonym (‘,’ synonym)* ‘;’
 *
 * @return <state of parser, isStateInvalid> after attempting to parse a single declaration.
 */
STATESTATUSPAIR parseSingleDeclaration(State state) {
    const TOKEN& designEntity = state.popUntilNonWhitespaceToken();
    if (designEntity.type != backend::lexer::NAME || !qpbackend::isEntityString(designEntity.nameValue)) {
        return STATESTATUSPAIR(state, false);
    }
    qpbackend::EntityType entityType = getEntityTypeFromToken(designEntity);

    TOKEN synonym = state.popUntilNonWhitespaceToken();
    TOKEN delimiter = state.popUntilNonWhitespaceToken();
    logLine(kQppLogInfoPrefix + "parseSingleDeclaration:\n Synonym: " + synonym.nameValue +
            "\nDelimiter type:" + backend::lexer::prettyPrintType(delimiter.type));
    // Handles (‘,’ synonym)* ‘;’
    while (isValidDeclarationDelimiter(delimiter)) {
        // Calling this will throw an error if the synonym is invalid.
        // Case 1.
        // Semantic error: Redeclaring a synonym.
        // Case 2.
        // Irrecoverable syntax error: NAME token expected but not encountered.
        // There is no QPL grammar where a non-NAME token comes after a ',' or a design-entity.
        state.addSynonymToQueryDeclarationMap(entityType, synonym);
        // ';' is the last token of a declaration. Return <state, true> when it is encountered.
        if (delimiter.type == backend::lexer::SEMICOLON) {
            return STATESTATUSPAIR(state, true);
        }
        synonym = state.popUntilNonWhitespaceToken();
        delimiter = state.popUntilNonWhitespaceToken();
    }

    // Encountered an invalid delimiter, return <state, false> to signal this is an invalid state.
    return STATESTATUSPAIR(state, false);
}

/**
 * Checks for the following tokens (',' | ';').
 *
 * In parsing a declaration, a ',' is expected as a delimiter between synonyms, or a ';' is expected
 * as a terminator for the declaration. No other delimeter is expected.
 * @return True if it's one of the tokens, otherwise false.
 */
bool isValidDeclarationDelimiter(const TOKEN& token) {
    return token.type == backend::lexer::SEMICOLON || token.type == backend::lexer::COMMA;
}

/**
 * ([ suchthat-cl ] | [ pattern-cl ])*
 */
State parseFilteringClauses(State state) {
    if (!state.hasTokensLeftToParse()) return state;
    State tempState = state;
    bool isParseSuchThatValid = true;
    bool isParsePatternValid = true;
    while (state.hasTokensLeftToParse() && (isParseSuchThatValid || isParsePatternValid)) {
        std::tie(tempState, isParseSuchThatValid) = parseSingleSuchThatClause(state);
        if (isParseSuchThatValid) {
            state = tempState;
        }
        std::tie(tempState, isParsePatternValid) = parseSinglePatternClause(state);
        if (isParsePatternValid) {
            state = tempState;
        }
        state.popToNextNonWhitespaceToken();
    }
    if (!isParsePatternValid && !isParseSuchThatValid) {
        throw std::runtime_error(kQppErrorPrefix + "parseFilteringClauses: Unable to parse such that or pattern clauses\n" +
                                 state.getQuery().toString());
    }
    return state;
}

/**
 * suchthat-cl : ‘such that’ relRef
 * @return <state of parser, isStateInvalid>
 */
STATESTATUSPAIR parseSingleSuchThatClause(State state) {
    state.popToNextNonWhitespaceToken();
    if (!state.hasTokensLeftToParse()) return STATESTATUSPAIR(state, false);
    TOKEN suchToken = state.popUntilNonWhitespaceToken();
    if (suchToken.type != backend::lexer::NAME && suchToken.nameValue != "such") {
        return STATESTATUSPAIR(state, false);
    }
    TOKEN thatToken = state.popUntilNonWhitespaceToken();
    if (thatToken.type != backend::lexer::NAME && thatToken.nameValue != "that") {
        return STATESTATUSPAIR(state, false);
    }
    return parseRelRef(state);
}

/**
 * relRef : Follows | FollowsT | Parent | ParentT | UsesS | UsesP | ModifiesS | ModifiesP
 * @return <state of parser, isStateInvalid>
 */
STATESTATUSPAIR parseRelRef(State state) {
    std::stringstream stringstream;
    TOKEN keywordToken = state.popUntilNonWhitespaceToken();
    if (keywordToken.type != backend::lexer::NAME) {
        return STATESTATUSPAIR(state, false);
    }
    stringstream << keywordToken.nameValue;
    // A "*" may immediately follow the keyword
    if (state.peekToken().type == backend::lexer::TokenType::MULT) {
        state.popToken();
        stringstream << "*";
    }
    std::string possibleRelationString = stringstream.str();
    if (!qpbackend::isRelationString(possibleRelationString)) {
        return STATESTATUSPAIR(state, false);
    }
    qpbackend::RelationType relationType = qpbackend::relationTypeFromString(possibleRelationString);
    switch (relationType) {
    case qpbackend::FOLLOWS:
    case qpbackend::FOLLOWST:
    case qpbackend::PARENT:
    case qpbackend::PARENTT:
        return parseRelationStmtStmt(state, relationType);
    case qpbackend::USES:
    case qpbackend::MODIFIES:
        return parseRelationStmtEntOrEntEnt(state, relationType);
    default:
        return STATESTATUSPAIR(state, false);
    }
}

/**
 * Follows : ... ‘(’ stmtRef ‘,’ stmtRef ‘)’
 * FollowsT : ... ‘(’ stmtRef ‘,’ stmtRef ‘)’
 * Parent : ... ‘(’ stmtRef ‘,’ stmtRef ‘)’
 * ParentT : ... ‘(’ stmtRef ‘,’ stmtRef ‘)’
 * @return <state of parser, isStateInvalid>
 */
STATESTATUSPAIR parseRelationStmtStmt(State state, qpbackend::RelationType relationType) {
    TOKEN lParenToken = state.popUntilNonWhitespaceToken();
    if (lParenToken.type != backend::lexer::LPAREN || !state.hasTokensLeftToParse()) {
        logLine(kQppLogWarnPrefix +
                "parseRelationStmtStmt: Expected more tokens but finished "
                "consuming tokens or LPAREN token not found. Obtained " +
                backend::lexer::prettyPrintType(lParenToken.type));
        return STATESTATUSPAIR(state, false);
    }
    TOKEN stmt1Token = state.popUntilNonWhitespaceToken();
    if (!isStmtRefToken(stmt1Token) || !state.hasTokensLeftToParse()) {
        logLine(kQppLogWarnPrefix +
                "parseRelationStmtStmt: Expected more tokens but finished "
                "consuming tokens or STMT token not found. Obtained " +
                backend::lexer::prettyPrintType(stmt1Token.type));
        return STATESTATUSPAIR(state, false);
    }
    TOKEN commaToken = state.popUntilNonWhitespaceToken();
    if (commaToken.type != backend::lexer::COMMA || !state.hasTokensLeftToParse()) {
        logLine(kQppLogWarnPrefix +
                "parseRelationStmtStmt: Expected more tokens but finished "
                "consuming tokens or COMMA token not found. Obtained " +
                backend::lexer::prettyPrintType(commaToken.type));
        return STATESTATUSPAIR(state, false);
    }
    TOKEN stmt2Token = state.popUntilNonWhitespaceToken();
    if (!isStmtRefToken(stmt2Token) || !state.hasTokensLeftToParse()) {
        logLine(kQppLogWarnPrefix +
                "parseRelationStmtStmt: Expected more tokens but finished "
                "consuming tokens or STMT token not found. Obtained " +
                backend::lexer::prettyPrintType(stmt2Token.type));
        return STATESTATUSPAIR(state, false);
    }
    TOKEN rParenToken = state.popUntilNonWhitespaceToken();
    if (rParenToken.type != backend::lexer::RPAREN) {
        logLine(kQppLogWarnPrefix + "parseRelationStmtStmt: RPAREN token not found. Obtained " +
                backend::lexer::prettyPrintType(rParenToken.type));
        return STATESTATUSPAIR(state, false);
    }
    state.addSuchThatClause(relationType, getStmtRefStringValue(stmt1Token), getStmtRefStringValue(stmt2Token));
    return STATESTATUSPAIR(state, true);
}


// Helper methods for stmtRef : synonym | ‘_’ | INTEGER

std::string getStmtRefStringValue(const TOKEN& token) {
    switch (token.type) {
    case backend::lexer::INTEGER:
        return token.integerValue;
    case backend::lexer::UNDERSCORE:
        return "_";
    case backend::lexer::NAME:
        return token.nameValue;
    default:
        throw std::invalid_argument(kQppErrorPrefix + "getStmtRefStringValue: A non StmtRef token is supplied of type:" +
                                    backend::lexer::prettyPrintType(token.type));
    }
}

bool isStmtRefToken(const TOKEN& token) {
    switch (token.type) {
    case backend::lexer::INTEGER:
    case backend::lexer::UNDERSCORE:
    case backend::lexer::NAME:
        return true;
    default:
        return false;
    }
};

/**
 * UsesS : ... ‘(’ stmtRef ‘,’ entRef ‘)’
 * UsesP : ... ‘(’ entRef ‘,’ entRef ‘)’
 * ModifiesS : ... ‘(’ stmtRef ‘,’ entRef ‘)’
 * ModifiesP : ... ‘(’ entRef ‘,’ entRef ‘)’
 * @return <state of parser, isStateInvalid>
 */
STATESTATUSPAIR parseRelationStmtEntOrEntEnt(State state, qpbackend::RelationType relationType) {
    // Mutable variables in function.
    std::string stmtOrEntString;
    bool isValidState = true;
    std::string entString;
    // state argument is also mutable.

    TOKEN lParenToken = state.popUntilNonWhitespaceToken();
    if (lParenToken.type != backend::lexer::LPAREN || !state.hasTokensLeftToParse()) {
        logLine(kQppLogWarnPrefix +
                "parseRelationStmtEntOrEntEnt: Expected more tokens but finished "
                "consuming tokens or LPAREN token not found. Obtained " +
                backend::lexer::prettyPrintType(lParenToken.type));
        return STATESTATUSPAIR(state, false);
    }

    // Check the current token to see if it should be handled as an EntRef or StmtRef
    state.popIfCurrentTokenIsWhitespaceToken();
    if (!state.hasTokensLeftToParse()) {
        logLine(kQppLogWarnPrefix +
                "parseRelationStmtEntOrEntEnt: Expected more tokens but finished consuming tokens");
        return STATESTATUSPAIR(state, false);
    }
    TOKEN stmtOrEntToken = state.peekToken();
    if (isStmtRefToken(stmtOrEntToken)) {
        state.popToken();
        stmtOrEntString = getStmtRefStringValue(stmtOrEntToken);
    } else {
        std::tie(state, stmtOrEntString, isValidState) = parseEntRef(state);
    }
    if (!isValidState || !state.hasTokensLeftToParse()) {
        logLine(kQppLogWarnPrefix +
                "parseRelationStmtEntOrEntEnt: Expected more tokens but finished "
                "consuming tokens or STMT | ENT not found. Obtained " +
                backend::lexer::prettyPrintType(stmtOrEntToken.type));
        return STATESTATUSPAIR(state, false);
    }
    if (stmtOrEntString == "_") {
        logLine(kQppLogWarnPrefix + "parseRelationStmtEntOrEntEnt: Semantically invalid to have a"
                                    " UNDERSCORE('_') as USES/MODIFIES first argument ");
        return STATESTATUSPAIR(state, false);
    }

    TOKEN commaToken = state.popUntilNonWhitespaceToken();
    if (commaToken.type != backend::lexer::COMMA || !state.hasTokensLeftToParse()) {
        logLine(kQppLogWarnPrefix +
                "parseRelationStmtEntOrEntEnt: Expected more tokens but finished "
                "consuming tokens or COMMA token not found. Obtained " +
                backend::lexer::prettyPrintType(commaToken.type));
        return STATESTATUSPAIR(state, false);
    }

    std::tie(state, entString, isValidState) = parseEntRef(state);
    if (!isValidState || !state.hasTokensLeftToParse()) {
        logLine(kQppLogWarnPrefix +
                "parseRelationStmtEntOrEntEnt: Expected more tokens but finished "
                "consuming tokens or ENT not found. Obtained token of value:" +
                entString);
        return STATESTATUSPAIR(state, false);
    }

    TOKEN rParenToken = state.popUntilNonWhitespaceToken();
    if (rParenToken.type != backend::lexer::RPAREN) {
        logLine(kQppLogWarnPrefix + "parseRelationStmtEntOrEntEnt: RPAREN token not found. Obtained " +
                backend::lexer::prettyPrintType(rParenToken.type));
        return STATESTATUSPAIR(state, false);
    }
    state.addSuchThatClause(relationType, stmtOrEntString, entString);
    return STATESTATUSPAIR(state, true);
}

/**
 * entRef : synonym | ‘_’ | ‘"’ IDENT ‘"’
 * @return <state of parser, string representation of valid ent ref, isValidState>
 */
STATE_RESULT_STATUS_TRIPLE parseEntRef(State state) {
    TOKEN firstToken = state.popUntilNonWhitespaceToken();
    // Handle (synonym | '_')
    switch (firstToken.type) {
    case backend::lexer::NAME:
        return STATE_RESULT_STATUS_TRIPLE(state, firstToken.nameValue, true);
    case backend::lexer::UNDERSCORE:
        return STATE_RESULT_STATUS_TRIPLE(state, "_", true);
    }

    // Handle ‘"’ IDENT ‘"’
    if (firstToken.type != backend::lexer::DOUBLE_QUOTE || !state.hasTokensLeftToParse()) {
        logLine(kQppLogWarnPrefix + "parseEntRef: Either ran out of tokens or expected DOUBLE_QUOTE Token, found " +
                backend::lexer::prettyPrintType(firstToken.type));
        return STATE_RESULT_STATUS_TRIPLE(state, "", false);
    }
    TOKEN identToken = state.popToken();
    if (identToken.type != backend::lexer::NAME || !state.hasTokensLeftToParse()) {
        logLine(kQppLogWarnPrefix + "parseEntRef: Either ran out of tokens or expected NAME Token, found " +
                backend::lexer::prettyPrintType(identToken.type));
        return STATE_RESULT_STATUS_TRIPLE(state, "", false);
    }
    TOKEN closingDoubleQuoteToken = state.popToken();
    state.popIfCurrentTokenIsWhitespaceToken();
    if (closingDoubleQuoteToken.type != backend::lexer::DOUBLE_QUOTE || !state.hasTokensLeftToParse()) {
        logLine(kQppLogWarnPrefix + "parseEntRef: Either ran out of tokens or expected DOUBLE_QUOTE Token, found " +
                backend::lexer::prettyPrintType(closingDoubleQuoteToken.type));
        return STATE_RESULT_STATUS_TRIPLE(state, "", false);
    }
    std::string result = "\"";
    result += identToken.nameValue;
    result += "\"";
    return STATE_RESULT_STATUS_TRIPLE(state, result, true);
}

/**
 * pattern-cl : ‘pattern’ syn-assign ‘(‘ entRef ‘,’ expression-spec ’)’
 * syn-assign must be declared as synonym of assignment (design entity ‘assign’).
 * entRef : synonym | ‘_’ | ‘"’ IDENT ‘"’
 * expression-spec :  ‘"‘ expr’"’ | ‘_’ ‘"’ expr ‘"’ ‘_’ | ‘_’
 */
STATESTATUSPAIR parseSinglePatternClause(State state) {
    state.popToNextNonWhitespaceToken();
    if (!state.hasTokensLeftToParse()) return STATESTATUSPAIR(state, false);
    // Mutable vars
    bool isValidState;
    std::string entRefString;
    std::string expressionSpecString;
    // state is also mutable
    logLine(kQppLogInfoPrefix + "parseSinglePatternClause: Begin");
    TOKEN patternToken = state.popUntilNonWhitespaceToken();
    state.popIfCurrentTokenIsWhitespaceToken();
    if (patternToken.type != backend::lexer::NAME || patternToken.nameValue != kPatternKeyword ||
        !state.hasTokensLeftToParse()) {
        return STATESTATUSPAIR(state, false);
    }

    TOKEN synAssignToken = state.popUntilNonWhitespaceToken();
    state.popIfCurrentTokenIsWhitespaceToken();
    if (!isSynAssignToken(synAssignToken, state) || !state.hasTokensLeftToParse()) {
        return STATESTATUSPAIR(state, false);
    }

    TOKEN lParenToken = state.popUntilNonWhitespaceToken();
    if (lParenToken.type != backend::lexer::LPAREN || !state.hasTokensLeftToParse()) {
        return STATESTATUSPAIR(state, false);
    }

    std::tie(state, entRefString, isValidState) = parseEntRef(state);
    if (!isValidState) return STATESTATUSPAIR(state, false);

    TOKEN commaToken = state.popUntilNonWhitespaceToken();
    if (commaToken.type != backend::lexer::COMMA || !state.hasTokensLeftToParse()) {
        return STATESTATUSPAIR(state, false);
    }

    std::tie(state, expressionSpecString, isValidState) = parseExpressionSpec(state);
    if (!isValidState) return STATESTATUSPAIR(state, false);

    TOKEN rParenToken = state.popUntilNonWhitespaceToken();
    if (rParenToken.type != backend::lexer::RPAREN) {
        return STATESTATUSPAIR(state, false);
    }

    state.addPatternClause(synAssignToken.nameValue, entRefString, expressionSpecString);
    state.popIfCurrentTokenIsWhitespaceToken();
    logLine(kQppLogInfoPrefix + "parseSinglePatternClause: Success End");
    return STATESTATUSPAIR(state, true);
}

bool isSynAssignToken(const TOKEN& token, State& state) {
    if (token.type != backend::lexer::NAME) {
        return false;
    }
    const std::unordered_map<std::string, qpbackend::EntityType>& declarationMap = state.getQuery().declarationMap;
    auto declaration = declarationMap.find(token.nameValue);
    if (declaration == declarationMap.end()) {
        return false;
    }
    return declaration->second == qpbackend::EntityType::ASSIGN;
}

/**
 * The original grammar is:
 * expression-spec :  ‘"‘ expr’"’ | ‘_’ ‘"’ expr ‘"’ ‘_’ | ‘_’
 * expr will be parsed by the PKB's parser to build an AST tree.
 * @return _"<expr>"_ | "<expr>"
 */
STATE_RESULT_STATUS_TRIPLE parseExpressionSpec(State state) {
    const TOKEN& firstToken = state.popUntilNonWhitespaceToken();

    if (!state.hasTokensLeftToParse()) return STATE_RESULT_STATUS_TRIPLE(state, "", false);
    state.popToNextNonWhitespaceToken();
    const TOKEN& secondToken = state.peekToken();

    bool isSubExpression;
    int doubleQuotesPoppedCount = 0; // When doubleQuotesPoppedCount = 2, it means the end of the expr is reached.
    int nonWhitespaceTokensInExpr = 0;
    std::string expressionSpec;
    // TODO(https://github.com/nus-cs3203/team24-cp-spa-20s1/issues/231):
    //  Use switch case statements to optimise.
    if (firstToken.type == backend::lexer::DOUBLE_QUOTE) {
        isSubExpression = false;
        doubleQuotesPoppedCount += 1;
        expressionSpec += '"';
    } else if (firstToken.type == backend::lexer::UNDERSCORE && secondToken.type == backend::lexer::DOUBLE_QUOTE) {
        isSubExpression = true;
        expressionSpec += '_';
    } else if (firstToken.type == backend::lexer::UNDERSCORE) {
        return STATE_RESULT_STATUS_TRIPLE(state, "_", true);
    } else {
        return STATE_RESULT_STATUS_TRIPLE(state, "", false);
    }

    // Stringify all tokens that are in between the double quotes
    // "<stringify all tokens here>"
    while (doubleQuotesPoppedCount < 2) {
        const TOKEN& currToken = state.popToken();
        if (currToken.type != backend::lexer::WHITESPACE) {
            nonWhitespaceTokensInExpr += 1;
        }
        switch (currToken.type) {
        case backend::lexer::NAME:
            expressionSpec += currToken.nameValue;
            break;
        case backend::lexer::INTEGER:
            expressionSpec += currToken.integerValue;
            break;
        case backend::lexer::LPAREN:
            expressionSpec += '(';
            break;
        case backend::lexer::RPAREN:
            expressionSpec += ')';
            break;
        case backend::lexer::MULT:
            expressionSpec += '*';
            break;
        case backend::lexer::PLUS:
            expressionSpec += '+';
            break;
        case backend::lexer::MINUS:
            expressionSpec += '-';
            break;
        case backend::lexer::DIV:
            expressionSpec += '/';
            break;
        case backend::lexer::DOUBLE_QUOTE:
            expressionSpec += '"';
            doubleQuotesPoppedCount += 1;
            break;
        case backend::lexer::WHITESPACE:
            // Keep whitespace so that the SIMPLE parser can tell the difference between
            // "1 + 23" (valid) and "1 + 2 3" (invalid)
            expressionSpec += ' ';
            break;
        default:
            return STATE_RESULT_STATUS_TRIPLE(state, "", false);
        }
    }

    if (nonWhitespaceTokensInExpr == 0) {
        logLine(kQppLogWarnPrefix + "parseExpressionSpec: no EXPR is matched between 2 "
                                    "DOUBLE_QUOTE tokens.");
        return STATE_RESULT_STATUS_TRIPLE(state, "", false);
    }


    if (isSubExpression) {
        const TOKEN& endingUnderscoreToken = state.popUntilNonWhitespaceToken();
        if (endingUnderscoreToken.type != backend::lexer::UNDERSCORE) {
            logLine(kQppLogWarnPrefix +
                    "parseExpressionSpec: Missing ending UNDERSCORE for _\"expr\"_ group.");
            return STATE_RESULT_STATUS_TRIPLE(state, "", false);
        }
        expressionSpec += '_';
    }

    state.popIfCurrentTokenIsWhitespaceToken();
    return STATE_RESULT_STATUS_TRIPLE(state, expressionSpec, true);
}

// QueryPreprocessor API definitions.

/**
 * Parses tokens of a QPL query into a Query struct for easier processing.
 *
 * TODO(https://github.com/nus-cs3203/team24-cp-spa-20s1/issues/166):
 * tokens can be converted to an rvalue reference for optimization in the future.
 * `Tokens` are currently copied within state.
 *
 * @return A Query struct representing the valid QPL query. If the QPL query is invalid, return
 * an empty Query struct.
 */
qpbackend::Query parseTokens(const TOKENS& tokens) {
    State initialState = State(tokens);
    try {
        State completedState = parseSelect(initialState);
        logLine(kQppLogInfoPrefix + "parseTokens: completed parsing.\n" + completedState.getQuery().toString());
        return completedState.getQuery();
    } catch (const std::runtime_error& e) {
        logLine(e.what());
    } catch (const std::invalid_argument& e) {
        logLine(e.what());
    }
    return qpbackend::Query();
}

} // namespace querypreprocessor
