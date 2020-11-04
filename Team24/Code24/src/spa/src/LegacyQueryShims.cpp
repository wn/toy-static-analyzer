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

        qpbackend::ReturnType returnType = DEFAULT_VAL;
        returnCandidates.emplace_back(returnType, nameValue);
    }
    return returnCandidates;
}
} // namespace qpbackend
