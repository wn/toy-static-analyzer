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

} // namespace extractor
} // namespace backend
