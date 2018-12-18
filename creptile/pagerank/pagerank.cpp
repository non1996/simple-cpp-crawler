#include "pagerank.h"
#include "sparse_matrix.h"
#include "log.h"
#include "util.h"
#include <algorithm>
#include <fstream>
#include <sstream>
using std::ifstream;
using std::ofstream;
using std::stringstream;
using std::endl;

bool pagerank::load_relation() {
	ifstream fin(infile);
	stringstream ss;

	logger::notice("Start Loading url relations from %s.", infile.c_str());

	if (!fin.is_open()) {
		logger::warm("Failed to open %s.", infile.c_str());
		return false;
	}

	while (!fin.eof()) {
		string url_line, relation_line;
		size_t idx_from, idx_to;
		pair<string, string> temp;

		std::getline(fin, url_line);

		if (url_line.empty())
			continue;

		temp = util::string_split_pair(url_line, ' ');
		idx_from = std::stoi(temp.first);

		std::getline(fin, relation_line);
		
		ss << relation_line;
		while (!ss.eof()) {
			ss >> idx_to;
			coo->insert(idx_to, idx_from, 1);
		}

		urls.emplace_back(temp.second);
		ss.clear();
	}

	coo->sort();

	logger::info("Url relations loaded.", infile.c_str());

	return true;
}

void pagerank::make_revise_matrix() {
	logger::notice("Make revice matrix.");

	ell.reset(coo->make_ell());
	ell->make_trans();
	ell->scalar_mult(1 - alpha);
}

void pagerank::caculte() {
	size_t urls_num = urls.size();									//	N
	my_vector e(urls_num, alpha / (float)urls_num);					//	(alpha / N) * Ve
	my_vector ranks(urls_num, 1 / (float)urls_num), res(ranks);		//	pageranks: Vr									
	size_t times = 0;

	logger::notice("Start caculating.");

	do {
		ranks = res;												
		res = ell->mult_vector(ranks);
		res.add(e);
		logger::info("%dth times.", times);
	} while (++times < times_threshold && !res.similar(ranks, accuracy_threshold));

	for (size_t index = 0; index < urls_num; ++index) {
		urls.at(index).r = res.at(index);
	}
}

bool pagerank::output() {
	std::sort(urls.begin(), urls.end(), [](const rank &r1, const rank &r2) { return r1.r > r2.r; });

	ofstream fout(outfile);

	logger::notice("Start output url relations to %s.", outfile.c_str());

	if (!fout.is_open()) {
		logger::warm("Failed to open %s.", outfile.c_str());
		return false;
	}

	size_t end = urls.size() > 10 ? 10 : urls.size();

	for (size_t index = 0; index < end; ++index) {
		fout << urls.at(index).url << " " << urls.at(index).r << endl;
	}

	fout.close();
	return true;
}

pagerank::pagerank(const string & _infile, const string & _outfile, float _alpha, float _accuracy_threshold, size_t times) 
	:coo(new coo_matrix), infile(_infile), outfile(_outfile), alpha(_alpha),
	 accuracy_threshold(_accuracy_threshold), times_threshold(times){
}

pagerank::~pagerank() {
}

void pagerank::run() {
	logger::add_file("pagerank_debug.log", logger::log_level::DEBUG);
	logger::set_default_level(logger::log_level::DEBUG);

	load_relation();
	make_revise_matrix();
	caculte();
	output();

	logger::notice("Quit.", infile.c_str());
}

pagerank::rank::rank(const string & _url)
	:url(_url), r(1) {
}

void pagerank_work(const char * input, const char * output) {
	pagerank pg(input, output);
	pg.run();
}
