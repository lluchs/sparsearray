#include "catch.hpp"
#include "../sparsearray.h"

TEST_CASE("BitmapSA: Basic actions", "[BitmapSA]")
{
    // We need to test more than 64 to verify calculations with
    // multiple mask elements.
    constexpr int N = 100;
    BitmapSA<int, N> array;

#include "common.h"
}
