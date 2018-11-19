#pragma once

#include "afx.h"
#include <memory>
#include <vector>
using std::unique_ptr;
using std::string;
using std::pair;
using std::vector;

class coo_matrix;
class ell_matrix;

class pagerank {
	struct rank {
		string url;
		float r;

		rank(const string &_url);
	};

	string infile, outfile;

	unique_ptr<coo_matrix> coo;
	unique_ptr<ell_matrix> ell;
	vector<rank> urls;

	float alpha, accuracy_threshold;
	size_t times_threshold;

	bool load_relation();
	void make_revise_matrix();
	void caculte();
	bool output();

public:
	pagerank(const string &infile, const string &outfile, float alpha = 0.15f, 
		float accuracy_threshold = 0.0001f, size_t times = 50u);
	~pagerank();

	void run();
};

void pagerank_work(const char *input, const char *output);