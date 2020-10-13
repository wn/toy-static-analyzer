#include "LegacyQueryShims.h"

#include "Lexer.h"
#include "QPTypes.h"

namespace qpbackend {

const RETURN_CANDIDATE_LIST convertToReturnValues(const std::vector<std::string>& nameValues,
                                                  const qpbackend::DECLARATION_MAP& declarationMap) {
    RETURN_CANDIDATE_LIST returnCandidates;
    for (const std::string& nameValue : nameValues) {
        if (declarationMap.find(nameValue) == declarationMap.end()) {
            throw std::runtime_error("convertToReturnValues: Cannot return values for synonym " +
                                     nameValue + " as it has not been declared.");
        }

        qpbackend::ReturnType returnType;
        switch (declarationMap.at(nameValue)) {
        case qpbackend::ASSIGN:
            returnType = qpbackend::ASSIGN_STMT_NO;
            break;
        case qpbackend::STMT:
            returnType = qpbackend::STMT_STMT_NO;
            break;
        case qpbackend::READ:
            returnType = qpbackend::READ_STMT_NO;
            break;
        case qpbackend::PRINT:
            returnType = qpbackend::PRINT_STMT_NO;
            break;
        case qpbackend::CALL:
            returnType = qpbackend::CALL_STMT_NO;
            break;
        case qpbackend::WHILE:
            returnType = qpbackend::WHILE_STMT_NO;
            break;
        case qpbackend::IF:
            returnType = qpbackend::IF_STMT_NO;
            break;
        case qpbackend::VARIABLE:
            returnType = qpbackend::VAR_VAR_NAME;
            break;
        case qpbackend::CONSTANT:
            returnType = qpbackend::CONSTANT_VALUE;
            break;
        case qpbackend::PROCEDURE:
            returnType = qpbackend::PROC_PROC_NAME;
            break;
        }

        returnCandidates.emplace_back(returnType, nameValue);
    }
    return returnCandidates;
}
} // namespace qpbackend
