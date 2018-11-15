#pragma once

#include "afx.h"
#include "bitset.h"
#include "util.h"
#include <memory>
using std::unique_ptr;

class bitset;

class bloom_filter {
	constexpr uint32_t bit(uint32_t num) { return num & mask; };
	
	size_t mask;
	unique_ptr<bitset> bs;

public:
	bloom_filter(size_t max_elem) {
		assert(max_elem);
		size_t nbits = 1u << ((size_t)floor(log2(max_elem)) + 3);
		mask = nbits - 1;
		bs = unique_ptr<bitset>(new bitset(nbits));
	}

	~bloom_filter() {
	}

	inline void add(const std::string &data) {
		const uint64_t hs1 = util::rs_hash(data.c_str(), data.size());
		const uint64_t hs2 = util::elf_hash(data.c_str(), data.size());
		const uint32_t idx1 = (uint32_t)hs1;
		const uint32_t idx2 = (uint32_t)((hs1 >> 16) + hs1);
		const uint32_t idx3 = (uint32_t)((hs1 >> 32) + hs1);
		const uint32_t idx4 = (uint32_t)((hs1 >> 48) + hs1);
		const uint32_t idx5 = (uint32_t)hs2;
		const uint32_t idx6 = (uint32_t)((hs2 >> 16) + hs2);
		const uint32_t idx7 = (uint32_t)((hs2 >> 32) + hs2);
		const uint32_t idx8 = (uint32_t)((hs2 >> 48) + hs2);

		bs->set(bit(idx1));
		bs->set(bit(idx2));
		bs->set(bit(idx3));
		bs->set(bit(idx4));
		bs->set(bit(idx5));
		bs->set(bit(idx6));
		bs->set(bit(idx7));
		bs->set(bit(idx8));
	}

	inline bool contains(const std::string &data) {
		const uint64_t hs1 = util::rs_hash(data.c_str(), data.size());
		const uint64_t hs2 = util::elf_hash(data.c_str(), data.size());
		const uint32_t idx1 = (uint32_t)hs1;
		const uint32_t idx2 = (uint32_t)((hs1 >> 16) + hs1);
		const uint32_t idx3 = (uint32_t)((hs1 >> 32) + hs1);
		const uint32_t idx4 = (uint32_t)((hs1 >> 48) + hs1);
		const uint32_t idx5 = (uint32_t)hs2;
		const uint32_t idx6 = (uint32_t)((hs2 >> 16) + hs2);
		const uint32_t idx7 = (uint32_t)((hs2 >> 32) + hs2);
		const uint32_t idx8 = (uint32_t)((hs2 >> 48) + hs2);

		return bs->is_set(bit(idx1)) && bs->is_set(bit(idx2)) &&
				bs->is_set(bit(idx3)) && bs->is_set(bit(idx4)) &&
				bs->is_set(bit(idx5)) && bs->is_set(bit(idx6)) &&
				bs->is_set(bit(idx7)) && bs->is_set(bit(idx8));
	}
};

