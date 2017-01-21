#include "catch.hpp"
#include "../sparsearray.h"

TEST_CASE("LinkedListSA: Basic actions", "[LinkedListSA]")
{
    constexpr int N = 10;
    LinkedListSA<int, N> array;

#include "common.h"
}

typedef LinkedListSA<int, 3> LinkedListSAIntThree;

TEST_CASE_METHOD(LinkedListSAIntThree, "LinkedListSA: Internals", "[LinkedListSA]")
{
    int *one = New(), *two = New(), *three = New();
    *one = 1; *two = 2; *three = 3;

    SECTION("New should set pointers correctly")
    {
        REQUIRE(&firstUsed->data == one);
        REQUIRE(firstFree == nullptr);
    }

    SECTION("Delete should set pointers correctly")
    {
        Delete(two);
        REQUIRE(&firstFree->data == two);
        REQUIRE(firstFree->next == nullptr);
        REQUIRE(&firstUsed->next->data == three);
    }

    SECTION("out-of-order deletion should set pointers correctly")
    {
        Delete(two); Delete(three); Delete(one);
        REQUIRE(firstUsed == nullptr);
        REQUIRE(firstFree->data == 1);
        REQUIRE(firstFree->next->data == 2);
        REQUIRE(firstFree->next->next->data == 3);
        REQUIRE(firstFree->next->next->next == nullptr);
    }

    SECTION("insertion should keep order")
    {
        Delete(three); Delete(one);
        REQUIRE(*New() == 1);
        REQUIRE(*New() == 3);
        REQUIRE(firstUsed->data == 1);
    }

}
