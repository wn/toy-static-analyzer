#include "ResultTable.h"
#include "catch.hpp"

namespace qpbackend {
TEST_CASE("get table columns") {
    std::vector<std::string> header = { "A", "B", "C", "D" };
    std::unordered_set<std::vector<std::string>, StringVectorHash> content = {
        { "11", "12", "13", "14" },
        { "11", "12", "23", "24" },
        { "21", "22", "13", "14" },
        { "21", "22", "23", "24" },
    };
    ResultTable rt(header, content);
    std::unordered_set<std::string> s1, s1_expected;
    std::unordered_set<std::vector<std::string>, StringVectorHash> s2, s2_expected;

    // test synonyms exist
    REQUIRE(rt.updateSynonymValueSet("A", s1));
    REQUIRE(rt.updateSynonymValueTupleSet({ "A", "B" }, s2));
    s1_expected = { "11", "21" };
    s2_expected = { { "11", "12" }, { "21", "22" } };
    REQUIRE(s1 == s1_expected);
    REQUIRE(s2 == s2_expected);

    // test conversion to vectors
    std::vector<std::string> v1, v1_expected;
    std::vector<std::vector<std::string>> v2, v2_expected;
    REQUIRE(rt.updateSynonymValueVector("A", v1));
    REQUIRE(rt.updateSynonymValueTupleVector({ "A", "B" }, v2));
    v1_expected = { "11", "21" };
    v2_expected = { { "11", "12" }, { "21", "22" } };
    std::sort(v1.begin(), v1.end());
    std::sort(v2.begin(), v2.end(), CompareStrVec());
    REQUIRE(s1 == s1_expected);
    REQUIRE(s2 == s2_expected);

    // test synonyms not exist
    REQUIRE_FALSE(rt.updateSynonymValueSet("E", s1));
    REQUIRE_FALSE(rt.updateSynonymValueTupleSet({ "A", "E" }, s2));
    REQUIRE(s1 == s1_expected); // remain unchanged
    REQUIRE(s2 == s2_expected); // remain unchanged
}

TEST_CASE("merge table without common columns") {
    std::vector<std::string> header1 = { "A", "B" };
    std::vector<std::string> header2 = { "C", "D" };
    std::unordered_set<std::vector<std::string>, StringVectorHash> content1 = { { "11", "12" }, { "21", "22" } };
    std::unordered_set<std::vector<std::string>, StringVectorHash> content2 = { { "13", "14" }, { "23", "24" } };

    std::vector<std::string> header = { "A", "B", "C", "D" };
    std::unordered_set<std::vector<std::string>, StringVectorHash> content = {
        { "11", "12", "13", "14" },
        { "11", "12", "23", "24" },
        { "21", "22", "13", "14" },
        { "21", "22", "23", "24" },
    };

    ResultTable rt1(header1, content1);
    ResultTable rt2(header2, content2);
    ResultTable rtExpected(header, content);
    rt1.mergeTable(std::move(rt2));

    // sort the table
    rt1.sortTable();
    rtExpected.sortTable();

    REQUIRE(rt1 == rtExpected);
}

TEST_CASE("merge table with common columns") {
    std::vector<std::string> header1 = { "A", "B", "C" };
    std::vector<std::string> header2 = { "D", "B", "A" };
    std::unordered_set<std::vector<std::string>, StringVectorHash> content1 = { { "11", "22", "33" },
                                                                                { "12", "22", "30" } };
    std::unordered_set<std::vector<std::string>, StringVectorHash> content2 = { { "40", "21", "11" },
                                                                                { "44", "22", "11" } };

    std::vector<std::string> header = { "A", "B", "C", "D" };
    std::unordered_set<std::vector<std::string>, StringVectorHash> content = {
        { "11", "22", "33", "44" },
    };

    ResultTable rt1(header1, content1);
    ResultTable rt2(header2, content2);
    ResultTable rtExpected(header, content);
    rt1.mergeTable(std::move(rt2));

    // sort the table
    rt1.sortTable();
    rtExpected.sortTable();

    REQUIRE(rt1 == rtExpected);
}

TEST_CASE("merge table with subsets") {
    std::vector<std::string> header1 = { "A", "B", "C" };
    std::vector<std::string> header2 = { "B", "A" };
    std::unordered_set<std::vector<std::string>, StringVectorHash> content1 = { { "11", "22", "31" },
                                                                                { "11", "22", "32" },
                                                                                { "12", "22", "30" } };
    std::unordered_set<std::vector<std::string>, StringVectorHash> content2 = { { "22", "11" } };

    std::vector<std::string> header = { "A", "B", "C" };
    std::unordered_set<std::vector<std::string>, StringVectorHash> content = { { "11", "22", "31" },
                                                                               { "11", "22", "32" } };

    ResultTable rt1(header1, content1);
    ResultTable rt2(header2, content2);
    ResultTable rtExpected(header, content);
    rt1.mergeTable(std::move(rt2));

    // sort the table
    rt1.sortTable();
    rtExpected.sortTable();

    REQUIRE(rt1 == rtExpected);
}
} // namespace qpbackend
