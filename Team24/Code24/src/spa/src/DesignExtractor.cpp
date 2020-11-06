#include "DesignExtractor.h"

#include "Foost.hpp"
#include "Logger.h"
#include "PKB.h"
#include "TNode.h"

#include <algorithm>
#include <cassert>
#include <memory>
#include <set>
#include <stack>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>

namespace backend {
namespace extractor {

typedef int STATEMENT_NUMBER;

std::unordered_map<const TNode*, int> getTNodeToStatementNumber(const TNode& ast) {
    logLine("start getTNodeToStatementNumber");
    logWord("getTNodeToStatementNumber: ast is ");
    logLine(ast.toString());

    std::unordered_map<const TNode*, int> tNodeToStatementNumber;
    int currentStatementNumber = 1;

    // DFS through the AST and assign statement numbers.
    // Here, we implicitly assume that AST children are ordered by their
    // beginning line number in the source code. For example, we assume that the
    // "then" statement list is a child
    std::stack<const TNode*> nodeStack;
    nodeStack.push(&ast);
    while (!nodeStack.empty()) {
        const TNode* tNode = nodeStack.top();
        nodeStack.pop();

        logWord("Popped tNode from stack: ");
        logLine(tNode->toShortString());

        if (tNode->isStatementNode()) {

            logWord("Found Statement tNode");
            logWord(tNode->toShortString());
            logWord(", assigning");
            logLine(currentStatementNumber);
            logWord("  Does this tNode already exist in the Hashmap?");
            logLine(tNodeToStatementNumber.count(tNode));

            tNodeToStatementNumber[tNode] = currentStatementNumber;
            currentStatementNumber++;
        }


        logWord(" number of children: ");
        logLine(tNode->children.size());
        // Iterate in the reverse direction to ensure that the top of the stack is the
        // "first" statement in the SIMPLE program
        for (int childIndex = (int)tNode->children.size() - 1; childIndex >= 0; childIndex--) {
            logWord("  childindex ");
            logLine(childIndex);

            const TNode& child = tNode->children[childIndex];

            logWord("  Adding ");
            logLine(child.toShortString());

            nodeStack.push(&child);
        }
    }

    logLine("success getTNodeToStatementNumber");
    return tNodeToStatementNumber;
}

std::unordered_map<TNodeType, std::vector<const TNode*>, EnumClassHash> getTNodeTypeToTNodes(const TNode& ast) {
    std::unordered_map<TNodeType, std::vector<const TNode*>, EnumClassHash> typesToTNode;
    std::vector<const TNode*> toVisit;
    toVisit.push_back(&ast);
    while (toVisit.size() > 0) {
        const TNode* visiting = toVisit.back();
        TNodeType type = visiting->type;
        if (typesToTNode.find(type) == typesToTNode.end()) {
            typesToTNode[type] = {};
        }
        typesToTNode[type].push_back(visiting);

        toVisit.pop_back();
        for (const TNode& c : visiting->children) {
            toVisit.push_back(&c);
        }
    }
    return typesToTNode;
}

std::unordered_map<int, const TNode*>
getStatementNumberToTNode(const std::unordered_map<const TNode*, int>& tNodeToStatementNumber) {
    std::unordered_map<int, const TNode*> statementNumberToTNode;
    for (const auto& p : tNodeToStatementNumber) {
        statementNumberToTNode[p.second] = p.first;
    }
    return statementNumberToTNode;
}

const TNode* getProcedureFromProcedureName(
const std::string& procedureName,
const std::unordered_map<TNodeType, std::vector<const TNode*>, EnumClassHash>& tNodeTypeToTNodes) {
    for (auto tNode : tNodeTypeToTNodes.at(TNodeType::Procedure)) {
        if (tNode->name == procedureName) {
            return tNode;
        }
    }
    throw std::runtime_error("Error: Could not find procedure " + procedureName);
}

;
std::unordered_map<const TNode*, std::unordered_set<const TNode*>>
getProcedureToCallees(const std::unordered_map<TNodeType, std::vector<const TNode*>, EnumClassHash>& tNodeTypeToTNodes) {
    std::unordered_map<const TNode*, std::unordered_set<const TNode*>> procedureToCallees;
    logLine("getProcedureToCallees started");
    for (const auto& callingProcedure : tNodeTypeToTNodes.at(TNodeType::Procedure)) {
        logLine("getProcedureToCallees: Looking at procedure " + callingProcedure->toShortString());
        // Initialize an entry
        procedureToCallees[callingProcedure] = std::unordered_set<const TNode*>();

        // Get procedures called by this procedure
        std::vector<const TNode*> stack = { callingProcedure };
        while (!stack.empty()) {
            const TNode* tNode = stack.back();
            stack.pop_back();
            logLine("getProcedureToCallees: Looking at tNode " + tNode->toShortString());

            for (auto& child : tNode->children) {
                stack.push_back(&child);
            }

            if (tNode->type != TNodeType::Call) {
                continue;
            }

            logLine("getProcedureToCallees: Found call statement " + tNode->toShortString());
            auto procedureName = tNode->children[0].name;
            auto calledProcedure = getProcedureFromProcedureName(procedureName, tNodeTypeToTNodes);
            logLine("getProcedureToCallees: Adding callee" + calledProcedure->toShortString() +
                    " for " + callingProcedure->toShortString());
            procedureToCallees[callingProcedure].insert(calledProcedure);
        }
    }
    return procedureToCallees;
}

bool cyclicCallExistsHelper(const TNode* currentNode,
                            const std::unordered_map<const TNode*, std::unordered_set<const TNode*>>& procedureToCallees,
                            std::unordered_set<const TNode*>& currentlyVisiting) {
    if (currentlyVisiting.find(currentNode) != currentlyVisiting.end()) {
        logLine("cyclicCallExistsHelper: Cycle detected at " + currentNode->toShortString());
        return true;
    }

    logLine("cyclicCallExistsHelper: visiting " + currentNode->toShortString());
    currentlyVisiting.insert(currentNode);
    bool cyclicCallExists = false;
    if (procedureToCallees.find(currentNode) != procedureToCallees.end()) {
        for (const TNode* callee : procedureToCallees.find(currentNode)->second) {
            cyclicCallExists =
            cyclicCallExists || cyclicCallExistsHelper(callee, procedureToCallees, currentlyVisiting);
        }
    }
    currentlyVisiting.erase(currentNode);

    logWord("cyclicCallExistsHelper: visited " + currentNode->toShortString() + ", cyclic call exists? ");
    logLine(cyclicCallExists);

    return cyclicCallExists;
}

bool cyclicCallExists(const std::unordered_map<const TNode*, std::unordered_set<const TNode*>>& procedureToCallees) {
    for (auto& p : procedureToCallees) {
        const TNode* currentProcedure = p.first;
        std::unordered_set<const TNode*> visitedNodes;
        if (cyclicCallExistsHelper(currentProcedure, procedureToCallees, visitedNodes)) {
            return true;
        }
    }
    return false;
}


/**
 * A DFS Helper that is used to compute the Uses mapping.
 * @param currentNode
 * @param tNodeTypeToTNodes
 * @param usesMapping a reference to the mapping, that will be updated with the Uses information for
 * the currentNode.
 */
void getUsesMappingHelper(const TNode* currentNode,
                          const std::unordered_map<TNodeType, std::vector<const TNode*>, EnumClassHash>& tNodeTypeToTNodes,
                          std::unordered_map<const TNode*, std::unordered_set<std::string>>& usesMapping) {
    logLine("Starting usesHelper for node " + currentNode->toString());
    // The Uses information for this node was already computed.
    if (usesMapping.find(currentNode) != usesMapping.end()) {
        return;
    }

    // In the case of a read statement, we don't want to accumulate the variable name
    // since we modify it instead of using it.
    if (currentNode->type == TNodeType::Read) {
        return;
    }

    std::unordered_set<std::string> variablesUsedByCurrentTNode;

    if (currentNode->type == TNodeType::Variable) {
        // Base case. We want to propagate the usage of a variable upwards.
        variablesUsedByCurrentTNode.insert(currentNode->name);
        logLine("adding var name (" + currentNode->name + ")");
    } else if (currentNode->type == TNodeType::Assign) {
        // Only visit the RHS. Ignore the LHS of the assignment.
        for (auto it = currentNode->children.begin() + 1; it != currentNode->children.end(); it++) {
            // get a pointer to the child
            const TNode* child = &(*it);
            // Index all the variables used by the child
            getUsesMappingHelper(child, tNodeTypeToTNodes, usesMapping);
            // If the child has used any variables, the currentNode has also used the variables
            // used by the child.
            if (usesMapping.find(child) != usesMapping.end()) {
                variablesUsedByCurrentTNode.insert(usesMapping[child].begin(), usesMapping[child].end());
            }
        }
    } else if (currentNode->type == TNodeType::Call) {
        // Retrieve the procedure name
        const TNode* procNameNode = &(currentNode->children[0]);
        auto calledProc = getProcedureFromProcedureName(procNameNode->name, tNodeTypeToTNodes);

        // Compute the Uses mapping of the called procedure.
        getUsesMappingHelper(calledProc, tNodeTypeToTNodes, usesMapping);

        // Index the variables used in the called procedure, if any, becuase
        // this call statement uses all the variables used by the called procedure
        if (usesMapping.find(calledProc) != usesMapping.end()) {
            variablesUsedByCurrentTNode.insert(usesMapping[calledProc].begin(),
                                               usesMapping[calledProc].end());
        } else {
            logLine("Procedure " + calledProc->toShortString() + " uses nothing");
        }
    } else {
        // Accumulate the variables used by currentNode's children (if any), and add them to the variables used by the currentNode
        for (auto& child : currentNode->children) {
            getUsesMappingHelper(&child, tNodeTypeToTNodes, usesMapping);
            if (usesMapping.find(&child) != usesMapping.end()) {
                variablesUsedByCurrentTNode.insert(usesMapping[&child].begin(), usesMapping[&child].end());
            }
        }
    }

    if (usesMapping.find(currentNode) != usesMapping.end()) {
        throw std::runtime_error(
        "Current node " + currentNode->toShortString() +
        " has already been indexed, but every TNode should only be indexed once.");
    }
    usesMapping[currentNode] = std::unordered_set<std::string>(variablesUsedByCurrentTNode.begin(),
                                                               variablesUsedByCurrentTNode.end());
}

std::unordered_map<const TNode*, std::unordered_set<std::string>>
getUsesMapping(std::unordered_map<TNodeType, std::vector<const TNode*>, EnumClassHash>& tNodeTypeToTNodes) {
    std::unordered_map<const TNode*, std::unordered_set<std::string>> usesMapping;
    for (const TNode* procedure : tNodeTypeToTNodes[Procedure]) {
        logLine("Starting uses for proc " + procedure->toShortString());
        getUsesMappingHelper(procedure, tNodeTypeToTNodes, usesMapping);
    }

    // Filter out TNodeTypes that should not have the Uses relation.
    // We do this because we want certain TNodes (e.g. StatementList) to retain Uses
    // information from their descendants, so that their ancestors can copy that information
    // too.
    std::vector<const TNode*> keysToDelete;

    std::set<TNodeType> validTypesForUses = { Assign, Print, IfElse, While, Call, Procedure };
    for (auto& p : usesMapping) {
        // Some TNodes are added simply to propagate the Uses mappings upwwards, e.g.
        // most expression nodes.
        if (validTypesForUses.find(p.first->type) == validTypesForUses.end()) {
            keysToDelete.push_back(p.first);
            // Visited TNodes with empty uses information should not be returned
        } else if (p.second.empty()) {
            keysToDelete.push_back(p.first);
        }
    }

    for (auto& keyToDelete : keysToDelete) {
        usesMapping.erase(keyToDelete);
    }

    return usesMapping;
}

/**
 * A DFS Helper that is used to compute the Modifies mapping.
 * Prerequisite: any procedure(s) that currentNode calls must have been indexed, i.e. it must exist
 * in the modifiesMapping.
 * @param currentNode
 * @param tNodeTypeToTNodes
 * @param modifiesMapping a reference to the mapping, that will be updated with the Modifies
 * information for the currentNode.
 */
void getModifiesMappingHelper(const TNode* currentNode,
                              const std::unordered_map<TNodeType, std::vector<const TNode*>, EnumClassHash>& tNodeTypeToTNodes,
                              std::unordered_map<const TNode*, std::unordered_set<std::string>>& modifiesMapping) {
    if (modifiesMapping.find(currentNode) != modifiesMapping.end()) {
        return;
    }

    std::unordered_set<std::string> variablesModifiedByCurrentTNode;

    if (currentNode->type == TNodeType::Assign) {
        std::string variableAssignedTo = currentNode->children.at(0).name;
        variablesModifiedByCurrentTNode.insert(variableAssignedTo);
        logLine("adding Assigned var name (" + variableAssignedTo + ")");
    } else if (currentNode->type == TNodeType::Read) {
        std::string variableReadTo = currentNode->children.at(0).name;
        variablesModifiedByCurrentTNode.insert(variableReadTo);
        logLine("adding Read var name (" + variableReadTo + ")");
    } else if (currentNode->type == TNodeType::Call) {
        // Retrieve the procedure name
        const TNode* procNameNode = &(currentNode->children[0]);
        auto calledProc = getProcedureFromProcedureName(procNameNode->name, tNodeTypeToTNodes);

        // Compute the Modifies mapping of the called procedure.
        getModifiesMappingHelper(calledProc, tNodeTypeToTNodes, modifiesMapping);

        // Index the variables modified in this procedure, if any, because
        // this call statement modifies all the variables modified by the called procedure.
        if (modifiesMapping.find(calledProc) != modifiesMapping.end()) {
            variablesModifiedByCurrentTNode.insert(modifiesMapping[calledProc].begin(),
                                                   modifiesMapping[calledProc].end());
        } else {
            logLine("Procedure " + calledProc->toShortString() + " modifies nothing");
        }
    } else {
        // Accumulate the variables modified by currentNode's children (if any), and add them to the variables used by the currentNode
        for (auto& child : currentNode->children) {
            getModifiesMappingHelper(&child, tNodeTypeToTNodes, modifiesMapping);
            if (modifiesMapping.find(&child) != modifiesMapping.end()) {
                variablesModifiedByCurrentTNode.insert(modifiesMapping[&child].begin(),
                                                       modifiesMapping[&child].end());
            }
        }
    }

    if (modifiesMapping.find(currentNode) != modifiesMapping.end()) {
        throw std::runtime_error(
        "Current node " + currentNode->toShortString() +
        " has already been indexed, but every TNode should only be indexed once.");
    }

    modifiesMapping[currentNode] = variablesModifiedByCurrentTNode;
}

std::unordered_map<const TNode*, std::unordered_set<std::string>>
getModifiesMapping(std::unordered_map<TNodeType, std::vector<const TNode*>, EnumClassHash>& tNodeTypeToTNodes) {
    std::unordered_map<const TNode*, std::unordered_set<std::string>> modifiesMapping;
    for (const auto procedure : tNodeTypeToTNodes[Procedure]) {
        logLine("Starting modifies for proc " + procedure->toShortString());
        getModifiesMappingHelper(procedure, tNodeTypeToTNodes, modifiesMapping);
    }

    // Filter out TNodeTypes that should not have the Modifies relation.
    // We do this because we want certain TNodes (e.g. StatementList) to retain Modifies
    // information from their descendants, so that their ancestors can copy that information
    // too.
    std::vector<const TNode*> keysToDelete;
    std::set<TNodeType> validTypesForModifies = { Assign, Read, IfElse, While, Call, Procedure };
    for (auto& p : modifiesMapping) {
        if (validTypesForModifies.find(p.first->type) == validTypesForModifies.end() || p.second.empty()) {
            keysToDelete.push_back(p.first);
        }
    }

    for (auto& keyToDelete : keysToDelete) {
        modifiesMapping.erase(keyToDelete);
    }

    return modifiesMapping;
}

std::pair<std::unordered_map<int, int>, std::unordered_map<int, int>> getFollowRelationship(const TNode& ast) {
    auto tNodeTypeToTNode = getTNodeTypeToTNodes(ast);
    auto tNodeToStmtNo = getTNodeToStatementNumber(ast);
    std::unordered_map<int, int> followerFollowedMap;
    std::unordered_map<int, int> followedFollowerMap;

    for (const TNode* stmtList : tNodeTypeToTNode[StatementList]) {
        const std::vector<TNode>& children = stmtList->children;
        for (unsigned int i = 1; i < children.size(); ++i) {
            const TNode* curr = &(children[i]);
            const TNode* prev = &(children[i - 1]);
            int currStmtNo = tNodeToStmtNo[curr];
            int prevStmtNo = tNodeToStmtNo[prev];
            followedFollowerMap[prevStmtNo] = currStmtNo;
            followerFollowedMap[currStmtNo] = prevStmtNo;
        }
    }
    return { followerFollowedMap, followedFollowerMap };
}


std::pair<std::unordered_map<int, int>, std::unordered_map<int, STATEMENT_NUMBER_SET>>
getParentRelationship(const TNode& ast) {
    auto tNodetypeToTNode = getTNodeTypeToTNodes(ast);
    auto tNodeToStmtNo = getTNodeToStatementNumber(ast);

    std::unordered_map<int, int> childParentMap;
    std::unordered_map<int, STATEMENT_NUMBER_SET> parentChildrenMap;


    for (const TNode* parent : tNodetypeToTNode[While]) {
        int parentStmtNo = tNodeToStmtNo[parent]; // we do not expect this to throw as parent has stmt_no.
        if (parentChildrenMap.find(parentStmtNo) == parentChildrenMap.end()) {
            parentChildrenMap[parentStmtNo] = {};
        }

        // WHILE-tNode must have 2 children; cond and stmt-list
        const TNode& stmtLists = parent->children[1];
        for (const TNode& stmt : stmtLists.children) {
            int childStmtNo = tNodeToStmtNo[&stmt]; // we do not expect this to throw as child has stmt_no.
            childParentMap[childStmtNo] = parentStmtNo;
            parentChildrenMap[parentStmtNo].insert(childStmtNo);
        }
    }

    for (const TNode* parent : tNodetypeToTNode[IfElse]) {
        int parentStmtNo = tNodeToStmtNo[parent]; // we do not expect this to throw as parent has stmt_no.
        // If-else-tNode must have 2 children; cond, if-stmt-list and else-stmt-list
        if (parentChildrenMap.find(parentStmtNo) == parentChildrenMap.end()) {
            parentChildrenMap[parentStmtNo] = {};
        }

        // if-stmts
        const TNode& ifStmtLists = parent->children[1];
        for (const TNode& stmt : ifStmtLists.children) {
            int childStmtNo = tNodeToStmtNo[&stmt]; // we do not expect this to throw as child has stmt_no.
            childParentMap[childStmtNo] = parentStmtNo;
            parentChildrenMap[parentStmtNo].insert(childStmtNo);
        }

        // else-stmts
        const TNode& elseStmtLists = parent->children[2];
        for (const TNode& stmt : elseStmtLists.children) {
            int childStmtNo = tNodeToStmtNo[&stmt]; // we do not expect this to throw as child has stmt_no.
            childParentMap[childStmtNo] = parentStmtNo;
            parentChildrenMap[parentStmtNo].insert(childStmtNo);
        }
    }
    return { childParentMap, parentChildrenMap };
}

STATEMENT_NUMBER_SET getVisitedPathFromStart(int start, const std::unordered_map<int, int>& relation) {
    STATEMENT_NUMBER_SET result;
    auto it = relation.find(start);
    while (it != relation.end()) {
        int next = it->second;
        result.insert(next);
        it = relation.find(next);
    }
    return result;
}

/**
 * Helper method for getPatternsMap when running a post-order traversal on the expr of an assignment statement.
 * @param result - the hashmap storing the result
 * @param visiting - The node that we are currently visiting
 * @param assignee - For each assignment statement in the AST, there is an assignee, which is
 * the left of the assignment statement. i.e. the "a" in a = 1 + 1;
 * @param stmtNo - The statement number of the assignee and expression.
 * @param isSubExpr - boolean of whether the visiting tNode is a sub expression
 * @return the string representation of visitingNode.
 */
std::string dfsHelper(std::unordered_map<std::string, std::vector<std::tuple<std::string, int, bool>>>& result,
                      const TNode& visiting,
                      const std::string& assignee,
                      int stmtNo,
                      bool isSubExpr) {
    if (visiting.children.size() == 0) {
        // By validity of AST, if `visiting` is leaf node, it must be a const or a var.
        if (visiting.type == TNodeType::Constant) {
            result[visiting.constant].emplace_back(assignee, stmtNo, isSubExpr);
            return visiting.constant;
        } else if (visiting.type == TNodeType::Variable) {
            result[visiting.name].emplace_back(assignee, stmtNo, isSubExpr);
            return visiting.name;
        } else {
            throw std::runtime_error(
            "dfsHelper: leaf node should only be a constant or a variable. Got: " +
            getTNodeTypeString(visiting.type));
        }
    }
    // there should be 2 children for any operator.
    const TNode& lhs = visiting.children.at(0);
    const TNode& rhs = visiting.children.at(1);

    // the lhs string must have a right subtree (rhsString), and vice-versa
    std::string lhsString = dfsHelper(result, lhs, assignee, stmtNo, true);
    std::string rhsString = dfsHelper(result, rhs, assignee, stmtNo, true);
    std::string opString = getOperatorStringFromTNodeType(visiting.type);

    std::string visitingString = "(" + lhsString + opString + rhsString + ")";
    result[visitingString].emplace_back(assignee, stmtNo, isSubExpr);
    return visitingString;
}

/**
 * Helper method for getPatternsMap when running a post-order traversal on the ast.
 * @param result - the hashmap storing the result
 * @param visiting - The node that we are currently visiting
 * @param assignee - For each assignment statement in the AST, there is an assignee, which is
 * the left of the assignment statement. i.e. the "a" in a = 1 + 1;
 * @param stmtNo - The statement number of the assignee and expression.
 * @param hasLeft - boolean of whether this visiting node has a left expr. e.g. for 1 + (visiting
 * node) + 2, visitingNode has both left(1) and right(2) expr.
 * @param hasRight - symmetric to hasLeft
 * @return map<string, {assignee, stmt_number, has_left_node, has_right_node}>
 * eg {1+2*4/5: {a, 4, false, true}};
 */
std::unordered_map<std::string, std::vector<std::tuple<std::string, int, bool>>>
getPatternsMap(const std::vector<const TNode*>& assignTNodes,
               const std::unordered_map<const TNode*, int>& tNodeToStatementNumber) {
    std::unordered_map<std::string, std::vector<std::tuple<std::string, int, bool>>> result;
    for (const TNode* tNodePtr : assignTNodes) {
        const TNode& assigneeNode = tNodePtr->children.at(0);
        const std::string& assigneeName = assigneeNode.name;
        int stmtNo = tNodeToStatementNumber.at(tNodePtr);

        const TNode& exprTNode = tNodePtr->children.at(1);
        dfsHelper(result, exprTNode, assigneeName, stmtNo, false);
    }
    return result;
}

std::unordered_map<int, TNodeType>
getStatementNumberToTNodeTypeMap(const std::unordered_map<int, const TNode*>& statementNumberToTNode) {
    std::unordered_map<int, TNodeType> result;
    for (auto it : statementNumberToTNode) {
        int stmtNo = it.first;
        const TNode* tNode = it.second;
        result[stmtNo] = tNode->type;
    }
    return result;
}

bool isValidSimpleProgram(const TNode& ast) {
    auto tNodeTypeToTNodes = getTNodeTypeToTNodes(ast);
    // Two procedures with the same name is considered an error.
    std::unordered_set<std::string> procedureNames;
    for (const TNode* procedure : tNodeTypeToTNodes[Procedure]) {
        if (procedureNames.find(procedure->name) != procedureNames.end()) {
            return false;
        }
        procedureNames.insert(procedure->name);
    }

    // Call to a non-existing procedure produces an error
    for (const TNode* callStatement : tNodeTypeToTNodes[Call]) {
        std::string calledProcedureName = callStatement->children[0].name;
        if (procedureNames.find(calledProcedureName) == procedureNames.end()) {
            return false;
        }
    }

    // Cyclic calls are not allowed.
    // For example, procedure A calls procedure B, procedure B calls C, and C calls A
    // should not be accepted in a correct SIMPLE code.
    auto procedureToCallees = getProcedureToCallees(tNodeTypeToTNodes);
    if (cyclicCallExists(procedureToCallees)) {
        return false;
    }

    // The SIMPLE program didn't fail any of the checks, so it is valid.
    return true;
}

/**
 * @returns the last lines that will be executed in a procedure, according to the provided
 * nextRelationship.
 */
std::unordered_set<PROGRAM_LINE>
getTerminatingLinesOfProcedure(const TNode* procedure,
                               const std::unordered_map<int, std::unordered_set<int>>& nextRelationship,
                               const std::unordered_map<const TNode*, int>& tNodeToStatementNumber) {
    const TNode* stmtList = &procedure->children[0];
    const TNode* firstStmt = &stmtList->children[0];
    const PROGRAM_LINE firstLine = tNodeToStatementNumber.at(firstStmt);

    std::unordered_set<PROGRAM_LINE> allTNodesInProcedure =
    foost::getVisitedInDFS(firstLine, nextRelationship, true);
    allTNodesInProcedure.insert(firstLine);

    std::unordered_set<PROGRAM_LINE> terminatingNodesOfProcedure;

    for (auto& p : tNodeToStatementNumber) {
        const TNode* currNode = p.first;
        PROGRAM_LINE currLine = p.second;
        if (allTNodesInProcedure.find(currLine) == allTNodesInProcedure.end()) {
            continue;
        }

        if (nextRelationship.find(currLine) == nextRelationship.end()) {
            terminatingNodesOfProcedure.insert(currLine);
            continue;
        }
        const std::unordered_set<PROGRAM_LINE>& nextLines = nextRelationship.at(currLine);

        if (nextLines.empty() || (currNode->type == While && nextLines.size() == 1)) {
            terminatingNodesOfProcedure.insert(currLine);
        }
    }

    return terminatingNodesOfProcedure;
}

// @returns the first program line executed in a procedure.
PROGRAM_LINE getFirstStatementOfProcedure(const TNode* procedure,
                                          const std::unordered_map<const TNode*, int>& tNodeToStatementNumber) {
    const TNode* stmtList = &procedure->children[0];
    const TNode* firstStmt = &stmtList->children[0];
    return tNodeToStatementNumber.at(firstStmt);
}

std::pair<std::unordered_map<PROGRAM_LINE, std::unordered_set<NextBipEdge>>, std::unordered_set<std::unique_ptr<const TNode>>>
getNextBipRelationship(const std::unordered_map<int, std::unordered_set<int>>& nextRelationship,
                       const std::unordered_map<TNodeType, std::vector<const TNode*>, EnumClassHash>& tNodeTypeToTNode,
                       const std::unordered_map<const TNode*, int>& tNodeToStatementNumberOriginal) {
    std::unordered_map<const TNode*, int> tNodeToStatementNumber = tNodeToStatementNumberOriginal;


    std::unordered_map<PROGRAM_LINE, std::unordered_set<NextBipEdge>> nextBipRelationship;

    // Register every statement in NextBip
    for (auto& p : tNodeToStatementNumber) {
        nextBipRelationship[p.second] = std::unordered_set<NextBipEdge>();
    }

    // Copy over edges from the "Next" relationship, with empty labels
    for (auto& p : nextRelationship) {
        std::unordered_set<NextBipEdge> edges;
        for (auto& s : p.second) {
            edges.insert(NextBipEdge(p.first, s));
        }
        nextBipRelationship[p.first] = edges;
    }

    // Keep the end-nodes of each procedure ("dummy" nodes)
    std::unordered_map<const TNode*, PROGRAM_LINE> procedureToEndNode;
    std::unordered_set<std::unique_ptr<const TNode>> createdEndNodes;
    for (int i = 0; i < tNodeTypeToTNode.at(Procedure).size(); ++i) {
        const TNode* procedureNode = tNodeTypeToTNode.at(Procedure).at(i);

        const TNode* endNode = new TNode(DUMMY);
        createdEndNodes.insert(std::unique_ptr<const TNode>(endNode));

        PROGRAM_LINE endNodeStatementNumber = -(i + 1);
        tNodeToStatementNumber[endNode] = endNodeStatementNumber;
        nextBipRelationship[endNodeStatementNumber] = {};
        procedureToEndNode[procedureNode] = endNodeStatementNumber;
    }

    // Link up the terminating nodes in a procedure to the end-nodes
    // (lines that may not have a line executing after them) with the dummy end-node
    for (const TNode* procedureNode : tNodeTypeToTNode.at(Procedure)) {
        PROGRAM_LINE endNodeStatementNumber = procedureToEndNode[procedureNode];
        const std::unordered_set<PROGRAM_LINE> terminatingLinesOfProcedure =
        getTerminatingLinesOfProcedure(procedureNode, nextRelationship, tNodeToStatementNumberOriginal);

        for (PROGRAM_LINE terminatingLine : terminatingLinesOfProcedure) {
            nextBipRelationship[terminatingLine].insert(NextBipEdge(terminatingLine, endNodeStatementNumber));
        }
    }

    // Return if there is no extra work to be done wrt call statements.
    if (tNodeTypeToTNode.find(Call) == tNodeTypeToTNode.end()) {
        return { nextBipRelationship, std::move(createdEndNodes) };
    }

    // For every call statement, modify its outgoing edges and add edges from the end-nodes
    // of the called procedures.
    for (const TNode* callStatement : tNodeTypeToTNode.at(Call)) {
        PROGRAM_LINE callProgramLine = tNodeToStatementNumber.at(callStatement);
        std::unordered_set<NextBipEdge>& callStatementNextBipEdges = nextBipRelationship.at(callProgramLine);

        const TNode* calledProcedure =
        getProcedureFromProcedureName(callStatement->children[0].name, tNodeTypeToTNode);
        const PROGRAM_LINE firstStatementOfProcedure =
        getFirstStatementOfProcedure(calledProcedure, tNodeToStatementNumber);

        assert(callStatementNextBipEdges.size() == 1);

        // Get the next statement that should be executed after the procedure call.
        PROGRAM_LINE nextProgramLine = callStatementNextBipEdges.begin()->nextLine;

        // Add an outgoing edge to the called proc, with the current line number as the label
        callStatementNextBipEdges.insert(NextBipEdge(callProgramLine, firstStatementOfProcedure, callProgramLine));

        // Add a BranchBack edge from the end-node of the called procedure to the next line that
        // should be executed.
        PROGRAM_LINE calledProcedureEndNode = procedureToEndNode[calledProcedure];
        nextBipRelationship[calledProcedureEndNode].insert(
        NextBipEdge(calledProcedureEndNode, nextProgramLine, callProgramLine));

        // Remove the original edge from the call statement line -> next line.
        // This edge was added by Next, but it should not be preserved by NextBip
        callStatementNextBipEdges.erase(NextBipEdge(callProgramLine, nextProgramLine));
    }

    return { nextBipRelationship, std::move(createdEndNodes) };
}

std::unordered_map<PROGRAM_LINE, std::unordered_set<NextBipEdge>>
getPreviousBipRelationship(const std::unordered_map<PROGRAM_LINE, std::unordered_set<NextBipEdge>>& nextBipRelationship) {
    std::unordered_map<PROGRAM_LINE, std::unordered_set<NextBipEdge>> previousBipRelationship;
    for (const auto& p : nextBipRelationship) {
        previousBipRelationship[p.first] = {};
    }
    for (const auto& p : nextBipRelationship) {
        for (const NextBipEdge& edge : p.second) {
            PROGRAM_LINE from = edge.prevLine;
            PROGRAM_LINE to = edge.nextLine;
            previousBipRelationship[to].insert(edge.reversed());
        }
    }
    return previousBipRelationship;
}

/**
 * helper method for getNextRelationship
 * @return first and last statement number of a statement list (statement block)
 */
std::pair<STATEMENT_NUMBER, STATEMENT_NUMBER>
getStartAndEndStmtNumOfStmtList(const TNode* stmtList,
                                const std::unordered_map<const TNode*, int>& tNodeToStatementNumber) {
    return { tNodeToStatementNumber.at(&stmtList->children.at(0)),
             tNodeToStatementNumber.at(&stmtList->children.back()) };
}

std::unordered_map<int, std::unordered_set<int>>
getNextRelationship(const std::unordered_map<TNodeType, std::vector<const TNode*>, EnumClassHash>& tNodeTypeToTNode,
                    const std::unordered_map<const TNode*, STATEMENT_NUMBER>& tNodeToStatementNumber) {
    std::unordered_map<STATEMENT_NUMBER, std::unordered_set<STATEMENT_NUMBER>> result;

    // Create next relationship for statement lists
    for (const TNode* stmtListNode : tNodeTypeToTNode.at(StatementList)) {
        STATEMENT_NUMBER prevNodeStmtNum = tNodeToStatementNumber.at(&stmtListNode->children[0]);
        for (size_t i = 1; i < stmtListNode->children.size(); ++i) {
            const TNode* currNode = &stmtListNode->children[i];
            STATEMENT_NUMBER currNodeStmtNum = tNodeToStatementNumber.at(currNode);
            result[prevNodeStmtNum].insert(currNodeStmtNum);
            prevNodeStmtNum = currNodeStmtNum;
        }
    }

    // Create next relationship for while statements
    // Evaluation of next relationship for while statements must come before if-else to point the
    // last statement (which could be if-else) of a while-block back to the while-loop.
    if (tNodeTypeToTNode.find(While) != tNodeTypeToTNode.end()) {
        for (const TNode* whileNode : tNodeTypeToTNode.at(While)) {
            STATEMENT_NUMBER startStmtNo, endStmtNo;
            std::tie(startStmtNo, endStmtNo) =
            getStartAndEndStmtNumOfStmtList(&whileNode->children.at(1), tNodeToStatementNumber);
            STATEMENT_NUMBER whileNodeStmtNo = tNodeToStatementNumber.at(whileNode);
            result[whileNodeStmtNo].insert(startStmtNo);
            // the last statement of a while-loop jumps back to the while-cond
            assert((result.find(endStmtNo) == result.end()));
            result[endStmtNo].insert(whileNodeStmtNo);
        }
    }

    // Currently, the 'next' of the if-else block is the next statement that its children should go
    // to. This block of code sort the if-else statement by statement number. And for it's if-else
    // block, we set the last statement's 'next' to be the if-else's 'next'.
    if (tNodeTypeToTNode.find(IfElse) != tNodeTypeToTNode.end()) {
        std::vector<std::pair<STATEMENT_NUMBER, const TNode*>> ifElseStmts;
        for (const TNode* ifElseNode : tNodeTypeToTNode.at(IfElse)) {
            ifElseStmts.emplace_back(tNodeToStatementNumber.at(ifElseNode), ifElseNode);
        }
        std::sort(ifElseStmts.begin(), ifElseStmts.end());

        for (const std::pair<STATEMENT_NUMBER, const TNode*>& stmtNodePair : ifElseStmts) {
            const TNode* ifElseNode = stmtNodePair.second;
            STATEMENT_NUMBER ifNodeStmtNo = stmtNodePair.first;
            STATEMENT_NUMBER nextStatement = -1;
            if (result.find(ifNodeStmtNo) != result.end()) {
                // there's a next statement for the if-else block!
                // Since ifNodeStmtNo must only have 1 next statement, we simply retrieve it.
                assert((!result.at(ifNodeStmtNo).empty()));
                nextStatement = *(result.at(ifNodeStmtNo).begin());
            }

            STATEMENT_NUMBER startIfStmtNo, endIfStmtNo;
            std::tie(startIfStmtNo, endIfStmtNo) =
            getStartAndEndStmtNumOfStmtList(&ifElseNode->children.at(1), tNodeToStatementNumber);

            STATEMENT_NUMBER startElseStmtNo, endElseStmtNo;
            std::tie(startElseStmtNo, endElseStmtNo) =
            getStartAndEndStmtNumOfStmtList(&ifElseNode->children.at(2), tNodeToStatementNumber);

            if (nextStatement != -1) {
                // There is a next statement for the if-block and else-block to go to.
                result[endIfStmtNo].insert(nextStatement);
                result[endElseStmtNo].insert(nextStatement);
            }

            // reset result[ifNodeStmtNo] `next` to be the start of the if and else block.
            result[ifNodeStmtNo] = { startIfStmtNo, startElseStmtNo };
        }
    }

    return result;
}

std::unordered_map<int, std::unordered_set<int>>
getPreviousRelationship(const std::unordered_map<int, std::unordered_set<int>>& nextRelationship) {
    std::unordered_map<int, std::unordered_set<int>> result;
    for (const auto& nextPair : nextRelationship) {
        for (int goTo : nextPair.second) {
            result[goTo].insert(nextPair.first);
        }
    }
    return result;
}

std::vector<VARIABLE_NAME> getAllVariablesInTNode(const TNode* node) {
    std::vector<const TNode*> toVisit = { node };
    std::vector<VARIABLE_NAME> result;
    while (!toVisit.empty()) {
        const TNode* visiting = toVisit.back();
        toVisit.pop_back();
        if (visiting->type == Variable) {
            result.push_back(visiting->name);
        } else {
            for (const TNode& child : visiting->children) {
                toVisit.push_back(&child);
            }
        }
    }
    return result;
}

std::unordered_map<VARIABLE_NAME, STATEMENT_NUMBER_SET>
getConditionVariablesToStatementNumbers(const std::unordered_map<int, const TNode*>& statementNumberToTNode) {
    std::unordered_map<VARIABLE_NAME, STATEMENT_NUMBER_SET> result;
    for (const std::pair<int, const TNode*>& pair : statementNumberToTNode) {
        int statementNumber = pair.first;
        const TNode* tNode = pair.second;
        if (tNode->type == While || tNode->type == IfElse) {
            // get all variables in the condition
            for (const VARIABLE_NAME& var : getAllVariablesInTNode(&tNode->children.front())) {
                result[var].insert(statementNumber);
            }
        }
    }
    return result;
}


std::unordered_map<STATEMENT_NUMBER, STATEMENT_NUMBER_SET>
getAffectsMapping(const std::unordered_map<TNodeType, std::vector<const TNode*>, EnumClassHash>& tNodeTypeToTNodes,
                  const std::unordered_map<const TNode*, STATEMENT_NUMBER>& tNodeToStatementNumber,
                  const std::unordered_map<STATEMENT_NUMBER, const TNode*>& statementNumberToTNode,
                  const std::unordered_map<int, std::unordered_set<int>>& nextRelationship,
                  const std::unordered_map<int, std::unordered_set<int>>& previousRelationship,
                  const std::unordered_map<const TNode*, std::unordered_set<std::string>>& usesMapping,
                  // TODO(remo5000) use const here
                  std::unordered_map<const TNode*, std::unordered_set<std::string>> modifiesMapping) {
    // This is an implementation of a worklist algorithm for reaching definition analysis.

    // Represents the assignment that cause a certain variable to be modified.
    typedef std::unordered_map<VARIABLE_NAME, STATEMENT_NUMBER_SET> VariableToAssigners;

    std::unordered_map<STATEMENT_NUMBER, VariableToAssigners> variablesReachingIn;
    for (auto& p : statementNumberToTNode) {
        variablesReachingIn.insert({ p.first, VariableToAssigners() });
    }
    std::unordered_map<STATEMENT_NUMBER, VariableToAssigners> variablesGoingOut;
    for (auto& p : statementNumberToTNode) {
        variablesGoingOut.insert({ p.first, VariableToAssigners() });
    }


    // Initialize
    std::unordered_set<const TNode*> startingTNodes;
    startingTNodes.insert(tNodeTypeToTNodes.at(Assign).begin(), tNodeTypeToTNodes.at(Assign).end());
    for (const TNode* tNode : startingTNodes) {
        STATEMENT_NUMBER statementNumber = tNodeToStatementNumber.at(tNode);
        VariableToAssigners initialVariableToAssigners;
        for (const VARIABLE_NAME& variable : modifiesMapping[tNode]) {
            initialVariableToAssigners[variable] = STATEMENT_NUMBER_SET();
            initialVariableToAssigners[variable].insert(statementNumber);
        }
        variablesGoingOut[statementNumber] = initialVariableToAssigners;
    }


    std::vector<STATEMENT_NUMBER> changedStatements;
    for (auto& p : statementNumberToTNode) {
        STATEMENT_NUMBER statementNumber = p.first;
        changedStatements.push_back(statementNumber);
    }


    while (!changedStatements.empty()) {
        STATEMENT_NUMBER statementNumber = changedStatements.back();
        changedStatements.pop_back();

        if (previousRelationship.find(statementNumber) == previousRelationship.end()) {
            continue;
        }

        variablesReachingIn[statementNumber] = VariableToAssigners();
        VariableToAssigners& statementAffects = variablesReachingIn.at(statementNumber);

        for (const STATEMENT_NUMBER& previousStatement : previousRelationship.at(statementNumber)) {
            for (auto& p1 : variablesGoingOut.at(previousStatement)) {
                const VARIABLE_NAME& variable = p1.first;
                const STATEMENT_NUMBER_SET& newAffectors = p1.second;

                if (statementAffects.find(variable) == statementAffects.end()) {
                    statementAffects[variable] = STATEMENT_NUMBER_SET();
                }

                statementAffects[variable].insert(newAffectors.begin(), newAffectors.end());
            }
        }

        // Make a copy of the old affects
        const VariableToAssigners oldOutwardAffects = variablesGoingOut[statementNumber];

        // OUT := IN
        variablesGoingOut[statementNumber] = variablesReachingIn[statementNumber];

        const TNode* tNode = statementNumberToTNode.at(statementNumber);

        // - KILL modified variables
        if (tNode->type == Assign || tNode->type == Read || tNode->type == Call) {
            const VARIABLE_NAME_SET& modifiedVariables = modifiesMapping[tNode];
            for (const VARIABLE_NAME& modifiedVariable : modifiedVariables) {
                variablesGoingOut[statementNumber].erase(modifiedVariable);
            }
        }

        // + GEN new variables
        if (tNode->type == Assign) {
            assert(modifiesMapping[tNode].size() == 1);
            VARIABLE_NAME variableModified = *modifiesMapping[tNode].begin();
            variablesGoingOut[statementNumber][variableModified] = { statementNumber };
        }

        if (variablesGoingOut[statementNumber] != oldOutwardAffects) {
            if (nextRelationship.find(statementNumber) == nextRelationship.end()) {
                continue;
            }
            for (const STATEMENT_NUMBER& nextStatement : nextRelationship.at(statementNumber)) {
                changedStatements.push_back(nextStatement);
            }
        }
    }

    std::unordered_map<STATEMENT_NUMBER, STATEMENT_NUMBER_SET> affectsMapping;
    for (auto& p : statementNumberToTNode) {
        STATEMENT_NUMBER statementNumber = p.first;
        const TNode* tNode = p.second;

        // Only assignments affect each other
        if (tNode->type != Assign) {
            continue;
        }

        for (auto& p1 : variablesReachingIn[statementNumber]) {
            VARIABLE_NAME variable = p1.first;

            // If this tNode does not use the variable that reaching this, skip.
            if (usesMapping.find(tNode) == usesMapping.end()) {
                continue;
            }
            auto& tNodeUses = usesMapping.at(tNode);
            if (tNodeUses.find(variable) == tNodeUses.end()) {
                continue;
            }

            STATEMENT_NUMBER_SET& affectors = p1.second;
            for (const STATEMENT_NUMBER& affector : affectors) {
                const TNode* affectorTNode = statementNumberToTNode.at(affector);
                // Only assignments affect each other
                if (affectorTNode->type != Assign) {
                    continue;
                }

                if (affectsMapping.find(affector) == affectsMapping.end())
                    affectsMapping[affector] = STATEMENT_NUMBER_SET();
                affectsMapping[affector].insert(statementNumber);
            }
        }
    }

    return affectsMapping;
}

std::unordered_map<STATEMENT_NUMBER, STATEMENT_NUMBER_SET>
getAffectedMapping(const std::unordered_map<STATEMENT_NUMBER, STATEMENT_NUMBER_SET>& affectsMapping) {
    std::unordered_map<STATEMENT_NUMBER, STATEMENT_NUMBER_SET> result;
    for (const auto& affectsPair : affectsMapping) {
        for (int affected : affectsPair.second) {
            result[affected].insert(affectsPair.first);
        }
    }
    return result;
}

std::pair<std::unordered_map<STATEMENT_NUMBER, STATEMENT_NUMBER_SET>, std::map<ScopedStatement, ScopedStatements>>
getAffectsBipMapping(const std::unordered_map<TNodeType, std::vector<const TNode*>, EnumClassHash>& tNodeTypeToTNodes,
                     const std::unordered_map<const TNode*, STATEMENT_NUMBER>& tNodeToStatementNumber,
                     const std::unordered_map<STATEMENT_NUMBER, const TNode*>& statementNumberToTNode,
                     const std::unordered_map<PROGRAM_LINE, std::unordered_set<NextBipEdge>>& nextBipRelationship,
                     const std::unordered_map<PROGRAM_LINE, std::unordered_set<NextBipEdge>>& previousBipRelationship,
                     const std::unordered_map<const TNode*, std::unordered_set<std::string>>& usesMapping,
                     const std::unordered_map<const TNode*, std::unordered_set<std::string>>& modifiesMapping) {
    // This is a small modification to the Affects algorithm.
    // @see getAffectsMapping

    typedef std::unordered_map<VARIABLE_NAME, ScopedStatements> VariableToAssigners;

    // We don't store every scoped statement, as there could be an exponential number of them.
    // It is thus heuristically (but not asymptotically) better to realize
    // ScopedStatements when we run the algorithm.
    std::map<ScopedStatement, VariableToAssigners> variablesReachingIn;
    std::map<ScopedStatement, VariableToAssigners> variablesGoingOut;

    // Initialize
    std::unordered_set<const TNode*> startingTNodes;
    startingTNodes.insert(tNodeTypeToTNodes.at(Assign).begin(), tNodeTypeToTNodes.at(Assign).end());
    for (const TNode* tNode : startingTNodes) {
        // Make this assignment produce affects information
        ScopedStatement scopedStatement = { tNodeToStatementNumber.at(tNode), Scope() };
        VariableToAssigners initialVariableToAssigners;
        for (const VARIABLE_NAME& variable : modifiesMapping.at(tNode)) {
            initialVariableToAssigners[variable] = ScopedStatements();
            initialVariableToAssigners[variable].insert(scopedStatement);
        }
        variablesGoingOut[scopedStatement] = initialVariableToAssigners;
    }

    std::vector<ScopedStatement> changedScopedStatements;
    // Unlike Affects, we don't have every statement in the program (e.g. dummy statements)
    // so we initialize the algorithm with the statements that occur right after the assignments
    // (with their appropriate scope)
    for (const TNode* startingTNode : startingTNodes) {
        STATEMENT_NUMBER startNodeStatementNumber = tNodeToStatementNumber.at(startingTNode);
        for (const NextBipEdge& edge : nextBipRelationship.at(startNodeStatementNumber)) {
            STATEMENT_NUMBER nextStatementNumber = edge.nextLine;
            Scope scope;
            if (edge.isBranchLineEdge()) {
                scope.push_back(edge.label);
            } else if (edge.isBranchBackEdge()) {
                continue;
            }

            changedScopedStatements.emplace_back(nextStatementNumber, scope);
        }
    }

    while (!changedScopedStatements.empty()) {
        ScopedStatement scopedStatement = changedScopedStatements.back();
        changedScopedStatements.pop_back();

        STATEMENT_NUMBER statementNumber;
        Scope currentScope;
        std::tie(statementNumber, currentScope) = scopedStatement;

        variablesReachingIn[scopedStatement] = VariableToAssigners();
        VariableToAssigners& statementAffects = variablesReachingIn.at(scopedStatement);

        if (previousBipRelationship.find(statementNumber) != previousBipRelationship.end()) {
            for (const NextBipEdge previousBipEdge : previousBipRelationship.at(statementNumber)) {
                Scope previousStatementScope = currentScope;
                // If we are in a scope that could not possibly be created by
                if (previousBipEdge.reversed().isBranchLineEdge()) {
                    // If there is no scope here, we should not have come from this NextBipEdge.
                    if (currentScope.empty()) {
                        continue;
                    }

                    // If we took a branch that is labelled differently from the top of the scope,
                    // we should not have come from this NextBipEdge.
                    if (currentScope.back() != previousBipEdge.label) {
                        continue;
                    }

                    // Since we came from a branch that is, labelled the same as the top of the
                    // current scope, the current scope didnt exist in the previous statement.
                    previousStatementScope.pop_back();
                } else if (previousBipEdge.reversed().isBranchBackEdge()) {
                    // If we returned from a procedure, make sure that the previous statement's
                    // scope indicates that it was inside said procedure.
                    previousStatementScope.push_back(previousBipEdge.label);
                }

                // Post-condition: This ScopedStatement can be reached via this NextBipEdge.

                // Propagate the affected variables (and who caused them to be affected) from
                // previous statements.
                STATEMENT_NUMBER previousStatement = previousBipEdge.reversed().prevLine;
                const ScopedStatement previousScopedStatement = { previousStatement, previousStatementScope };
                for (auto& p1 : variablesGoingOut[previousScopedStatement]) {
                    const VARIABLE_NAME& variable = p1.first;
                    const ScopedStatements& newAffectors = p1.second;

                    if (statementAffects.find(variable) == statementAffects.end()) {
                        statementAffects[variable] = ScopedStatements();
                    }

                    statementAffects[variable].insert(newAffectors.begin(), newAffectors.end());
                }
            }
        }


        // Make a copy of the old affects
        const VariableToAssigners oldOutwardAffects = variablesGoingOut[scopedStatement];

        // OUT := IN
        variablesGoingOut[scopedStatement] = variablesReachingIn[scopedStatement];

        // KILL and GEN variables if this is not a dummy node.
        if (statementNumberToTNode.find(statementNumber) != statementNumberToTNode.end()) {
            const TNode* tNode = statementNumberToTNode.at(statementNumber);

            // - KILL modified variables. Unlike Affects, we don't KILL for Call statements.
            if (tNode->type == Assign || tNode->type == Read) {
                if (modifiesMapping.find(tNode) != modifiesMapping.end()) {
                    const VARIABLE_NAME_SET& modifiedVariables = modifiesMapping.at(tNode);
                    for (const VARIABLE_NAME& modifiedVariable : modifiedVariables) {
                        variablesGoingOut[scopedStatement].erase(modifiedVariable);
                    }
                }
            }

            // + GEN new variables
            if (tNode->type == Assign) {
                assert(modifiesMapping.at(tNode).size() == 1);
                VARIABLE_NAME variableModified = *modifiesMapping.at(tNode).begin();
                variablesGoingOut[scopedStatement][variableModified] = { scopedStatement };
            }
        }


        // If there were new affected variables,
        if (variablesGoingOut[scopedStatement] != oldOutwardAffects) {
            if (nextBipRelationship.find(statementNumber) == nextBipRelationship.end()) {
                continue;
            }
            for (const NextBipEdge& nextBipEdge : nextBipRelationship.at(statementNumber)) {
                STATEMENT_NUMBER nextStatement = nextBipEdge.nextLine;
                Scope nextStatementScope = currentScope;
                if (nextBipEdge.isBranchBackEdge()) {
                    // If we are trying to "return" from a call we didnt make, dont traverse
                    // this nextBipEdge.
                    if (currentScope.empty()) {
                        continue;
                    }

                    if (currentScope.back() != nextBipEdge.label) {
                        continue;
                    }
                }
                // Post-condition: The next ScopedStatement can be reached via this NextBipEdge.

                // Modify scope
                if (nextBipEdge.isBranchLineEdge()) {
                    nextStatementScope.push_back(nextBipEdge.label);
                } else if (nextBipEdge.isBranchBackEdge()) {
                    nextStatementScope.pop_back();
                }

                changedScopedStatements.emplace_back(nextStatement, nextStatementScope);
            }
        }
    }

    std::unordered_map<STATEMENT_NUMBER, STATEMENT_NUMBER_SET> unscopedAffectsBipMapping;
    std::map<ScopedStatement, ScopedStatements> scopedAffectsBipMapping;
    for (const auto& p : variablesReachingIn) {
        ScopedStatement inStatement = p.first;
        const VariableToAssigners& variableToAssigners = p.second;

        STATEMENT_NUMBER statementNumber = inStatement.first;

        if (statementNumberToTNode.find(statementNumber) == statementNumberToTNode.end()) {
            continue;
        }
        const TNode* tNode = statementNumberToTNode.at(statementNumber);

        // Only assignments affect each other
        if (tNode->type != Assign) {
            continue;
        }

        // If this assignment variable uses nothing, nothing can possibly Affect it.
        if (usesMapping.find(tNode) == usesMapping.end()) {
            continue;
        }
        auto& tNodeUses = usesMapping.at(tNode);

        for (const auto& p1 : variableToAssigners) {
            VARIABLE_NAME variable = p1.first;
            const ScopedStatements& affectors = p1.second;

            // If this statement does not use this variable, skip it.
            if (tNodeUses.find(variable) == tNodeUses.end()) {
                continue;
            }

            for (const ScopedStatement& affector : affectors) {
                STATEMENT_NUMBER affectingStatementNumber = affector.first;

                if (statementNumberToTNode.find(affectingStatementNumber) == statementNumberToTNode.end()) {
                    continue;
                }
                const TNode* affectorTNode = statementNumberToTNode.at(affectingStatementNumber);

                // Only assignments affect each other
                if (affectorTNode->type != Assign) {
                    continue;
                }

                if (unscopedAffectsBipMapping.find(affectingStatementNumber) ==
                    unscopedAffectsBipMapping.end())
                    unscopedAffectsBipMapping[affectingStatementNumber] = STATEMENT_NUMBER_SET();
                unscopedAffectsBipMapping[affectingStatementNumber].insert(statementNumber);

                if (scopedAffectsBipMapping.find(affector) == scopedAffectsBipMapping.end())
                    scopedAffectsBipMapping[affector] = ScopedStatements();
                scopedAffectsBipMapping[affector].insert(inStatement);
            }
        }
    }
    return { unscopedAffectsBipMapping, scopedAffectsBipMapping };
}

std::pair<std::unordered_map<STATEMENT_NUMBER, STATEMENT_NUMBER_SET>, std::map<ScopedStatement, ScopedStatements>>
getAffectedBipMapping(const std::pair<std::unordered_map<STATEMENT_NUMBER, STATEMENT_NUMBER_SET>,
                                      std::map<ScopedStatement, ScopedStatements>>& affectsBipMapping) {
    std::unordered_map<STATEMENT_NUMBER, STATEMENT_NUMBER_SET> unscopedAffectedBipMapping =
    getAffectedMapping(affectsBipMapping.first);

    std::map<ScopedStatement, ScopedStatements> scopedAffectedBipMapping;
    for (const auto& p : affectsBipMapping.second) {
        const ScopedStatement& affector = p.first;
        for (const ScopedStatement& affected : p.second) {
            if (scopedAffectedBipMapping.find(affected) == scopedAffectedBipMapping.end()) {
                scopedAffectedBipMapping[affected] = ScopedStatements();
            }
            scopedAffectedBipMapping[affected].insert(affector);
        }
    }

    return { unscopedAffectedBipMapping, scopedAffectedBipMapping };
}


} // namespace extractor
} // namespace backend
