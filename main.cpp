#include <iostream>
#include <chrono>
#include <cassert>
#include <cmath>

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

template<typename SparseArray>
int benchmark(int iterations, uint64_t seed)
{
	uint64_t r = seed;
	auto rand = [&r]() { return r = r * 6364136223846793005 + 1442695040888963407; };
	SparseArray array;
	int sum = 0;

	for (int i = 0; i < iterations; i++)
	{
		// Add new PXS periodically.
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
		sum += pxs.x + pxs.y;
	return sum;
}

template<typename SparseArray>
static void run_benchmark(const char *name)
{
	const int iterations = 100000;
	const uint64_t seed = 199897253124;

	std::chrono::time_point<std::chrono::high_resolution_clock> start, end;
	std::chrono::duration<double> elapsed_seconds;
	int sum;

	std::cout << "start " << name << std::endl;
	start = std::chrono::high_resolution_clock::now();
	sum = benchmark<SparseArray>(iterations, seed);
	end = std::chrono::high_resolution_clock::now();
	elapsed_seconds = end - start;
	std::cout << "end = " << std::chrono::duration_cast<std::chrono::microseconds>(elapsed_seconds).count() << " μs" << std::endl;
	std::cout << "sum = " << sum << std::endl << std::endl;
}

int main()
{
	const size_t list_size = 10000;

	run_benchmark<BitmapSA<C4PXS, list_size>>("BitmapSA");
	run_benchmark<ChunkSA<C4PXS, list_size>>("ChunkSA");
	run_benchmark<LinkedListSA<C4PXS, list_size>>("LinkedListSA");
	run_benchmark<DoubleLinkedListSA<C4PXS, list_size>>("DoubleLinkedListSA");
	run_benchmark<UnorderedLinkedListSA<C4PXS, list_size>>("UnorderedLinkedListSA");

	return 0;
}
