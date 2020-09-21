#include "TestQEHelper.h"

namespace qpbackend {
namespace qetest {
STATEMENT_NUMBER_LIST PKBMock::getAllStatements() const {
    std::vector<int> statements;
    switch (test_idx) {
    case 0:
        statements = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14 };
        break;
    }
    return statements;
}

VARIABLE_LIST PKBMock::getAllVariables() const {
    std::vector<std::string> variables;
    switch (test_idx) {
    case 0:
        variables = { "count", "cenX", "cenY", "x", "y", "flag", "normSq" };
    }
    return variables;
}

PROCEDURE_LIST PKBMock::getAllProcedures() const {
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
        if (s == 6) {
            stmts = { 7, 8, 9 };
        }
        if (s == 14) {
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
        if (s == 4) {
            stmts = { 1, 2, 3 };
        }
        if (s == 1) {
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
    if (test_idx == 0) {
        if (statementNumber == 6) {
            stmts = { 5 };
        }
        if (statementNumber == 2) {
            stmts = {};
        }
    }
    return stmts;
}

STATEMENT_NUMBER_LIST PKBMock::getStatementsThatHaveAncestors() const {
    std::vector<int> stmts;
    if (test_idx == 0) {
        stmts = { 6, 7, 8, 9, 11, 12, 13 };
    }
    return stmts;
}

STATEMENT_NUMBER_LIST PKBMock::getDescendants(STATEMENT_NUMBER statementNumber) const {
    std::vector<int> stmts;
    if (test_idx == 0) {
        if (statementNumber == 5) {
            stmts = { 6, 7, 8, 9 };
        }
        if (statementNumber == 3) {
            stmts = {};
        }
    }
    return stmts;
}

STATEMENT_NUMBER_LIST PKBMock::getStatementsThatHaveDescendants() const {
    std::vector<int> stmts;
    if (test_idx == 0) {
        stmts = { 5, 10 };
    }
    return stmts;
}

STATEMENT_NUMBER_LIST PKBMock::getStatementsThatUse(VARIABLE_NAME v) const {
    return STATEMENT_NUMBER_LIST();
}
STATEMENT_NUMBER_LIST PKBMock::getStatementsThatUseSomeVariable() const {
    return STATEMENT_NUMBER_LIST();
}
PROCEDURE_LIST PKBMock::getProceduresThatUse(STATEMENT_NUMBER s) const {
    return PROCEDURE_LIST();
}
PROCEDURE_LIST PKBMock::getProceduresThatUseSomeVariable() const {
    return PROCEDURE_LIST();
}
VARIABLE_LIST PKBMock::getVariablesUsedIn(PROCEDURE_NAME p) const {
    return VARIABLE_LIST();
}
VARIABLE_LIST PKBMock::getVariablesUsedBySomeProcedure() const {
    return VARIABLE_LIST();
}
VARIABLE_LIST PKBMock::getVariablesUsedIn(STATEMENT_NUMBER s) const {
    return VARIABLE_LIST();
}
VARIABLE_LIST PKBMock::getVariablesUsedBySomeStatement() const {
    return VARIABLE_LIST();
}

STATEMENT_NUMBER_LIST PKBMock::getStatementsThatModify(VARIABLE_NAME v) const {
    return STATEMENT_NUMBER_LIST();
}
STATEMENT_NUMBER_LIST PKBMock::getStatementsThatModifySomeVariable() const {
    return STATEMENT_NUMBER_LIST();
}
PROCEDURE_LIST PKBMock::getProceduresThatModify(VARIABLE_NAME v) const {
    return PROCEDURE_LIST();
}
PROCEDURE_LIST PKBMock::getProceduresThatModifySomeVariable() const {
    return PROCEDURE_LIST();
}
VARIABLE_LIST PKBMock::getVariablesModifiedBy(PROCEDURE_NAME p) const {
    return VARIABLE_LIST();
}
VARIABLE_LIST PKBMock::getVariablesModifiedBySomeProcedure() const {
    return VARIABLE_LIST();
}
VARIABLE_LIST PKBMock::getVariablesModifiedBy(STATEMENT_NUMBER s) const {
    return VARIABLE_LIST();
}
VARIABLE_LIST PKBMock::getVariablesModifiedBySomeStatement() const {
    return VARIABLE_LIST();
}

} // namespace qetest
} // namespace qpbackend
