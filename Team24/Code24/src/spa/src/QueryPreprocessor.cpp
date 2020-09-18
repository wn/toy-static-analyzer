#include "QueryPreprocessor.h"

#include "Lexer.h"
#include "Logger.h"
#include "Query.h"

#include <tuple>
#include <utility>
#include <vector>

// Constants
std::string const kQppErrorPrefix = "Log[Error-QueryPreprocessor]: ";
std::string const kQppLogInfoPrefix = "Log[INFO-QueryPreprocessor]: ";

namespace querypreprocessor {

// Declarations.

class State;
typedef std::pair<State, bool> STATESTATUSPAIR;
void throwIfTokenDoesNotHaveExpectedTokenType(backend::lexer::TokenType expectedTokenType, const TOKEN& token);
static qpbackend::EntityType getEntityTypeFromToken(const TOKEN& token);
State parseSelect(State state);
State parseDeclarations(State state);
STATESTATUSPAIR parseSingleDeclaration(State state);
bool isValidDeclarationDelimiter(const TOKEN& token);

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

    backend::lexer::Token popToken() {
        if (tokenPos >= tokens.size()) {
            throw std::runtime_error(kQppErrorPrefix + "State::popToken: QueryPreprocessor has "
                                                       "not successfully parsed a Query yet, "
                                                       "but has run out of tokens to parse.");
        }
        TOKEN tokenToReturn = tokens[tokenPos];
        tokenPos += 1;
        return tokenToReturn;
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
};
// Parser / Business logic methods

/**
 * select-cl : declaration* ‘Select’ synonym ([ suchthat-cl ] | [ pattern-cl ])*
 */
State parseSelect(State state) {
    state = parseDeclarations(state);
    logLine(kQppLogInfoPrefix + "parseSelect: Query state after parsing declaration*" +
            state.getQuery().toString());
    const TOKEN& selectToken = state.popToken();
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

    const TOKEN& synonymToken = state.popToken();
    state.addSynonymToReturn(synonymToken);
    // TODO(https://github.com/nus-cs3203/team24-cp-spa-20s1/issues/72): parse suchthat-cl
    // TODO(https://github.com/nus-cs3203/team24-cp-spa-20s1/issues/73): parse pattern-cl
    return state;
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
    const TOKEN& designEntity = state.popToken();
    if (designEntity.type != backend::lexer::NAME || !qpbackend::isEntityString(designEntity.nameValue)) {
        return STATESTATUSPAIR(state, false);
    }
    qpbackend::EntityType entityType = getEntityTypeFromToken(designEntity);

    TOKEN synonym = state.popToken();
    TOKEN delimiter = state.popToken();
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
        synonym = state.popToken();
        delimiter = state.popToken();
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

// QueryPreprocessor API definitions.

/**
 * Parses tokens of a QPL query into a Query struct for easier processing.
 *
 * TODO(https://github.com/nus-cs3203/team24-cp-spa-20s1/issues/166):
 * tokens can be converted to an rvalue reference for optimization in the future.
 * `Tokens` are currently copied within state.
 *
 * @return A Query struct representing the valid QPL query. If the QPL query is invalid, return an
 * empty Query struct.
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
