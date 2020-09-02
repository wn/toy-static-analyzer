#include "TNode.h"

#include "catch.hpp"
namespace backend {
namespace testTNode {
void require(bool b) {
    REQUIRE(b);
}

TEST_CASE("Test equality") {
    TNode root(TNodeType::While);
    TNode s1(TNodeType::Assign);
    root.addChild(s1);

    TNode root2(TNodeType::While);
    TNode s2(TNodeType::Assign);
    root2.addChild(s2);

    require(root == root2);
}
} // namespace testTNode
} // namespace backend
