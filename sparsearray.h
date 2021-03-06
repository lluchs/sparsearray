#pragma once
#include <cassert>
#include <bitset>

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
				mask[i] |= (uint64_t) 1 << j;
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
		mask[i] &= ~((uint64_t) 1 << j);
	}

	template<typename Ti, typename SA = BitmapSA>
	class Iterator : public std::iterator<std::forward_iterator_tag, Ti>
	{
		SA *array;
		size_t el, pos, start; // el: current element in data, pos: current bit position in cur
		uint64_t cur;
	public:
		Iterator(SA *array) : array(array), el(0), pos(64), cur(0)
		{
			start = 1;
			if (array)
				operator++();
			start = 0;
		}

		Iterator& operator++()
		{
			assert(array);
			while (!cur)
			{
				el += 64 - pos;
				if (el >= N) break;
				pos = 0;
				cur = array->mask[(el + 1) / 64];
			}
			size_t inc = __builtin_ffsll(cur);
			el += inc - start;
			pos += inc;
			cur >>= inc;
			if (el >= N)
			{
				array = nullptr;
				el = 0;
			}
			return *this;
		}

		bool operator==(Iterator other) { return array == other.array && el == other.el; }
		bool operator!=(Iterator other) { return !(*this == other); }
		Ti& operator*() const { assert(el < N); return array->data[el]; }
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
	std::bitset<N> UsedElements;

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
					if (!UsedElements[i*ChunkSize + j])
					{
						UsedElements.set(i*ChunkSize + j);
						ChunkFill[i]++;
						return &Chunk[i][j];
					}
		}
		return nullptr;
	}

	void Delete(T *el)
	{
		size_t i, j;
		for (i = 0; i < MaxChunk; i++)
			if (ChunkFill[i])
				if (el >= &Chunk[i][0] && el < &Chunk[i][ChunkSize])
					break;
		assert(i < MaxChunk);

		j = el - &Chunk[i][0];
		assert(UsedElements[i*ChunkSize + j]);
		UsedElements.reset(i*ChunkSize + j);

		if (--ChunkFill[i] == 0)
		{
			delete[] Chunk[i];
			Chunk[i] = nullptr;
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
						if (array->UsedElements[i*ChunkSize + j] && return_next)
							return *this;
						return_next = true;
					}
				}
				return_next = true;
			}
			// We're at the end.
			array = nullptr;
			i = j = 0;
			return *this;
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
		Ti& operator*() const { return array->Chunk[i][j]; }
	};

	Iterator<T> begin() { return Iterator<T>(this); }
	Iterator<T> end() { return Iterator<T>(nullptr); }
	Iterator<const T, const ChunkSA> begin() const { return Iterator<const T, const ChunkSA>(this); }
	Iterator<const T, const ChunkSA> end() const { return Iterator<const T, const ChunkSA>(nullptr); }
};

template<typename T, size_t N, size_t ChunkSize = 500>
class StaticChunkSA
{
	static_assert(N % ChunkSize == 0, "N must be a multiple of ChunkSize");
	static constexpr size_t MaxChunk = N / ChunkSize;
	T Chunk[MaxChunk][ChunkSize];
	size_t ChunkFill[MaxChunk];
	std::bitset<N> UsedElements;

public:

	StaticChunkSA()
	{
		for (size_t i = 0; i < MaxChunk; i++)
		{
			ChunkFill[i] = 0;
		}
	}

	T* New()
	{
		for (size_t i = 0; i < MaxChunk; i++)
		{
			// Check this chunk for space.
			if (ChunkFill[i] < ChunkSize)
				for (size_t j = 0; j < ChunkSize; j++)
					if (!UsedElements[i*ChunkSize + j])
					{
						UsedElements.set(i*ChunkSize + j);
						ChunkFill[i]++;
						return &Chunk[i][j];
					}
		}
		return nullptr;
	}

	void Delete(T *el)
	{
		size_t i, j;
		for (i = 0; i < MaxChunk; i++)
			if (ChunkFill[i])
				if (el >= &Chunk[i][0] && el < &Chunk[i][ChunkSize])
					break;
		assert(i < MaxChunk);

		j = el - &Chunk[i][0];
		assert(UsedElements[i*ChunkSize + j]);
		UsedElements.reset(i*ChunkSize + j);

		--ChunkFill[i];
	}

	template<typename Ti, typename SA = StaticChunkSA>
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
						if (array->UsedElements[i*ChunkSize + j] && return_next)
							return *this;
						return_next = true;
					}
				}
				return_next = true;
			}
			// We're at the end.
			array = nullptr;
			i = j = 0;
			return *this;
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
		Ti& operator*() const { return array->Chunk[i][j]; }
	};

	Iterator<T> begin() { return Iterator<T>(this); }
	Iterator<T> end() { return Iterator<T>(nullptr); }
	Iterator<const T, const StaticChunkSA> begin() const { return Iterator<const T, const StaticChunkSA>(this); }
	Iterator<const T, const StaticChunkSA> end() const { return Iterator<const T, const StaticChunkSA>(nullptr); }
};

template<typename T, size_t N>
class LinkedListSA
{
protected: // for tests
	struct ListElement
	{
		T data;
		ListElement *next;
		bool used = false;
	};
	// We need this to be able to cast a T* to a ListElement*.
	static_assert(std::is_standard_layout<ListElement>::value);

	ListElement array[N];
	ListElement *firstUsed, *firstFree;
	
	/*void PrintList(ListElement *start)
	{
		for (auto it = start; it; it = it->next)
			printf("%d -> ", it->data);
		printf("nullptr\n");
	}*/

public:
	LinkedListSA() : firstUsed(nullptr), firstFree(array)
	{
		// Everything starts in the free list.
		for (size_t i = 1; i < N; i++)
			array[i-1].next = &array[i];
		array[N-1].next = nullptr;
	}

	T* New()
	{
		if (!firstFree) return nullptr;
		ListElement *el = firstFree;
		firstFree = el->next;
		el->used = true;

		// The hard part is now to insert the element in the right place in the list. We could just
		// put it in the front, but this would destroy cache locality during iteration.
		if (firstUsed)
		{
			ListElement *prevEl = el;
			while (--prevEl >= array)
				if (prevEl->used)
				{
					el->next = prevEl->next;
					prevEl->next = el;
					goto done;
				}
			// We're at the front.
			el->next = firstUsed;
		}
		else
			el->next = nullptr;
		firstUsed = el;
done:
		return &el->data;
	}

	void Delete(T *dataEl)
	{
		ListElement *el = reinterpret_cast<ListElement*>(dataEl);
		assert(el >= &array[0] && el < &array[N]);
		assert(el->used);
		el->used = false;

		ListElement *next = el->next;
		bool lookingForUsed = false, lookingForFree = !!firstFree;
		if (el == firstUsed)
			firstUsed = next;
		else
			lookingForUsed = true;

		ListElement *prevEl = el;
		while (--prevEl >= array && (lookingForUsed || lookingForFree))
		{
			if (lookingForUsed && prevEl->used)
			{
				prevEl->next = next;
				lookingForUsed = false;
			}
			if (lookingForFree && !prevEl->used)
			{
				el->next = prevEl->next;
				prevEl->next = el;
				lookingForFree = false;
			}
		}

		if (!!firstFree == lookingForFree)
		{
			if (firstFree)
				el->next = firstFree;
			else
				el->next = nullptr;
			firstFree = el;
		}
	}

	template<typename Ti, typename SA = LinkedListSA>
	class Iterator : public std::iterator<std::forward_iterator_tag, Ti>
	{
		// We need to save the next element explicitly to allow deletion during iteration.
		ListElement *el, *next;
	public:
		Iterator(SA *array) : el(array ? array->firstUsed : nullptr), next(el ? el->next : nullptr) { }

		Iterator& operator++()
		{
			el = next;
			next = el ? el->next : nullptr;
			return *this;
		}

		bool operator==(Iterator other) { return el == other.el; }
		bool operator!=(Iterator other) { return !(*this == other); }
		Ti& operator*() const { return el->data; }
	};

	Iterator<T> begin() { return Iterator<T>(this); }
	Iterator<T> end() { return Iterator<T>(nullptr); }
	Iterator<const T, const LinkedListSA> begin() const { return Iterator<const T, const LinkedListSA>(this); }
	Iterator<const T, const LinkedListSA> end() const { return Iterator<const T, const LinkedListSA>(nullptr); }
};

template<typename T, size_t N>
class LinkedListBitmapSA
{
protected: // for tests
	struct ListElement
	{
		T data;
		ListElement *next;
	};
	// We need this to be able to cast a T* to a ListElement*.
	static_assert(std::is_standard_layout<ListElement>::value);

	ListElement array[N];
	ListElement *firstUsed, *firstFree;
	std::bitset<N> UsedElements;
	
	/*void PrintList(ListElement *start)
	{
		for (auto it = start; it; it = it->next)
			printf("%d -> ", it->data);
		printf("nullptr\n");
	}*/

public:
	LinkedListBitmapSA() : firstUsed(nullptr), firstFree(array)
	{
		// Everything starts in the free list.
		for (size_t i = 1; i < N; i++)
			array[i-1].next = &array[i];
		array[N-1].next = nullptr;
	}

	T* New()
	{
		if (!firstFree) return nullptr;
		ListElement *el = firstFree;
		firstFree = el->next;
		UsedElements.set(el - array);

		// The hard part is now to insert the element in the right place in the list. We could just
		// put it in the front, but this would destroy cache locality during iteration.
		if (firstUsed)
		{
			ListElement *prevEl = el;
			while (--prevEl >= array)
				if (UsedElements[prevEl - array])
				{
					el->next = prevEl->next;
					prevEl->next = el;
					goto done;
				}
			// We're at the front.
			el->next = firstUsed;
		}
		else
			el->next = nullptr;
		firstUsed = el;
done:
		return &el->data;
	}

	void Delete(T *dataEl)
	{
		ListElement *el = reinterpret_cast<ListElement*>(dataEl);
		assert(el >= &array[0] && el < &array[N]);
		assert(UsedElements[el - array]);
		UsedElements.reset(el - array);

		ListElement *next = el->next;
		bool lookingForUsed = false, lookingForFree = !!firstFree;
		if (el == firstUsed)
			firstUsed = next;
		else
			lookingForUsed = true;

		ListElement *prevEl = el;
		while (--prevEl >= array && (lookingForUsed || lookingForFree))
		{
			if (lookingForUsed && UsedElements[prevEl - array])
			{
				prevEl->next = next;
				lookingForUsed = false;
			}
			if (lookingForFree && !UsedElements[prevEl - array])
			{
				el->next = prevEl->next;
				prevEl->next = el;
				lookingForFree = false;
			}
		}

		if (!!firstFree == lookingForFree)
		{
			if (firstFree)
				el->next = firstFree;
			else
				el->next = nullptr;
			firstFree = el;
		}
	}

	template<typename Ti, typename SA = LinkedListBitmapSA>
	class Iterator : public std::iterator<std::forward_iterator_tag, Ti>
	{
		// We need to save the next element explicitly to allow deletion during iteration.
		ListElement *el, *next;
	public:
		Iterator(SA *array) : el(array ? array->firstUsed : nullptr), next(el ? el->next : nullptr) { }

		Iterator& operator++()
		{
			el = next;
			next = el ? el->next : nullptr;
			return *this;
		}

		bool operator==(Iterator other) { return el == other.el; }
		bool operator!=(Iterator other) { return !(*this == other); }
		Ti& operator*() const { return el->data; }
	};

	Iterator<T> begin() { return Iterator<T>(this); }
	Iterator<T> end() { return Iterator<T>(nullptr); }
	Iterator<const T, const LinkedListBitmapSA> begin() const { return Iterator<const T, const LinkedListBitmapSA>(this); }
	Iterator<const T, const LinkedListBitmapSA> end() const { return Iterator<const T, const LinkedListBitmapSA>(nullptr); }
};

template<typename T, size_t N>
class UnorderedLinkedListSA
{
protected: // for tests
	struct ListElement
	{
		T data;
		ListElement *next;
		bool used = false;
	};
	// We need this to be able to cast a T* to a ListElement*.
	static_assert(std::is_standard_layout<ListElement>::value);

	ListElement array[N];
	ListElement *firstUsed, *firstFree;
	
	/*void PrintList(ListElement *start)
	{
		for (auto it = start; it; it = it->next)
			printf("%d -> ", it->data);
		printf("nullptr\n");
	}*/

public:
	UnorderedLinkedListSA() : firstUsed(nullptr), firstFree(array)
	{
		// Everything starts in the free list.
		for (size_t i = 1; i < N; i++)
			array[i-1].next = &array[i];
		array[N-1].next = nullptr;
	}

	T* New()
	{
		if (!firstFree) return nullptr;
		ListElement *el = firstFree;
		firstFree = el->next;
		el->used = true;
		// Instead of keeping order, we just prepend the new element to the list.
		el->next = firstUsed;
		firstUsed = el;
		return &el->data;
	}

	void Delete(T *dataEl)
	{
		ListElement *el = reinterpret_cast<ListElement*>(dataEl);
		assert(el >= &array[0] && el < &array[N]);
		assert(el->used);
		el->used = false;

		ListElement *next = el->next;
		if (el == firstUsed)
			firstUsed = next;
		else
		{
			for (ListElement *prevEl = firstUsed; prevEl; prevEl = prevEl->next)
				if (prevEl->next == el)
				{
					prevEl->next = next;
					break;
				}
		}
		el->next = firstFree;
		firstFree = el;
	}

	template<typename Ti, typename SA = UnorderedLinkedListSA>
	class Iterator : public std::iterator<std::forward_iterator_tag, Ti>
	{
		// We need to save the next element explicitly to allow deletion during iteration.
		ListElement *el, *next;
	public:
		Iterator(SA *array) : el(array ? array->firstUsed : nullptr), next(el ? el->next : nullptr) { }

		Iterator& operator++()
		{
			el = next;
			next = el ? el->next : nullptr;
			return *this;
		}

		bool operator==(Iterator other) { return el == other.el; }
		bool operator!=(Iterator other) { return !(*this == other); }
		Ti& operator*() const { return el->data; }
	};

	Iterator<T> begin() { return Iterator<T>(this); }
	Iterator<T> end() { return Iterator<T>(nullptr); }
	Iterator<const T, const UnorderedLinkedListSA> begin() const { return Iterator<const T, const UnorderedLinkedListSA>(this); }
	Iterator<const T, const UnorderedLinkedListSA> end() const { return Iterator<const T, const UnorderedLinkedListSA>(nullptr); }
};

template<typename T, size_t N>
class DoubleLinkedListSA
{
protected: // for tests
	struct ListElement
	{
		T data;
		// prev is only valid while the element is used.
		ListElement *next, *prev;
		bool used = false;
	};
	// We need this to be able to cast a T* to a ListElement*.
	static_assert(std::is_standard_layout<ListElement>::value);

	ListElement array[N];
	ListElement *firstUsed, *firstFree;
	
	/*void PrintList(ListElement *start)
	{
		for (auto it = start; it; it = it->next)
			printf("%d -> ", it->data);
		printf("nullptr\n");
	}*/

public:
	DoubleLinkedListSA() : firstUsed(nullptr), firstFree(array)
	{
		// Everything starts in the free list.
		for (size_t i = 1; i < N; i++)
			array[i-1].next = &array[i];
		array[N-1].next = nullptr;
	}

	T* New()
	{
		if (!firstFree) return nullptr;
		ListElement *el = firstFree;
		firstFree = el->next;
		el->used = true;

		// The hard part is now to insert the element in the right place in the list. We could just
		// put it in the front, but this would destroy cache locality during iteration.
		if (firstUsed)
		{
			ListElement *prevEl = el;
			while (--prevEl >= array)
				if (prevEl->used)
				{
					el->next = prevEl->next;
					el->prev = prevEl;
					prevEl->next = el;
					if (el->next)
						el->next->prev = el;
					goto done;
				}
			// We're at the front.
			el->next = firstUsed;
			el->next->prev = el;
		}
		else
			el->next = nullptr;
		firstUsed = el;
		el->prev = nullptr;
done:
		return &el->data;
	}

	void Delete(T *dataEl)
	{
		ListElement *el = reinterpret_cast<ListElement*>(dataEl);
		assert(el >= &array[0] && el < &array[N]);
		assert(el->used);
		el->used = false;

		ListElement *next = el->next;
		if (el == firstUsed)
			firstUsed = next;
		if (next)
			next->prev = el->prev;
		if (el->prev)
			el->prev->next = next;

		if (firstFree)
		{
			ListElement *prevEl = el;
			while (--prevEl >= array)
			{
				if (!prevEl->used)
				{
					el->next = prevEl->next;
					prevEl->next = el;
					return;
				}
			}
			el->next = firstFree;
		}
		else
			el->next = nullptr;
		firstFree = el;
	}

	template<typename Ti, typename SA = DoubleLinkedListSA>
	class Iterator : public std::iterator<std::forward_iterator_tag, Ti>
	{
		// We need to save the next element explicitly to allow deletion during iteration.
		ListElement *el, *next;
	public:
		Iterator(SA *array) : el(array ? array->firstUsed : nullptr), next(el ? el->next : nullptr) { }

		Iterator& operator++()
		{
			el = next;
			next = el ? el->next : nullptr;
			return *this;
		}

		bool operator==(Iterator other) { return el == other.el; }
		bool operator!=(Iterator other) { return !(*this == other); }
		Ti& operator*() const { return el->data; }
	};

	Iterator<T> begin() { return Iterator<T>(this); }
	Iterator<T> end() { return Iterator<T>(nullptr); }
	Iterator<const T, const DoubleLinkedListSA> begin() const { return Iterator<const T, const DoubleLinkedListSA>(this); }
	Iterator<const T, const DoubleLinkedListSA> end() const { return Iterator<const T, const DoubleLinkedListSA>(nullptr); }
};

template<typename T, size_t N>
class ReorderingSA
{
	T data[N];
	T *firstFree = data;

public:
	T* New()
	{
		if (firstFree < data + N)
			return firstFree++;
		return nullptr;
	}

	void Delete(T *el)
	{
		assert(el >= data && el < firstFree);
		auto lastUsed = firstFree - 1;
		if (el != lastUsed)
			*el = std::move(*lastUsed);
		firstFree--;
	}

	template<typename Ti, typename SA = ReorderingSA>
	class Iterator : public std::iterator<std::forward_iterator_tag, Ti>
	{
		SA *array;
		Ti *el, *firstFree;
	public:
		Iterator(SA *array) : array(array), el(array ? array->data : nullptr), firstFree(array ? array->firstFree : nullptr)
		{
		}

		Iterator& operator++()
		{
			// When el is deleted, the next element is the same element.
			if (firstFree != array->firstFree)
				firstFree = array->firstFree;
			else
				el++;
			if (el >= firstFree)
				el = nullptr;
			return *this;
		}

		bool operator==(Iterator other) { return el == other.el; }
		bool operator!=(Iterator other) { return !(*this == other); }
		Ti& operator*() const { return *el; }
	};

	Iterator<T> begin() { return Iterator<T>(this); }
	Iterator<T> end() { return Iterator<T>(nullptr); }
	Iterator<const T, const ReorderingSA> begin() const { return Iterator<const T, const ReorderingSA>(this); }
	Iterator<const T, const ReorderingSA> end() const { return Iterator<const T, const ReorderingSA>(nullptr); }
};
