#pragma once

#include "afx.h"
#include <vector>
#include <string>
#include <map>
using std::vector;
using std::string;
using std::pair;
using std::map;

class persistor {
private:
	struct relation{
		const string &url;
		vector<size_t> to;

		relation(const string &_url)
			:url(_url) {

		}
	};

	map<string, size_t> url_to_id;
	vector<relation> relations;
	
	string saving_path;
	string urls_filename;

	void persist_urls();

public:
	persistor(const string &_saving_path, const string &_urls_filename, size_t _treshold);
	~persistor();

	void append(const string &from, const string url);
	void persist_body(const string &url, const string &body);
};

