#include <catch2/catch_test_macros.hpp>
#include "Allocators.h"
#include "List.h"
#include <map>

TEST_CASE("Fixed allocator test") {
    auto a1 = allocators::fixed::Allocator<int, 3>{};
    auto a2 = allocators::fixed::Allocator<int, 3>{};

    REQUIRE(a1 == a2);

    auto p1 = a1.allocate(3);
    auto p2 = a2.allocate(3);

    new(p1) int[3];
    new(p2) int[3];

    p1[0] = 3;
    p1[1] = 8;
    p1[2] = 7;

    REQUIRE(a1 != a2);

    p2[0] = 3;
    p2[1] = 8;
    p2[2] = 7;

    REQUIRE(a1 == a2);

    auto a3 = std::move(a1);

    REQUIRE(a3 == a3);
    REQUIRE(a2 == a3);
    REQUIRE(a1 != a2);

    p2[1] = 25;

    REQUIRE(a2 != a3);

    REQUIRE_THROWS_AS(a2.allocate(3), std::overflow_error);
}

TEST_CASE("Extensible allocator test") {
    auto a1 = allocators::extensible::Allocator<int, 2>{};
    auto a2 = allocators::extensible::Allocator<int, 2>{};

    REQUIRE(a1 == a2);

    auto p1_0 = a1.allocate(1);
    auto p1_1 = a1.allocate(1);

    auto p1_0_cr = new(p1_0) int[1];
    auto p1_1_cr = new(p1_1) int[1];

    *p1_0_cr = 59;
    *p1_1_cr = 36;

    REQUIRE(a1 != a2);

    auto p2_0 = a2.allocate(1);
    auto p2_1 = a2.allocate(1);

    auto p2_0_cr = new(p2_0) int[1];
    auto p2_1_cr = new(p2_1) int[1];

    *p2_0_cr = 59;
    *p2_1_cr = 36;

    REQUIRE(a1 == a2);

    auto a3 = std::move(a1);

    REQUIRE(a3 == a3);
    REQUIRE(a1 != a3);
    REQUIRE(a2 == a3);

    *p1_1_cr = 78;

    REQUIRE(a2 != a3);

    *p2_1_cr = 78;

    REQUIRE(a2 == a3);

    auto p3_2 = a3.allocate(1);

    REQUIRE(a2 != a3);
    REQUIRE(a3.capacity() == 4);

    a3.deallocate(p3_2, 1);

    REQUIRE(a3.allocatedCount() == 2);
    REQUIRE_THROWS_AS(a3.deallocate(p1_0, 2), std::invalid_argument);
    REQUIRE_THROWS_AS(a2.allocate(3), std::invalid_argument);
}

TEST_CASE("Container test with fixed allocator") {
    std::map<int, std::string, std::less<int>, allocators::fixed::Allocator<std::pair<const int, std::string>, 3> > map1;
    map1[0] = "test0";
    map1[1] = "test1";

    auto map2 = map1;

    REQUIRE(map1 == map2);

    map2[2] = "test2";

    REQUIRE(map1.size() == 2);
    REQUIRE(map2.size() == 3);
    REQUIRE(map1[0] == "test0");
    REQUIRE(map1[1] == "test1");
    REQUIRE(map2[0] == "test0");
    REQUIRE(map2[1] == "test1");
    REQUIRE(map2[2] == "test2");
    REQUIRE_THROWS_AS(map2[3] = "test3", std::overflow_error);
}

TEST_CASE("Container test with extensible allocator") {
    std::map<int, std::string, std::less<int>, allocators::extensible::Allocator<std::pair<const int, std::string>, 3> > map1;
    map1[0] = "test0";
    map1[1] = "test1";

    auto map2 = map1;

    REQUIRE(map1 == map2);

    map2[2] = "test2";
    map2[3] = "test3";

    REQUIRE(map1.size() == 2);
    REQUIRE(map2.size() == 4);
    REQUIRE(map1[0] == "test0");
    REQUIRE(map1[1] == "test1");
    REQUIRE(map2[0] == "test0");
    REQUIRE(map2[1] == "test1");
    REQUIRE(map2[2] == "test2");
    REQUIRE(map2[3] == "test3");
}

TEST_CASE("Custom container test") {
    containers::List<std::string, allocators::extensible::Allocator<std::string, 3>> list;

    list.append("first");
    list.append("second");
    list.append("third");

    auto s1 = list.getValue();
    auto s2 = list.getValue();
    auto s3 = list.getValue();

    REQUIRE(s1.value_or("") == "first");
    REQUIRE(s2.value_or("") == "second");
    REQUIRE(s3.value_or("") == "third");

    list.append("fourth");

    auto s4 = list.getValue();

    REQUIRE(s4.value_or("") == "fourth");
}
