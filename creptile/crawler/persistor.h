#pragma once

#include "afx.h"
#include <vector>
#include <string>
#include <map>
#include <set>
#include <ostream>
using std::vector;
using std::string;
using std::pair;
using std::map;
using std::set;

class persistor {
private:
	struct relation {
		size_t idx;
		string url;
		set<relation*> to;
		bool fetched;

		relation(const string &_url)
			:idx(0), url(_url), fetched(false) {
		}

		void push_back(relation *r) {
			if (to.find(r) == to.end())
				to.insert(r);
		}

		void output(std::ostream &os) {
			os << idx << " " << url << std::endl;
			for (auto r : to)
				if (r->fetched)
					os << r->idx << " ";
			os << std::endl;
		}
	};

	map<string, relation*> fetched;
	vector<shared_ptr<relation>> relations;
	
	string saving_path;
	string urls_filename;

	void persist_urls();

public:
	persistor(const string &_saving_path, const string &_urls_filename, size_t _treshold);
	~persistor();

	void append(const string &from, const string &url);
	//void persist_body(const string &url, const string &body);
};

