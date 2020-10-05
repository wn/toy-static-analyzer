#include "TestQEHelper.h"

namespace qpbackend {
namespace qetest {

std::string printVectorString(const std::vector<std::string>& lst) {
    std::string res;
    for (const std::string& s : lst) {
        res += s + ", ";
    }
    return res;
}

bool checkIfVectorOfStringMatch(const std::vector<std::string>& lst1, const std::vector<std::string>& lst2) {
    std::vector<std::string> copy1(lst1);
    std::vector<std::string> copy2(lst2);
    std::sort(copy1.begin(), copy1.end());
    std::sort(copy2.begin(), copy2.end());
    if (copy1 != copy2) {
        // This will print the error :)
        throw std::runtime_error("string dont match: " + printVectorString(copy1) + " | " +
                                 printVectorString(copy2));
    }
    return true;
}

const STATEMENT_NUMBER_LIST& PKBMock::getAllStatements() const {
    static std::vector<int> statements;
    switch (test_idx) {
    case 0:
        statements = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14 };
        break;
    case 1:
        statements = { 1, 2, 3, 4, 5, 6 };
        break;
    case 2:
        statements = { 1, 2, 3, 4, 5, 6, 7, 8, 9 };
        break;
    case 3:
        statements = { 1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12,
                       13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23 };
    }
    return statements;
}

const VARIABLE_NAME_LIST& PKBMock::getAllVariables() const {
    static std::vector<std::string> variables;
    switch (test_idx) {
    case 0:
        variables = { "count", "cenX", "cenY", "x", "y", "flag", "normSq" };
        break;
    case 1:
        variables = { "x", "y", "z" };
        break;
    case 2:
        variables = { "x", "y", "z", "m", "n", "count", "random", "a" };
    }
    return variables;
}

const PROCEDURE_NAME_LIST& PKBMock::getAllProcedures() const {
    static std::vector<std::string> procedures;
    switch (test_idx) {
    case 0:
        procedures = { "computeCentroid" };
        break;
    case 1:
        procedures = { "main" };
        break;
    case 2:
        procedures = { "foo", "bar" };
    }
    return procedures;
}

const CONSTANT_NAME_SET& PKBMock::getAllConstants() const {
    static std::unordered_set<std::string> constants;
    switch (test_idx) {
    case 0:
        constants = { "0", "1" };
    }
    return constants;
}

STATEMENT_NUMBER_LIST PKBMock::getDirectFollowedBy(STATEMENT_NUMBER s) const {
    std::vector<int> stmts;
    if (test_idx == 0) {
        switch (s) {
        case 2:
            stmts = { 1 };
            break;
        case 3:
            stmts = { 2 };
            break;
        case 4:
            stmts = { 3 };
            break;
        case 5:
            stmts = { 4 };
            break;
        case 10:
            stmts = { 5 };
            break;
        case 14:
            stmts = { 10 };
            break;
        case 7:
            stmts = { 6 };
            break;
        case 8:
            stmts = { 7 };
            break;
        case 9:
            stmts = { 8 };
            break;
        case 13:
            stmts = { 12 };
            break;
        default:
            stmts = {};
        }
    }
    return stmts;
}

STATEMENT_NUMBER_LIST PKBMock::getDirectFollow(STATEMENT_NUMBER s) const {
    std::vector<int> stmts;
    if (test_idx == 0) {
        switch (s) {
        case 1:
            stmts = { 2 };
            break;
        case 2:
            stmts = { 3 };
            break;
        case 3:
            stmts = { 4 };
            break;
        case 4:
            stmts = { 5 };
            break;
        case 5:
            stmts = { 10 };
            break;
        case 10:
            stmts = { 14 };
            break;
        case 6:
            stmts = { 7 };
            break;
        case 7:
            stmts = { 8 };
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

STATEMENT_NUMBER_LIST PKBMock::getStatementsThatFollows(STATEMENT_NUMBER s) const {
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

STATEMENT_NUMBER_LIST PKBMock::getStatementsFollowedBy(STATEMENT_NUMBER s) const {
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
            break;
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

STATEMENT_NUMBER_LIST PKBMock::getParent(STATEMENT_NUMBER statementNumber) const {
    std::vector<int> stmts;
    if (test_idx == 0) {
        switch (statementNumber) {
        case 6:
        case 7:
        case 8:
        case 9:
            stmts = { 5 };
            break;
        case 11:
        case 12:
        case 13:
            stmts = { 10 };
            break;
        default:
            stmts = {};
        }
    }
    if (test_idx == 1) {
        switch (statementNumber) {
        case 2:
            stmts = { 1 };
            break;
        case 3:
            stmts = { 2 };
            break;
        case 4:
            stmts = { 1 };
            break;
        case 5:
        case 6:
            stmts = { 4 };
            break;
        default:
            stmts = {};
        }
    }
    return stmts;
}

STATEMENT_NUMBER_LIST PKBMock::getChildren(STATEMENT_NUMBER statementNumber) const {
    std::vector<int> stmts;
    if (test_idx == 0) {
        switch (statementNumber) {
        case 5:
            stmts = { 6, 7, 8, 9 };
            break;
        case 10:
            stmts = { 11, 12, 13 };
            break;
        default:
            stmts = {};
        }
    }
    if (test_idx == 1) {
        switch (statementNumber) {
        case 1:
            stmts = { 2, 4 };
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

STATEMENT_NUMBER_LIST PKBMock::getAncestors(STATEMENT_NUMBER statementNumber) const {
    std::vector<int> stmts;
    if (test_idx == 0) {
        switch (statementNumber) {
        case 6:
        case 7:
        case 8:
        case 9:
            stmts = { 5 };
            break;
        case 11:
        case 12:
        case 13:
            stmts = { 10 };
            break;
        default:
            stmts = {};
        }
    }
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
        stmts = { 6, 7, 8, 9, 11, 12, 13 };
    }
    if (test_idx == 1) {
        stmts = { 2, 3, 4, 5, 6 };
    }
    return stmts;
}

STATEMENT_NUMBER_LIST PKBMock::getDescendants(STATEMENT_NUMBER statementNumber) const {
    std::vector<int> stmts;
    if (test_idx == 0) {
        switch (statementNumber) {
        case 5:
            stmts = { 6, 7, 8, 9 };
            break;
        case 10:
            stmts = { 11, 12, 13 };
            break;
        default:
            stmts = {};
        }
    }
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
    if (test_idx == 0) {
        stmts = { 5, 10 };
    }
    if (test_idx == 1) {
        stmts = { 1, 2, 4 };
    }
    return stmts;
}

STATEMENT_NUMBER_LIST PKBMock::getStatementsThatUse(VARIABLE_NAME v) const {
    std::vector<int> stmts;
    if (test_idx == 2) {
        if (v == "x") {
            stmts = { 1 };
        } else if (v == "z") {
            stmts = { 2, 4, 6 };
        } else if (v == "m") {
            stmts = { 2, 4, 6 };
        } else if (v == "n") {
            stmts = { 2, 4, 5 };
        } else if (v == "count") {
            stmts = { 2, 4 };
        }
    }
    return stmts;
}

STATEMENT_NUMBER_LIST PKBMock::getStatementsThatUseSomeVariable() const {
    std::vector<int> stmts;
    if (test_idx == 2) {
        stmts = { 1, 2, 4, 5, 6 };
    }
    return stmts;
}

PROCEDURE_NAME_LIST PKBMock::getProceduresThatUse(VARIABLE_NAME v) const {
    std::vector<std::string> procs;
    if (test_idx == 2) {
        if (v == "x") {
            procs = { "foo" };
        } else if (v == "z" || v == "m" || v == "n" || v == "count") {
            procs = { "foo", "bar" };
        }
    }
    return procs;
}

PROCEDURE_NAME_LIST PKBMock::getProceduresThatUseSomeVariable() const {
    std::vector<std::string> procs;
    if (test_idx == 2) {
        procs = { "foo", "bar" };
    }
    return procs;
}

VARIABLE_NAME_LIST PKBMock::getVariablesUsedIn(PROCEDURE_NAME p) const {
    std::vector<std::string> vars;
    if (test_idx == 2) {
        if (p == "foo") {
            vars = { "x", "z", "m", "n", "count" };
        } else if (p == "bar") {
            vars = { "z", "m", "n", "count" };
        }
    }
    return vars;
}

VARIABLE_NAME_LIST PKBMock::getVariablesUsedBySomeProcedure() const {
    std::vector<std::string> vars;
    if (test_idx == 2) {
        vars = { "x", "z", "m", "n", "count" };
    }
    return vars;
}

VARIABLE_NAME_LIST PKBMock::getVariablesUsedIn(STATEMENT_NUMBER s) const {
    std::vector<std::string> vars;
    if (test_idx == 2) {
        switch (s) {
        case 1:
            vars = { "x" };
            break;
        case 2:
        case 4:
            vars = { "z", "m", "n", "count" };
            break;
        case 5:
            vars = { "n" };
            break;
        case 6:
            vars = { "z", "m" };
        }
    }
    return vars;
}

VARIABLE_NAME_LIST PKBMock::getVariablesUsedBySomeStatement() const {
    std::vector<std::string> vars;
    if (test_idx == 2) {
        vars = { "x", "z", "m", "n", "count" };
    }
    return vars;
}

STATEMENT_NUMBER_LIST PKBMock::getStatementsThatModify(VARIABLE_NAME v) const {
    std::vector<int> stmts;
    if (test_idx == 2) {
        if (v == "random") {
            stmts = { 2, 3 };
        } else if (v == "n") {
            stmts = { 2, 4, 5 };
        } else if (v == "y") {
            stmts = { 2, 4, 6 };
        } else if (v == "a") {
            stmts = { 7, 8 };
        }
    }
    return stmts;
}
STATEMENT_NUMBER_LIST PKBMock::getStatementsThatModifySomeVariable() const {
    std::vector<int> stmts;
    if (test_idx == 2) {
        stmts = { 2, 3, 4, 5, 6, 7, 8, 9 };
    }
    return stmts;
}
PROCEDURE_NAME_LIST PKBMock::getProceduresThatModify(VARIABLE_NAME v) const {
    std::vector<std::string> procs;
    if (test_idx == 2) {
        if (v == "random" || v == "n" || v == "y") {
            procs = { "foo", "bar" };
        }
    }
    return procs;
}
PROCEDURE_NAME_LIST PKBMock::getProceduresThatModifySomeVariable() const {
    std::vector<std::string> procs;
    if (test_idx == 2) {
        procs = { "foo", "bar" };
    }
    return procs;
}
VARIABLE_NAME_LIST PKBMock::getVariablesModifiedBy(PROCEDURE_NAME p) const {
    std::vector<std::string> vars;
    if (test_idx == 2) {
        if (p == "foo") {
            vars = { "n", "random", "y", "a" };
        } else if (p == "bar") {
            vars = { "n", "random", "y", "a" };
        }
    }
    return vars;
}
VARIABLE_NAME_LIST PKBMock::getVariablesModifiedBySomeProcedure() const {
    std::vector<std::string> vars;
    if (test_idx == 2) {
        vars = { "n", "y", "random", "a" };
    }
    return vars;
}
VARIABLE_NAME_LIST PKBMock::getVariablesModifiedBy(STATEMENT_NUMBER s) const {
    std::vector<std::string> vars;
    if (test_idx == 2) {
        switch (s) {
        case 2:
            vars = { "n", "random", "y" };
            break;
        case 3:
            vars = { "random" };
            break;
        case 4:
            vars = { "n", "y" };
            break;
        case 5:
            vars = { "n" };
            break;
        case 6:
            vars = { "y" };
            break;
        case 7:
            vars = { "a" };
            break;
        case 8:
            vars = { "a" };
            break;
        case 9:
            vars = { "a" };
            break;
        }
    }
    return vars;
}
VARIABLE_NAME_LIST PKBMock::getVariablesModifiedBySomeStatement() const {
    std::vector<std::string> vars;
    if (test_idx == 2) {
        vars = { "n", "y", "random", "a" };
    }
    return vars;
}

STATEMENT_NUMBER_LIST
PKBMock::getAllAssignmentStatementsThatMatch(const std::string& assignee, const std::string& pattern, bool isSubExpr) const {
    if (test_idx == 0) {
        if (!isSubExpr) {
            if (assignee == "_") {
                if (pattern == "") {
                    throw std::runtime_error(
                    "getAllAssignmentStatementsThatMatch: This should not happen!");
                }
                if (pattern == "0") {
                    return { 1, 2, 3 };
                }
                if (assignee == "count" && pattern == "count+1") {
                    return { 6 };
                }
                if (assignee == "cenX" && pattern == "cenX+x") {
                    return { 7 };
                }
                if (assignee == "cenY" && pattern == "cenY+y") {
                    return { 8 };
                }
                if (assignee == "flag" && pattern == "1") {
                    return { 11 };
                }
                if (assignee == "cenX" && pattern == "cenX/count") {
                    return { 12 };
                }
                if (assignee == "cenY" && pattern == "cenY/count") {
                    return { 13 };
                }
                if (assignee == "normSq" && pattern == "cenX*cenX+cenY*cenY") {
                    return { 14 };
                }
                return {};
            }
            if (assignee == "count" && pattern == "0") {
                return { 1 };
            }
            if (assignee == "cenX" && pattern == "0") {
                return { 2 };
            }
            if (assignee == "cenY" && pattern == "0") {
                return { 3 };
            }
            if (assignee == "count" && pattern == "count+1") {
                return { 6 };
            }
            if (assignee == "cenX" && pattern == "cenX+x") {
                return { 7 };
            }
            if (assignee == "cenY" && pattern == "cenY+y") {
                return { 8 };
            }
            if (assignee == "flag" && pattern == "1") {
                return { 11 };
            }
            if (assignee == "cenX" && pattern == "cenX/count") {
                return { 12 };
            }
            if (assignee == "cenY" && pattern == "cenY/count") {
                return { 13 };
            }
            if (assignee == "normSq" && pattern == "cenX*cenX+cenY*cenY") {
                return { 14 };
            }
            return {};
        }
        if (assignee == "_") {
            if (pattern == "") {
                return { 1, 2, 3, 6, 7, 8, 11, 12, 13, 14 };
            }
            if (pattern == "cenX") {
                return { 7, 12, 14 };
            }
            if (pattern == "cenY") {
                return { 8, 13, 14 };
            }
            if (pattern == "0") {
                return { 1, 2, 3 };
            }
            if (pattern == "1") {
                return { 6, 11 };
            }
            if (pattern == "count") {
                return { 6, 12, 13 };
            }
            if (pattern == "x") {
                return { 7 };
            }
            if (pattern == "y") {
                return { 8 };
            }
            return {};
        }
        if (pattern == "") {
            if (assignee == "count") {
                return { 1, 6 };
            }
            if (assignee == "cenX") {
                return { 2, 7, 12 };
            }
            if (assignee == "cenY") {
                return { 3, 8, 13 };
            }
            if (assignee == "normSq") {
                return { 14 };
            }
            if (assignee == "flag") {
                return { 11 };
            }
            return {};
        }

        if (assignee == "count" && pattern == "0") {
            return { 1 };
        }
        if (assignee == "cenX" && pattern == "0") {
            return { 2 };
        }
        if (assignee == "cenY" && pattern == "0") {
            return { 3 };
        }
        if (assignee == "count" && pattern == "count+1") {
            return { 6 };
        }
        if (assignee == "count" && pattern == "count") {
            return { 6 };
        }
        if (assignee == "count" && pattern == "1") {
            return { 6 };
        }
        if (assignee == "cenX" && pattern == "cenX+x") {
            return { 7 };
        }
        if (assignee == "cenX" && pattern == "cenX") {
            return { 7, 12 };
        }
        if (assignee == "cenX" && pattern == "x") {
            return { 7 };
        }
        if (assignee == "cenY" && pattern == "cenY+y") {
            return { 8 };
        }
        if (assignee == "cenY" && pattern == "cenY") {
            return { 8, 13 };
        }
        if (assignee == "cenY" && pattern == "y") {
            return { 8 };
        }
        if (assignee == "flag" && pattern == "1") {
            return { 11 };
        }
        if (assignee == "cenX" && pattern == "cenX/count") {
            return { 12 };
        }
        if (assignee == "cenX" && pattern == "cenX/count") {
            return { 12 };
        }
        if (assignee == "cenY" && pattern == "cenY/count") {
            return { 13 };
        }
        if (assignee == "cenY" && pattern == "count") {
            return { 13 };
        }
        if (assignee == "normSq" && pattern == "cenX*cenX+cenY*cenY") {
            return { 14 };
        }
        if (assignee == "normSq" && pattern == "cenX*cenX") {
            return { 14 };
        }
        if (assignee == "normSq" && pattern == "cenY*cenY") {
            return { 14 };
        }
        if (assignee == "normSq" && pattern == "cenX") {
            return { 14 };
        }
        if (assignee == "normSq" && pattern == "cenY") {
            return { 14 };
        }
        return {};
    }

    if (test_idx == 2) {
        if (isSubExpr) {
            if (pattern == "n") {
                if (assignee == "n" || assignee == "_") {
                    return { 5 };
                }
            } else if (pattern == "1") {
                if (assignee == "n") {
                    return { 5 };
                } else if (assignee == "a") {
                    return { 9 };
                } else if (assignee == "_") {
                    return { 5, 9 };
                }
            } else if (pattern == "z") {
                if (assignee == "y" || assignee == "_") {
                    return { 6 };
                }
            } else if (pattern == "m") {
                if (assignee == "y" || assignee == "_") {
                    return { 6 };
                }
            } else if (pattern == "") {
                if (assignee == "n") {
                    return { 5 };
                } else if (assignee == "y") {
                    return { 6 };
                } else if (assignee == "a") {
                    return { 9 };
                } else if (assignee == "_") {
                    return { 5, 6, 9 };
                }
            }
        } else {
            if (pattern == "n+1") {
                if (assignee == "n" || assignee == "_") {
                    return { 5 };
                } else if (assignee == "y" || assignee == "_") {
                    return { 6 };
                } else if (assignee == "a" || assignee == "_") {
                    return { 9 };
                }
            }
        }
    }

    return STATEMENT_NUMBER_LIST();
}

bool PKBMock::isRead(STATEMENT_NUMBER s) const {
    if (test_idx == 2) {
        switch (s) {
        case 3:
            return true;
        }
    }
    if (test_idx == 3) {
        switch (s) {
        case 4:
        case 5:
            return true;
        }
    }
    return false;
}
bool PKBMock::isPrint(STATEMENT_NUMBER s) const {
    if (test_idx == 1) {
        switch (s) {
        case 1:
            return true;
        }
    }
    if (test_idx == 3) {
        switch (s) {
        case 6:
        case 7:
        case 8:
        case 9:
            return true;
        }
    }
    return false;
}
bool PKBMock::isCall(STATEMENT_NUMBER s) const {
    if (test_idx == 0) {
        switch (s) {
        case 4:
        case 9:
            return true;
        }
    }
    if (test_idx == 2) {
        switch (s) {
        case 2:
            return true;
        }
    }
    if (test_idx == 3) {
        switch (s) {
        case 2:
        case 3:
        case 13:
        case 18:
            return true;
        }
    }
    return false;
}
bool PKBMock::isWhile(STATEMENT_NUMBER s) const {
    if (test_idx == 0) {
        switch (s) {
        case 5:
            return true;
        }
    }
    if (test_idx == 3) {
        switch (s) {
        case 14:
            return true;
        }
    }
    if (test_idx == 2) {
        switch (s) {
        case 7:
        case 8:
            return true;
        }
    }
    return false;
}
bool PKBMock::isIfElse(STATEMENT_NUMBER s) const {
    if (test_idx == 0) {
        switch (s) {
        case 10:
            return true;
        }
    }
    if (test_idx == 3) {
        switch (s) {
        case 19:
            return true;
        }
    }
    if (test_idx == 2) {
        switch (s) {
        case 4:
            return true;
        }
    }
    return false;
}
bool PKBMock::isAssign(STATEMENT_NUMBER s) const {
    if (test_idx == 0) {
        switch (s) {
        case 1:
        case 2:
        case 3:
        case 6:
        case 7:
        case 8:
        case 11:
        case 12:
        case 13:
        case 14:
            return true;
        }
    }
    if (test_idx == 2) {
        switch (s) {
        case 5:
        case 6:
        case 9:
            return true;
        }
    }
    if (test_idx == 3) {
        switch (s) {
        case 1:
        case 10:
        case 11:
        case 12:
        case 15:
        case 16:
        case 17:
        case 20:
        case 21:
        case 22:
        case 23:
            return true;
        }
    }
    return false;
}
PROCEDURE_NAME_SET PKBMock::getProcedureThatCalls(const VARIABLE_NAME& procedureName, bool isTransitive) const {
    return PROCEDURE_NAME_SET();
}

PROCEDURE_NAME_SET PKBMock::getProceduresCalledBy(const VARIABLE_NAME& procedureName, bool isTransitive) const {
    return PROCEDURE_NAME_SET();
}

STATEMENT_NUMBER_SET PKBMock::getNextStatementOf(STATEMENT_NUMBER statementNumber, bool isTransitive) const {
    return STATEMENT_NUMBER_SET();
}

STATEMENT_NUMBER_SET PKBMock::getPreviousStatementOf(STATEMENT_NUMBER statementNumber, bool isTransitive) const {
    return STATEMENT_NUMBER_SET();
}


} // namespace qetest
} // namespace qpbackend
