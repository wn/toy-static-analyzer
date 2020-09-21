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

const TNode*
getProcedureFromProcedureName(const std::string& procedureName,
                              std::unordered_map<TNodeType, std::vector<const TNode*>, EnumClassHash>& tNodeTypeToTNodes) {
    for (auto tNode : tNodeTypeToTNodes[TNodeType::Procedure]) {
        if (tNode->name == procedureName) {
            return tNode;
        }
    }
    throw std::runtime_error("Error: Could not find procedure " + procedureName);
}

std::unordered_map<const TNode*, std::vector<const TNode*>>
getProcedureToCallers(std::unordered_map<TNodeType, std::vector<const TNode*>, EnumClassHash>& tNodeTypeToTNodes) {
    std::unordered_map<const TNode*, std::vector<const TNode*>> procedureToCallers;
    logLine("getProcedureToCallers started");
    for (const auto& callingProcedure : tNodeTypeToTNodes[TNodeType::Procedure]) {
        logLine("getProcedureToCallers: Looking at procedure " + callingProcedure->toShortString());
        // Get procedures called by this procedure
        std::vector<const TNode*> stack = { callingProcedure };
        while (!stack.empty()) {
            const TNode* tNode = stack.back();
            stack.pop_back();
            logLine("getProcedureToCallers: Looking at tNode " + tNode->toShortString());

            for (auto& child : tNode->children) {
                stack.push_back(&child);
            }

            if (tNode->type != TNodeType::Call) {
                continue;
            }

            logLine("getProcedureToCallers: Found call statement " + tNode->toShortString());
            auto procedureName = tNode->children[0].name;
            auto calledProcedure = getProcedureFromProcedureName(procedureName, tNodeTypeToTNodes);
            if (procedureToCallers.find(calledProcedure) == procedureToCallers.end()) {
                logLine("getProcedureToCallers: Initializing empty list for called procedures" +
                        calledProcedure->toShortString());
                procedureToCallers[calledProcedure] = std::vector<const TNode*>();
            }
            logLine("getProcedureToCallers: Adding caller" + callingProcedure->toShortString() +
                    " for " + calledProcedure->toShortString());
            procedureToCallers[calledProcedure].push_back(callingProcedure);
        }
    }
    return procedureToCallers;
}


/**
 * A modified DFS is used to get a topological ordering of the "called-by" graph.
 * @param currentNode
 * @param procedureToCallers a mapping of procedure to procedures that call it.
 * @param topologicalOrderOfCalledByGraph the result vector to store the topological order in.
 * Note that we push_back into the vector, which means that it must be reversed to be a valid
 * topological ordering of the reverse cal graph.
 */
void getTopologicalOrderingOfCalledByGraphHelper(
const TNode* currentNode,
const std::unordered_map<const TNode*, std::vector<const TNode*>>& procedureToCallers,
std::vector<const TNode*>& topologicalOrderOfCalledByGraph) {
    if (std::find(topologicalOrderOfCalledByGraph.begin(), topologicalOrderOfCalledByGraph.end(),
                  currentNode) != topologicalOrderOfCalledByGraph.end()) {
        return;
    }
    logLine("getTopologicalOrderingOfCalledByGraphHelper: visiting " + currentNode->toShortString());
    if (procedureToCallers.find(currentNode) != procedureToCallers.end()) {
        for (const auto& caller : procedureToCallers.find(currentNode)->second) {
            getTopologicalOrderingOfCalledByGraphHelper(caller, procedureToCallers, topologicalOrderOfCalledByGraph);
        }
    }
    logLine("getTopologicalOrderingOfCalledByGraphHelper: adding " + currentNode->toShortString() + " to topo order");
    topologicalOrderOfCalledByGraph.push_back(currentNode);
}

std::vector<const TNode*> getTopologicalOrderingOfCalledByGraph(
std::unordered_map<TNodeType, std::vector<const TNode*>, EnumClassHash>& tNodeTypeToTNodes) {
    const std::unordered_map<const TNode*, std::vector<const TNode*>>& procedureToCallers =
    getProcedureToCallers(tNodeTypeToTNodes);
    std::vector<const TNode*> topologicalOrderResult;
    for (const auto& procedure : tNodeTypeToTNodes[TNodeType::Procedure]) {
        getTopologicalOrderingOfCalledByGraphHelper(procedure, procedureToCallers, topologicalOrderResult);
    }
    std::reverse(topologicalOrderResult.begin(), topologicalOrderResult.end());
    return topologicalOrderResult;
}

/**
 * A DFS Helper that is used to compute the Uses mapping.
 * Prerequisite: any procedure(s) that currentNode calls must have been indexed, i.e. it must exist
 * in the usesMapping.
 * @param currentNode
 * @param tNodeTypeToTNodes
 * @param usesMapping a reference to the mapping, that will be updated with the Uses information for
 * the currentNode.
 */
void getUsesMappingHelper(const TNode* currentNode,
                          std::unordered_map<TNodeType, std::vector<const TNode*>, EnumClassHash>& tNodeTypeToTNodes,
                          std::unordered_map<const TNode*, std::unordered_set<std::string>>& usesMapping) {
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
        // We expect the called procedure to have been indexed already
        if (usesMapping.find(calledProc) == usesMapping.end()) {
            throw std::runtime_error("Expected Procedure " + calledProc->toShortString() + " to exist in the Uses mapping, as it is called by the call statement " +
                                     currentNode->toShortString());
        }
        // This call statement uses all the variables used by the called procedure
        variablesUsedByCurrentTNode.insert(usesMapping[calledProc].begin(), usesMapping[calledProc].end());
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
    for (const auto procedure : getTopologicalOrderingOfCalledByGraph(tNodeTypeToTNodes)) {
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
        if (validTypesForUses.find(p.first->type) == validTypesForUses.end()) {
            keysToDelete.push_back(p.first);
        }
    }

    for (auto& keyToDelete : keysToDelete) {
        usesMapping.erase(keyToDelete);
    }

    return usesMapping;
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

} // namespace extractor
} // namespace backend
