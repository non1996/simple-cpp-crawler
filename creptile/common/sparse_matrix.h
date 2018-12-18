#pragma once
#include "afx.h"
#include <vector>
#include <list>
#include <algorithm>
using std::vector;
using std::list;

class my_vector {
	vector<float> values;

public:
	my_vector(size_t size)
		: values(size, 0) {
	}

	my_vector(size_t size, float v)
		: values(size, v) {
	}

	inline bool similar(my_vector &other, float accuracy_threshold) {
		return std::equal(values.begin(), values.end(), other.values.begin(),
			[accuracy_threshold](float f1, float f2) { return abs(f1 - f2) < accuracy_threshold; });
	}

	inline const my_vector &add(my_vector &to_add) {
		std::transform(values.begin(), values.end(), to_add.values.begin(), values.begin(),
			[](float f1, float f2) { return f1 + f2; });
		return *this;
	}

	inline const my_vector &scalar_mult(float a) {
		for (auto &i : values)
			i *= a;
		return *this;
	}

	inline size_t size() const {
		return values.size();
	}

	inline float &at(size_t index) {
		return values.at(index);
	}

	inline const float &at(size_t index) const {
		return values.at(index);
	}
};

class coo_matrix;

class ell_matrix {
	struct ell_entry{
		int col;
		float val;
	
		ell_entry(int _col, float _val) 
			: col(_col), val(_val) {
		}
	};

	vector<vector<ell_entry>> matrix;
	vector<int> not_zero_nums;

	friend class coo_matrix;

	ell_matrix(size_t rows)
		:matrix(rows), not_zero_nums(rows) {

	}
public:
	inline const ell_matrix &scalar_mult(float a) {
		for (auto &line : matrix)
			for (auto &entry : line)
				entry.val *= a;
		return *this;
	}

	my_vector mult_vector(my_vector &v) {
		my_vector res(v);
		size_t row_num = 0;
		for (auto &row: matrix) {
			res.at(row_num) = 0;
			for (auto &entry: row)
				res.at(row_num) += entry.val * v.at(entry.col);
			++row_num;
		}
		return res;
	}

	void make_trans() {
		for (auto &line: matrix)
			for (auto &entry: line)
				entry.val /= not_zero_nums.at(entry.col);
	}
};

class coo_matrix {
	struct coo_entry {
		size_t row, col;
		float value;

		coo_entry(size_t _row, size_t _col, float _value)
			:row(_row), col(_col), value(_value) {
		}

		bool same_pos(const coo_entry &other) const {
			return col == other.col && row == other.row;
		}

		bool less_pos(const coo_entry &other) const {
			return row < other.row || (row == other.row && col < other.col);
		}
	};

private:
	vector<coo_entry> matrix;

public:
	coo_matrix() {
	}

	void insert(size_t row, size_t col, float value) {
		coo_entry e(row, col, value);

		matrix.emplace_back(row, col, value);

		//auto res = std::find_if_not(matrix.begin(), matrix.end(),
		//	[&e](const coo_entry&item) { return e.less_pos(item); });

	//	for (auto iter = matrix.begin(); iter != matrix.end(); ++iter) {
	//		if (e.less_pos(*iter)) {
	//			if (e.same_pos(*iter)) {
	//				iter->value = e.value;
	//				return;
	//			}

	//			matrix.insert(iter, e);
	//			return;
	//		}
	//	}
	//	matrix.push_back(e);
	}

	void sort() {
		std::sort(matrix.begin(), matrix.end(), [](const coo_entry &e1, const coo_entry &e2) { return e1.less_pos(e2); });
	}

	ell_matrix *make_ell() {
		if (matrix.empty())
			return nullptr;

		ell_matrix *ell = new ell_matrix(matrix.back().row + 1);

		for (auto &entry : matrix) {
			ell->not_zero_nums.at(entry.col)++;

			if (!ell->matrix.at(entry.row).empty() && ell->matrix.at(entry.row).back().col == entry.col)
				continue;

			ell->matrix.at(entry.row).emplace_back(entry.col, entry.value);
		}
		return ell;
	}
};
