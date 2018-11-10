#pragma once

#include "afx.h"

class bitset {
	const static size_t shift = 5;
	const static size_t word_bits = 32;
	const static size_t mask = ((1u << shift) - 1);
	constexpr static size_t store_size(size_t nbit) { return (nbit + mask) >> shift; };
	constexpr static size_t index(size_t bit) { return bit >> shift; };
	constexpr static size_t inner(size_t bit) { return bit & mask; };
	constexpr static size_t bit_mask(size_t bit) { return 1u << inner(bit); };

	uint32_t * arr;
	size_t nbit;

public:
	bitset(size_t count) {
		arr = mem_alloc_zero<uint32_t>(store_size(count));
		nbit = count;
	}

	~bitset() {
		mem_free(arr);
	}

	inline size_t size(bitset *self) {
		return self->nbit;
	}

	//inline void check(bitset *self, size_t expect) {
	//	if (self->nbit >= expect)
	//		return;

	//	while (self->nbit < expect)
	//		self->nbit <<= 1;

	//	mem_realloc(uint32_t, self->arr, store_size(self->nbit));
	//}

	inline void set(size_t bit) {
		//bitset_check(self, bit);
		if (bit >= nbit)
			return;

		arr[index(bit)] |= bit_mask(bit);
	}

	inline void clear(size_t bit) {
		if (bit >= nbit)
			return;

		arr[index(bit)] &= ~bit_mask(bit);
	}

	inline bool is_set(size_t bit) {
		if (bit >= nbit)
			return false;

		return arr[index(bit)] & bit_mask(bit);
	}

	inline void clear_all() {
		memset(arr, 0, 4 * store_size(nbit));
	}
};
