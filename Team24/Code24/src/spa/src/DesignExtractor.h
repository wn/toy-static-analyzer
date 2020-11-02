#pragma once

#include "PKB.h"
#include "TNode.h"

#include <unordered_map>
#include <unordered_set>

namespace backend {
namespace extractor {

/**
 * Returns the Uses mapping of the program.
 * @param tNodeTypeToTNodes
 * @return a Map where the TNode keys are in the domain of the Uses relation, and they map to
 * variables used by the TNode.
 */
std::unordered_map<const TNode*, std::unordered_set<std::string>>
getUsesMapping(std::unordered_map<TNodeType, std::vector<const TNode*>, EnumClassHash>& tNodeTypeToTNodes);

/**
 * Returns the Modifies mapping of the program.
 * @param tNodeTypeToTNodes
 * @return a Map where the TNode keys are in the domain of the Modifies relation, and they map to
 * variables modified by the TNode.
 */
std::unordered_map<const TNode*, std::unordered_set<std::string>>
getModifiesMapping(std::unordered_map<TNodeType, std::vector<const TNode*>, EnumClassHash>& tNodeTypeToTNodes);

std::unordered_map<const TNode*, std::unordered_set<const TNode*>>
getProcedureToCallees(const std::unordered_map<TNodeType, std::vector<const TNode*>, EnumClassHash>& tNodeTypeToTNodes);

const TNode* getProcedureFromProcedureName(
const std::string& procedureName,
const std::unordered_map<TNodeType, std::vector<const TNode*>, EnumClassHash>& tNodeTypeToTNodes);

std::unordered_map<const TNode*, int> getTNodeToStatementNumber(const TNode& ast);
std::unordered_map<int, const TNode*>
getStatementNumberToTNode(const std::unordered_map<const TNode*, int>& tNodeToStatementNumber);
std::unordered_map<TNodeType, std::vector<const TNode*>, EnumClassHash> getTNodeTypeToTNodes(const TNode& ast);

bool isValidSimpleProgram(const TNode& ast);

// Return a pair of mapping {{follower : followed}, {followed, follower}}.
std::pair<std::unordered_map<int, int>, std::unordered_map<int, int>> getFollowRelationship(const TNode& ast);
// Return a pair of mapping {{child : parent}, {parent, children}}.
std::pair<std::unordered_map<int, int>, std::unordered_map<int, STATEMENT_NUMBER_SET>>
getParentRelationship(const TNode& ast);

template <typename T> STATEMENT_NUMBER_SET getKeysInMap(const std::unordered_map<int, T>& map) {
    STATEMENT_NUMBER_SET result;
    for (auto it = map.begin(); it != map.end(); ++it) {
        result.insert(it->first);
    }
    return result;
}

STATEMENT_NUMBER_SET getVisitedPathFromStart(int start, const std::unordered_map<int, int>& relation);

/**
 * Get a mapping of pattern to {assignee, stmtNumber, isSubExpr}
 *
 * Note:
 * - isSubExpr - boolean of whether the visiting tNode is a sub expression
 * - stmtNo - the statement number of the assignee and expression.
 * - assignee - for each assign statement in the AST, the assignee is the variable on
 *   the left of the assignment statement. i.e. the "a" in a = 1 + 1;
 */
std::unordered_map<std::string, std::vector<std::tuple<std::string, int, bool>>>
getPatternsMap(const std::vector<const TNode*>& assignTNodes,
               const std::unordered_map<const TNode*, int>& tNodeToStatementNumber);

std::unordered_map<int, TNodeType>
getStatementNumberToTNodeTypeMap(const std::unordered_map<int, const TNode*>& statementNumberToTNode);

/**
 * Get mapping of the possible statements that a statement can go next.
 */
std::unordered_map<int, std::unordered_set<int>>
getNextRelationship(const std::unordered_map<TNodeType, std::vector<const TNode*>, EnumClassHash>& tNodeTypeToTNode,
                    const std::unordered_map<const TNode*, int>& tNodeToStatementNumber);

// Represents an edge on the NextBip graph.
// The label represents the program line of the call statement that
// links two procedures back and forth.
class NextBipEdge {
  public:
    NextBipEdge(PROGRAM_LINE prevLine, PROGRAM_LINE nextLine, PROGRAM_LINE label = 0)
    : prevLine(prevLine), nextLine(nextLine), label(label) {
    }
    PROGRAM_LINE prevLine;
    PROGRAM_LINE nextLine;
    PROGRAM_LINE label;

    bool operator==(const NextBipEdge& other) const {
        return prevLine == other.prevLine && nextLine == other.nextLine && label == other.label;
    }

    NextBipEdge reversed() const {
        return { nextLine, prevLine, label };
    }
};

/**
 * Generates the NextBip relationship, that relates program lines to other program lines with
 * labels on the edge. In NextBip, a negative program line corresponds to a virtual
 * end-node of a procedure, i.e. a node that denotes the end of the procedure execution.
 * @return the NextBip relation, as well as any created virtual nodes.
 */
std::pair<std::unordered_map<PROGRAM_LINE, std::unordered_set<NextBipEdge>>, std::unordered_set<std::unique_ptr<const TNode>>>
getNextBipRelationship(const std::unordered_map<int, std::unordered_set<int>>& nextRelationship,
                       const std::unordered_map<TNodeType, std::vector<const TNode*>, EnumClassHash>& tNodeTypeToTNode,
                       const std::unordered_map<const TNode*, int>& tNodeToStatementNumber);

std::unordered_map<PROGRAM_LINE, std::unordered_set<NextBipEdge>>
getPreviousBipRelationship(const std::unordered_map<PROGRAM_LINE, std::unordered_set<NextBipEdge>>& nextBipRelationship);

/**
 * Get mapping of the possible statements that goes to a statement.
 */
std::unordered_map<int, std::unordered_set<int>>
getPreviousRelationship(const std::unordered_map<int, std::unordered_set<int>>& nextRelationship);

std::unordered_map<VARIABLE_NAME, STATEMENT_NUMBER_SET>
getConditionVariablesToStatementNumbers(const std::unordered_map<int, const TNode*>& statementNumberToTNode);

/**
 * Get mapping of the possible assignment statements that Affect other assignment statements.
 */
std::unordered_map<STATEMENT_NUMBER, STATEMENT_NUMBER_SET>
getAffectsMapping(const std::unordered_map<TNodeType, std::vector<const TNode*>, EnumClassHash>& tNodeTypeToTNodes,
                  const std::unordered_map<const TNode*, STATEMENT_NUMBER>& tNodeToStatementNumber,
                  const std::unordered_map<STATEMENT_NUMBER, const TNode*>& statementNumberToTNode,
                  const std::unordered_map<int, std::unordered_set<int>> nextRelationship,
                  const std::unordered_map<int, std::unordered_set<int>> previousRelationship,
                  const std::unordered_map<const TNode*, std::unordered_set<std::string>> usesMapping,
                  std::unordered_map<const TNode*, std::unordered_set<std::string>> modifiesMapping);

std::unordered_map<STATEMENT_NUMBER, STATEMENT_NUMBER_SET>
getAffectedMapping(std::unordered_map<STATEMENT_NUMBER, STATEMENT_NUMBER_SET> affectsMapping);

} // namespace extractor
} // namespace backend

namespace std {
template <> struct ::std::hash<backend::extractor::NextBipEdge> {
    std::size_t operator()(backend::extractor::NextBipEdge const& s) const noexcept {
        return (s.nextLine ^ (s.prevLine << 1)) ^ (s.label << 1);
    }
};
} // namespace std
