#include "catch.hpp"
#include "../sparsearray.h"

TEST_CASE("Basic actions", "[LinkedListSA]")
{
    LinkedListSA<int, 10> array;

    SECTION("should be initially empty")
    {
        for (auto el : array)
            FAIL("not empty");
    }

    SECTION("filling should work")
    {
        for (int i = 0; i < 10; i++)
        {
            int *el = array.New();
            REQUIRE(el != nullptr);
            *el = i;
            CHECK(*el == i);
        }

        SECTION("New should return nullptr when full")
        {
            REQUIRE(array.New() == nullptr);
        }

        SECTION("iteration should work")
        {
            int i = 1;
            for (auto el : array)
                CHECK(*el == i++);
        }

        SECTION("deletion should work")
        {
            for (auto el : array)
                array.Delete(&*el);
            for (auto el : array)
            {
                CAPTURE(*el);
                FAIL("array should be empty");
            }
        }
    }
}
