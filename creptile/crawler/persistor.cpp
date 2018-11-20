#include "persistor.h"
#include "log.h"
#include <fstream>

using std::ofstream;
using std::endl;

void persistor::persist_urls() {
	if (relations.empty())
		return;

	ofstream fout(saving_path + urls_filename);

	size_t index = 0;
	for (auto &r : relations) {
		if (r->fetched)
			r->idx = index++;
	}

	for (auto &r : relations)
		if (r->fetched)
			r->output(fout);

	fout.close();
}

persistor::persistor(const string & _saving_path, const string & _urls_filename, size_t _threshold)
	:saving_path(_saving_path), urls_filename(_urls_filename){

}

persistor::~persistor() {
	persist_urls();
}

void persistor::append(const string & from, const string &to) {
	auto res_from = fetched.find(from), res_to = fetched.find(to);
	relation *r_from, *r_to;

	if (res_from == fetched.end()) {
		r_from = new relation(from);
		relations.emplace_back(r_from);
		fetched.emplace(from, r_from);
	}
	else
		r_from = res_from->second;
	
	r_from->fetched = true;

	if (res_to == fetched.end()) {
		r_to = new relation(to);
		relations.emplace_back(r_to);
		fetched.emplace(to, r_to);
	}
	else
		r_to = res_to->second;

	r_from->push_back(r_to);
}

//void persistor::persist_body(const string &url, const string & body) {
//	auto res = url_to_id.find(url);
//	
//	assert(res != url_to_id.end());
//	size_t id = res->second;
//
//	static char buf[32];
//	string filename = saving_path + _itoa(id, buf, 10) + ".txt";
//	ofstream fout(filename);
//
//	if (!fout.is_open()) {
//		logger::warm("Could not open file %s to persist &s.", filename.c_str(), url.c_str());
//		return;
//	}
//
//	fout.write(url.c_str(), url.size());
//	fout.write("\n\n", 2);
//	fout.write(body.c_str(), body.size());
//	fout.close();
//}
