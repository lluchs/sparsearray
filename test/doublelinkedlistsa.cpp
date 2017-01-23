#include "catch.hpp"
#include "../sparsearray.h"

TEST_CASE("DoubleLinkedListSA: Basic actions", "[DoubleLinkedListSA]")
{
    constexpr int N = 10;
    DoubleLinkedListSA<int, N> array;

#include "common.h"
}

typedef DoubleLinkedListSA<int, 3> LinkedListSAIntThree;

TEST_CASE_METHOD(LinkedListSAIntThree, "DoubleLinkedListSA: Internals", "[DoubleLinkedListSA]")
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
        REQUIRE(firstUsed->next->data == 3);
        REQUIRE(firstUsed->next->prev->data == 1);
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

    SECTION("insertion should keep order and set prev-pointers correctly")
    {
        Delete(three); Delete(one);
        REQUIRE(*New() == 1);
        REQUIRE(*New() == 3);
        Delete(two);
        REQUIRE(*New() == 2);
        REQUIRE(firstUsed->data == 1);
        REQUIRE(firstUsed->next->prev->data == 1);
        REQUIRE(firstUsed->next->next->prev->data == 2);
    }

}
