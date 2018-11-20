#pragma once

#include "afx.h"
#include "my_signal.h"
using std::string;

class html_parser {
public:
	signal(to_fetch, const string &);
	signal(to_persist, const string &, const string &);

private:
	static const char *url_regex;

public:
	html_parser();
	~html_parser();

	void parse(const string &url, const string &body);
};

