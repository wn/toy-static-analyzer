#include "DesignExtractor.h"

#include "Logger.h"
#include "TNode.h"

#include <set>
#include <stack>
#include <unordered_map>
#include <unordered_set>


namespace backend {
namespace extractor {

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


std::pair<std::unordered_map<int, int>, std::unordered_map<int, std::vector<int>>>
getParentRelationship(const TNode& ast) {
    auto tNodetypeToTNode = getTNodeTypeToTNodes(ast);
    auto tNodeToStmtNo = getTNodeToStatementNumber(ast);

    std::unordered_map<int, int> childParentMap;
    std::unordered_map<int, std::vector<int>> parentChildrenMap;


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
            parentChildrenMap[parentStmtNo].push_back(childStmtNo);
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
            parentChildrenMap[parentStmtNo].push_back(childStmtNo);
        }

        // else-stmts
        const TNode& elseStmtLists = parent->children[2];
        for (const TNode& stmt : elseStmtLists.children) {
            int childStmtNo = tNodeToStmtNo[&stmt]; // we do not expect this to throw as child has stmt_no.
            childParentMap[childStmtNo] = parentStmtNo;
            parentChildrenMap[parentStmtNo].push_back(childStmtNo);
        }
    }
    return { childParentMap, parentChildrenMap };
}

std::vector<int> getVisitedPathFromStart(int start, const std::unordered_map<int, int>& relation) {
    std::vector<int> result;
    auto it = relation.find(start);
    while (it != relation.end()) {
        int next = it->second;
        result.push_back(next);
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


} // namespace extractor
} // namespace backend
