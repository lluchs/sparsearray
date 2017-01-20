#pragma once

template<typename T, size_t N>
class BitmapSA
{
	T data[N];
	static constexpr size_t maskN = (N + 63) / 64;
	uint64_t mask[maskN] = {0};

public:
	T* New()
	{
		for (size_t i = 0; i < maskN; i++)
		{
			uint64_t m = ~mask[i];
			if (m)
			{
				size_t j = __builtin_ffsll(m) - 1;
				size_t idx = i*64 + j;
				if (idx >= N) return nullptr;
				mask[i] |= 1 << j;
				return &data[idx];
			}
		}
		return nullptr;
	}

	void Delete(T *el)
	{
		size_t idx = el - data;
		assert(idx < N);
		size_t i = idx / 64, j = idx % 64;
		mask[i] &= ~(1 << j);
	}

	template<typename Ti, typename SA = BitmapSA>
	class Iterator : public std::iterator<std::forward_iterator_tag, Ti>
	{
		SA *array;
		size_t el, pos; // el: current element in data, pos: current bit position in cur
		uint64_t cur;
	public:
		Iterator(SA *array) : array(array), el(0), pos(64), cur(0) { }

		Iterator& operator++()
		{
			while (!cur && el < N)
			{
				el += 64 - pos;
				pos = 0;
				cur = array->mask[el / 64];
			}
			size_t inc = __builtin_ffsll(cur) - 1;
			el += inc;
			pos += inc + 1;
			cur >>= inc + 1;
			if (el >= N)
			{
				array = nullptr;
				el = 0;
			}
			return *this;
		}

		bool operator==(Iterator other) { return array == other.array && el == other.el; }
		bool operator!=(Iterator other) { return !(*this == other); }
		Ti* operator*() const { return &array->data[el]; }
	};

	Iterator<T> begin() { return Iterator<T>(this); }
	Iterator<T> end() { return Iterator<T>(nullptr); }
	Iterator<const T, const BitmapSA> begin() const { return Iterator<const T, const BitmapSA>(this); }
	Iterator<const T, const BitmapSA> end() const { return Iterator<const T, const BitmapSA>(nullptr); }
};

template<typename T, size_t N, size_t ChunkSize = 500>
class ChunkSA
{
	static_assert(N % ChunkSize == 0, "N must be a multiple of ChunkSize");
	static constexpr size_t MaxChunk = N / ChunkSize;
	T *Chunk[MaxChunk];
	size_t ChunkFill[MaxChunk];

public:

	ChunkSA()
	{
		for (size_t i = 0; i < MaxChunk; i++)
		{
			Chunk[i] = nullptr;
			ChunkFill[i] = 0;
		}
	}

	~ChunkSA()
	{
		for (size_t i = 0; i < MaxChunk; i++)
		{
			if (Chunk[i]) delete[] Chunk[i];
			Chunk[i] = nullptr;
			ChunkFill[i] = 0;
		}
	}

	T* New()
	{
		for (size_t i = 0; i < MaxChunk; i++)
		{
			// Create new chunk if necessary.
			if (!Chunk[i])
			{
				Chunk[i] = new T[ChunkSize];
				ChunkFill[i] = 0;
			}
			// Check this chunk for space.
			if (ChunkFill[i] < ChunkSize)
				for (size_t j = 0; j < ChunkSize; j++)
					if (Chunk[i][j].IsNull())
					{
						ChunkFill[i]++;
						return &Chunk[i][j];
					}
		}
		return nullptr;
	}

	void Delete(T *el)
	{
		assert(el->IsNull());
		size_t i;
		for (i = 0; i < MaxChunk; i++)
			if (ChunkFill[i])
				if (el > Chunk[i] && el < Chunk[i] + ChunkSize)
					break;
		if (i < MaxChunk)
		{
			if (--ChunkFill[i] == 0)
				delete[] Chunk[i];
			assert(ChunkFill[i] >= 0);
		}
	}

	template<typename Ti, typename SA = ChunkSA>
	class Iterator : public std::iterator<std::forward_iterator_tag, Ti>
	{
		SA *array;
		size_t i, j; // chunk, position in chunk

		Iterator& next(bool return_next)
		{
			for (; i < MaxChunk; i++, j = 0)
			{
				if (array->ChunkFill[i])
				{
					for (; j < ChunkSize; j++)
					{
						if (array->Chunk[i][j].IsNull()) continue;
						if (return_next)
							return *this;
						return_next = true;
					}
				}
			}
			// We're at the end.
			array = nullptr;
			i = j = 0;
		}
	public:
		Iterator(SA *array) : array(array), i(0), j(0)
		{
			if (array)
				next(true);
		}

		Iterator& operator++()
		{
			return next(false);
		}

		bool operator==(Iterator other) { return array == other.array && i == other.i && j == other.j; }
		bool operator!=(Iterator other) { return !(*this == other); }
		Ti* operator*() const { return &array->Chunk[i][j]; }
	};

	Iterator<T> begin() { return Iterator<T>(this); }
	Iterator<T> end() { return Iterator<T>(nullptr); }
	Iterator<const T, const ChunkSA> begin() const { return Iterator<const T, const ChunkSA>(this); }
	Iterator<const T, const ChunkSA> end() const { return Iterator<const T, const ChunkSA>(nullptr); }
};
