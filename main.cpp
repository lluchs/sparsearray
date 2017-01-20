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
	float x,y,xdir,ydir;
};

template<typename SparseArray>
void benchmark(int iterations, uint64_t seed)
{
	uint64_t r = seed;
	auto rand = [&r]() { return r = r * 6364136223846793005 + 1442695040888963407; };
	SparseArray array;

	for (int i = 0; i < iterations; i++)
	{
		// Add new PXS periodically.
		for (int j = 0; j < 10; j++)
		{
			auto npxs = array.New();
			if (npxs)
			{
				npxs->Mat = 1;
				npxs->xdir = rand() % 100 - 50;
				npxs->ydir = rand() % 100 - 50;
			}
		}
		// walk through the array and do stuff
		for (auto pxs : array)
		{
			pxs->x += pxs->xdir; pxs->y += pxs->ydir;
			if (std::abs(pxs->x + pxs->y) > 10000.)
			{
				pxs->Mat = C4PXS::MNone;
				array.Delete(&*pxs);
			}
		}
	}
}

int main()
{
	int iterations = 1000000;
	uint64_t seed = 199897253124;

	std::chrono::time_point<std::chrono::high_resolution_clock> start, end;
	std::chrono::duration<double> elapsed_seconds;

	std::cout << "start BitmapSA" << std::endl;
	start = std::chrono::high_resolution_clock::now();
	benchmark<BitmapSA<C4PXS, 10000>>(iterations, seed);
	end = std::chrono::high_resolution_clock::now();
	elapsed_seconds = end - start;
	std::cout << "end BitmapSA = " << std::chrono::duration_cast<std::chrono::microseconds>(elapsed_seconds).count() << " μs" << std::endl;

	std::cout << "start ChunkSA" << std::endl;
	start = std::chrono::high_resolution_clock::now();
	benchmark<ChunkSA<C4PXS, 10000>>(iterations, seed);
	end = std::chrono::high_resolution_clock::now();
	elapsed_seconds = end - start;
	std::cout << "end ChunkSA = " << std::chrono::duration_cast<std::chrono::microseconds>(elapsed_seconds).count() << " μs" << std::endl;

	return 0;
}
