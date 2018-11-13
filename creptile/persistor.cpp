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
	for (auto iter = relations.begin(); iter != relations.end(); ++iter) {
		fout << index++ << " " << iter->url << endl;
		for (auto iter_re = iter->to.begin(); iter_re != iter->to.end(); ++iter_re) 
			fout << *iter_re << " ";
		
		fout << endl;
	}
	fout.close();
}

persistor::persistor(const string & _saving_path, const string & _urls_filename, size_t _threshold)
	:saving_path(_saving_path), urls_filename(_urls_filename){

}

persistor::~persistor() {
	persist_urls();
}

void persistor::append(const string & from, const string to) {
	auto res_from = url_to_id.find(from), res_to = url_to_id.find(to);
	size_t id_from, id_to;

	if (res_from == url_to_id.end()) {
		id_from = relations.size();
		url_to_id.emplace(from, id_from);
		res_from = url_to_id.find(from);
		relations.emplace_back(res_from->first);
	}
	
	if (res_to == url_to_id.end()) {
		id_to = relations.size();
		url_to_id.emplace(to, id_to);
		res_to = url_to_id.find(to);
		relations.emplace_back(res_to->first);
	}

	id_from = res_from->second;
	id_to = res_to->second;

	relations.at(id_from).to.push_back(id_to);
}

void persistor::persist_body(const string &url, const string & body) {
	auto res = url_to_id.find(url);
	
	assert(res != url_to_id.end());
	size_t id = res->second;

	static char buf[32];
	string filename = saving_path + _itoa(id, buf, 10) + ".txt";
	ofstream fout(filename);

	if (!fout.is_open()) {
		singleton<logger>::instance()->warm_fn(__FILE__, __LINE__, __func__, 
			"Could not open file %s to persist &s.", filename.c_str(), url.c_str());
		return;
	}

	fout.write(url.c_str(), url.size());
	fout.write("\n\n", 2);
	fout.write(body.c_str(), body.size());
	fout.close();
}
