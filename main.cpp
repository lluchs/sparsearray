#include <iostream>
#include <chrono>
#include <cassert>
#include <cmath>
#include <cstdlib>

#include <unistd.h>

#include "sparsearray.h"

// fake data class
class C4PXS
{
public:
	static const int32_t MNone = 0;
	C4PXS(): Mat(MNone), x(0), y(0), xdir(0), ydir(0) {}
	bool IsNull() { return Mat == MNone; }

	int32_t Mat;
	int x,y,xdir,ydir;
};

struct BenchmarkResult
{
	int count;
	int sum;
};

template<typename SparseArray>
BenchmarkResult benchmark(int iterations, uint64_t seed, int addmod)
{
	uint64_t r = seed;
	auto rand = [&r]() { return r = r * 6364136223846793005 + 1442695040888963407; };
	SparseArray array;
	BenchmarkResult result = {0};

	for (int i = 0; i < iterations; i++)
	{
		// Add new PXS periodically.
		if (i % addmod == 0)
			for (int j = 0; j < 10; j++)
			{
				auto npxs = array.New();
				if (npxs)
				{
					npxs->Mat = 1;
					npxs->x = 0; npxs->y = 0;
					npxs->xdir = (int) (rand() % 100) - 50;
					npxs->ydir = (int) (rand() % 100) - 50;
				}
			}
		// walk through the array and do stuff
		for (auto& pxs : array)
		{
			pxs.x += pxs.xdir; pxs.y += pxs.ydir;
			if (std::abs(pxs.x + pxs.y) > 10000)
			{
				pxs.Mat = C4PXS::MNone;
				array.Delete(&pxs);
			}
		}
	}

	for (auto& pxs : array)
	{
		result.count++;
		result.sum += pxs.x + pxs.y;
	}
	return result;
}

// Options
static int iterations = 100000;
static uint64_t seed = 199897253124;
static int addmod = 1;

template<typename SparseArray>
static void run_benchmark(const char *name)
{
	std::chrono::time_point<std::chrono::high_resolution_clock> start, end;
	std::chrono::duration<double> elapsed_seconds;

	std::cout << "start " << name << std::endl;
	start = std::chrono::high_resolution_clock::now();
	auto r = benchmark<SparseArray>(iterations, seed, addmod);
	end = std::chrono::high_resolution_clock::now();
	elapsed_seconds = end - start;
	std::cout << "end = " << std::chrono::duration_cast<std::chrono::microseconds>(elapsed_seconds).count() << " μs" << std::endl;
	std::cout << "static size = " << sizeof(SparseArray) << " byte" << std::endl;
	std::cout << "count = " << r.count << std::endl;
	std::cout << "sum = " << r.sum << std::endl << std::endl;
}

int main(int argc, char **argv)
{
	const size_t list_size = 10000;

	int opt;
	while ((opt = getopt(argc, argv, "l:i:s:a:")) != -1)
	{
		switch (opt)
		{
		// No error handling. Don't pass invalid values.
		case 'i': iterations = std::atoi(optarg); break;
		case 's': seed = std::strtoull(optarg, nullptr, 10); break;
		case 'a': addmod = std::atoi(optarg); break;
		default: std::cerr << "Invalid option " << (char) opt << std::endl;
		}
	}
	std::cout << "iterations = " << std::to_string(iterations) << std::endl;
	std::cout << "seed = " << std::to_string(seed) << std::endl;
	std::cout << "addmod = " << std::to_string(addmod) << std::endl;
	std::cout << "data size = " << sizeof(C4PXS[list_size]) << " byte" << std::endl << std::endl;

	run_benchmark<BitmapSA<C4PXS, list_size>>("BitmapSA");
	run_benchmark<ChunkSA<C4PXS, list_size>>("ChunkSA");
	run_benchmark<StaticChunkSA<C4PXS, list_size>>("StaticChunkSA");
	run_benchmark<LinkedListSA<C4PXS, list_size>>("LinkedListSA");
	run_benchmark<LinkedListBitmapSA<C4PXS, list_size>>("LinkedListBitmapSA");
	run_benchmark<DoubleLinkedListSA<C4PXS, list_size>>("DoubleLinkedListSA");
	run_benchmark<UnorderedLinkedListSA<C4PXS, list_size>>("UnorderedLinkedListSA");
	run_benchmark<ReorderingSA<C4PXS, list_size>>("ReorderingSA");

	return 0;
}
