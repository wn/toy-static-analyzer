#pragma once

#include "DesignExtractor.h"
#include "PKB.h"
#include "PKBImplementation.h"
#include "Parser.h"

#include <iostream>
#include <string>
#include <vector>

typedef std::string PROCEDURE;
typedef std::vector<std::string> PROCEDURE_LIST;
typedef std::string VARIABLE;
typedef std::vector<std::string> VARIABLE_LIST;
typedef int STATEMENT_NUMBER;
typedef std::vector<STATEMENT_NUMBER> STATEMENT_NUMBER_LIST;

namespace backend {
class PKBImplementation : public PKB {
  public:
    explicit PKBImplementation(const TNode& ast);
    STATEMENT_NUMBER_LIST getPreFollows(STATEMENT_NUMBER s) override;
    STATEMENT_NUMBER_LIST getPostFollows(STATEMENT_NUMBER s) override;

    STATEMENT_NUMBER_LIST getAllStatements() override;

    VARIABLE_LIST getAllVariables() override;

    PROCEDURE_LIST getAllProcedures() override;

    STATEMENT_NUMBER_LIST getStatementsFollowedBy(STATEMENT_NUMBER s) override;
    STATEMENT_NUMBER_LIST getStatementsFollowed() override;
    STATEMENT_NUMBER_LIST getStatementsThatFollows(STATEMENT_NUMBER s) override;
    STATEMENT_NUMBER_LIST getStatementsFollows() override;

    STATEMENT_NUMBER_LIST getAncestors(STATEMENT_NUMBER statementNumber) override;
    STATEMENT_NUMBER_LIST getStatementsThatHaveAncestors() override;
    STATEMENT_NUMBER_LIST getDescendants(STATEMENT_NUMBER statementNumber) override;
    STATEMENT_NUMBER_LIST getStatementsThatHaveDescendants() override;

  private:
    STATEMENT_NUMBER_LIST getStatementsInSameBlock(STATEMENT_NUMBER s);
};
} // namespace backend
