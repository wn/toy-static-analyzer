#include "DesignExtractor.h"

#include "Logger.h"
#include "TNode.h"

#include <stack>
#include <unordered_map>

using namespace std;

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

std::pair<std::unordered_map<int, int>, std::unordered_map<int, int>> getFollowRelationship(const TNode& ast) {
    auto tNodeTypeToTNode = getTNodeTypeToTNodes(ast);
    auto tNodeToStmtNo = getTNodeToStatementNumber(ast);
    std::unordered_map<int, int> followerFollowedMap;
    std::unordered_map<int, int> followedFollowerMap;

    for (const TNode* stmtList : tNodeTypeToTNode[StatementList]) {
        const std::vector<TNode>& children = stmtList->children;
        for (int i = 1; i < children.size(); ++i) {
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
