#pragma once

#include "afx.h"
#include <vector>
#include <string>
using std::vector;
using std::string;
using std::pair;

class persistor {
private:
	vector<pair<string, string>> urls;
	size_t threshold;
	size_t count;
	
	string saving_path;
	string urls_filename;
	bool should_persist_html;

	void persist_urls();
	void persist_html(const string &body);

public:
	persistor(const string &_saving_path, const string &_urls_filename, size_t _treshold);
	~persistor();

	void append(const pair<string, string> &url_pair, const string &body);
};

