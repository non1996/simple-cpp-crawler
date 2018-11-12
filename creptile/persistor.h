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
	vector<pair<string, string>> urls;

	size_t threshold;
	size_t count;
	
	string saving_path;
	string urls_filename;
	bool should_persist_html;

	void persist_urls();
	

public:
	persistor(const string &_saving_path, const string &_urls_filename, size_t _treshold);
	~persistor();

	void append(const string &from, const string url);
	void persist_body(const string &url, const string &body);
};

