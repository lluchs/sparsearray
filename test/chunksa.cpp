#include "catch.hpp"
#include "../sparsearray.h"

TEST_CASE("ChunkSA: Basic actions", "[ChunkSA]")
{
    constexpr int N = 10;
    ChunkSA<int, N, 2> array;

#include "common.h"
}

TEST_CASE("StaticChunkSA: Basic actions", "[StaticChunkSA]")
{
    constexpr int N = 10;
    StaticChunkSA<int, N, 2> array;

#include "common.h"
}
