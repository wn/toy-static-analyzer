#include "TestQEHelper.h"

namespace qpbackend {
namespace qetest {
std::vector<std::string> convertStrToVector(std::string const& str) {
    std::vector<std::string> v;
    std::string delimiter = ", ";
    std::string remain = str;
    std::string token;
    size_t pos = 0;

    while ((pos = remain.find(delimiter)) != std::string::npos) {
        token = remain.substr(0, pos);
        v.push_back(token);
        remain.erase(0, pos + delimiter.length());
    }

    v.push_back(remain);
    return v;
}

bool checkIfVectorOfStringMatch(std::string const& str1, std::string const& str2) {
    std::vector<std::string> str1_vec = convertStrToVector(str1);
    std::vector<std::string> str2_vec = convertStrToVector(str2);
    std::sort(str1_vec.begin(), str1_vec.end());
    std::sort(str2_vec.begin(), str2_vec.end());
    return str1_vec == str2_vec;
}

STATEMENT_NUMBER_LIST PKBMock::getAllStatements() const {
    std::vector<int> statements;
    switch (test_idx) {
    case 0:
        statements = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14 };
        break;
    case 1:
        statements = { 1, 2, 3, 4, 5, 6 };
    }
    return statements;
}

VARIABLE_NAME_LIST PKBMock::getAllVariables() const {
    std::vector<std::string> variables;
    switch (test_idx) {
    case 0:
        variables = { "count", "cenX", "cenY", "x", "y", "flag", "normSq" };
    }
    return variables;
}

PROCEDURE_NAME_LIST PKBMock::getAllProcedures() const {
    std::vector<std::string> procedures;
    switch (test_idx) {
    case 0:
        procedures = { "computeCentroid" };
    }
    return procedures;
}

STATEMENT_NUMBER_LIST PKBMock::getPreFollows(STATEMENT_NUMBER s) const {
    return STATEMENT_NUMBER_LIST();
}

STATEMENT_NUMBER_LIST PKBMock::getPostFollows(STATEMENT_NUMBER s) const {
    return STATEMENT_NUMBER_LIST();
}

STATEMENT_NUMBER_LIST PKBMock::getStatementsFollowedBy(STATEMENT_NUMBER s) const {
    std::vector<int> stmts;
    if (test_idx == 0) {
        switch (s) {
        case 1:
            stmts = { 2, 3, 4, 5, 10, 14 };
            break;
        case 2:
            stmts = { 3, 4, 5, 10, 14 };
            break;
        case 3:
            stmts = { 4, 5, 10, 14 };
            break;
        case 4:
            stmts = { 5, 10, 14 };
            break;
        case 5:
            stmts = { 10, 14 };
            break;
        case 10:
            stmts = { 14 };
            break;
        case 6:
            stmts = { 7, 8, 9 };
            break;
        case 7:
            stmts = { 8, 9 };
            break;
        case 8:
            stmts = { 9 };
            break;
        case 12:
            stmts = { 13 };
            break;
        default:
            stmts = {};
        }
    }
    return stmts;
}

STATEMENT_NUMBER_LIST PKBMock::getAllStatementsThatAreFollowed() const {
    std::vector<int> stmts;
    if (test_idx == 0) {
        stmts = { 1, 2, 3, 4, 5, 6, 7, 8, 10, 12 };
    }
    return stmts;
}

STATEMENT_NUMBER_LIST PKBMock::getStatementsThatFollows(STATEMENT_NUMBER s) const {
    std::vector<int> stmts;
    if (test_idx == 0) {
        switch (s) {
        case 2:
            stmts = { 1 };
            break;
        case 3:
            stmts = { 1, 2 };
            break;
        case 4:
            stmts = { 1, 2, 3 };
            break;
        case 5:
            stmts = { 1, 2, 3, 4 };
            break;
        case 10:
            stmts = { 1, 2, 3, 4, 5 };
            break;
        case 14:
            stmts = { 1, 2, 3, 4, 5, 10 };
            break;
        case 7:
            stmts = { 6 };
            break;
        case 8:
            stmts = { 6, 7 };
            break;
        case 9:
            stmts = { 6, 7, 8 };
            break;
        case 13:
            stmts = { 12 };
        default:
            stmts = {};
        }
    }
    return stmts;
}

STATEMENT_NUMBER_LIST PKBMock::getAllStatementsThatFollows() const {
    std::vector<int> stmts;
    if (test_idx == 0) {
        stmts = { 2, 3, 4, 5, 7, 8, 9, 10, 13, 14 };
    }
    return stmts;
}

STATEMENT_NUMBER_LIST PKBMock::getAncestors(STATEMENT_NUMBER statementNumber) const {
    std::vector<int> stmts;
    if (test_idx == 1) {
        switch (statementNumber) {
        case 2:
            stmts = { 1 };
            break;
        case 3:
            stmts = { 2, 1 };
            break;
        case 4:
            stmts = { 1 };
            break;
        case 5:
        case 6:
            stmts = { 4, 1 };
            break;
        default:
            stmts = {};
        }
    }
    return stmts;
}

STATEMENT_NUMBER_LIST PKBMock::getStatementsThatHaveAncestors() const {
    std::vector<int> stmts;
    if (test_idx == 1) {
        stmts = { 2, 3, 4, 5, 6 };
    }
    return stmts;
}

STATEMENT_NUMBER_LIST PKBMock::getDescendants(STATEMENT_NUMBER statementNumber) const {
    std::vector<int> stmts;
    if (test_idx == 1) {
        switch (statementNumber) {
        case 1:
            stmts = { 2, 3, 4, 5, 6 };
            break;
        case 2:
            stmts = { 3 };
            break;
        case 4:
            stmts = { 5, 6 };
            break;
        default:
            stmts = {};
        }
    }
    return stmts;
}

STATEMENT_NUMBER_LIST PKBMock::getStatementsThatHaveDescendants() const {
    std::vector<int> stmts;
    if (test_idx == 1) {
        stmts = { 1, 2, 4 };
    }
    return stmts;
}

STATEMENT_NUMBER_LIST PKBMock::getStatementsThatUse(VARIABLE_NAME v) const {
    return STATEMENT_NUMBER_LIST();
}
STATEMENT_NUMBER_LIST PKBMock::getStatementsThatUseSomeVariable() const {
    return STATEMENT_NUMBER_LIST();
}
PROCEDURE_NAME_LIST PKBMock::getProceduresThatUse(VARIABLE_NAME v) const {
    return PROCEDURE_NAME_LIST();
}
PROCEDURE_NAME_LIST PKBMock::getProceduresThatUseSomeVariable() const {
    return PROCEDURE_NAME_LIST();
}
VARIABLE_NAME_LIST PKBMock::getVariablesUsedIn(PROCEDURE_NAME p) const {
    return VARIABLE_NAME_LIST();
}
VARIABLE_NAME_LIST PKBMock::getVariablesUsedBySomeProcedure() const {
    return VARIABLE_NAME_LIST();
}
VARIABLE_NAME_LIST PKBMock::getVariablesUsedIn(STATEMENT_NUMBER s) const {
    return VARIABLE_NAME_LIST();
}
VARIABLE_NAME_LIST PKBMock::getVariablesUsedBySomeStatement() const {
    return VARIABLE_NAME_LIST();
}

STATEMENT_NUMBER_LIST PKBMock::getStatementsThatModify(VARIABLE_NAME v) const {
    return STATEMENT_NUMBER_LIST();
}
STATEMENT_NUMBER_LIST PKBMock::getStatementsThatModifySomeVariable() const {
    return STATEMENT_NUMBER_LIST();
}
PROCEDURE_NAME_LIST PKBMock::getProceduresThatModify(VARIABLE_NAME v) const {
    return PROCEDURE_NAME_LIST();
}
PROCEDURE_NAME_LIST PKBMock::getProceduresThatModifySomeVariable() const {
    return PROCEDURE_NAME_LIST();
}
VARIABLE_NAME_LIST PKBMock::getVariablesModifiedBy(PROCEDURE_NAME p) const {
    return VARIABLE_NAME_LIST();
}
VARIABLE_NAME_LIST PKBMock::getVariablesModifiedBySomeProcedure() const {
    return VARIABLE_NAME_LIST();
}
VARIABLE_NAME_LIST PKBMock::getVariablesModifiedBy(STATEMENT_NUMBER s) const {
    return VARIABLE_NAME_LIST();
}
VARIABLE_NAME_LIST PKBMock::getVariablesModifiedBySomeStatement() const {
    return VARIABLE_NAME_LIST();
}

} // namespace qetest
} // namespace qpbackend
