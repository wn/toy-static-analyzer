#include "TNode.h"
#include "catch.hpp"
namespace backend {
namespace testTNode {

TEST_CASE("Test equality") {
    TNode root(TNodeType::While);
    TNode s1(TNodeType::Assign);
    root.addChild(s1);

    TNode root2(TNodeType::While);
    TNode s2(TNodeType::Assign);
    root2.addChild(s2);

    REQUIRE(root == root2);
}

TEST_CASE("Test name inequality") {
    TNode root(TNodeType::While);
    TNode s1(TNodeType::Assign);
    s1.name = "abc";
    root.addChild(s1);

    TNode root2(TNodeType::While);
    TNode s2(TNodeType::Assign);
    s2.name = "xyz";
    root2.addChild(s2);

    REQUIRE_FALSE(root == root2);
}

TEST_CASE("Test constant inequality") {
    TNode root(TNodeType::While);
    TNode s1(TNodeType::Assign);
    s1.constant = 1;
    root.addChild(s1);

    TNode root2(TNodeType::While);
    TNode s2(TNodeType::Assign);
    s2.constant = 0;
    root2.addChild(s2);

    REQUIRE_FALSE(root == root2);
}
} // namespace testTNode
} // namespace backend
