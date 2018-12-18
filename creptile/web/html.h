#pragma once

#include "afx.h"
#include "my_signal.h"
#include <fstream>

using std::string;

class html_parser {
public:
	signal(to_fetch, const string &);
	signal(to_persist, const string &, const string &);

private:
	static const char *url_regex;
	std::ofstream fout;

	inline bool ignore(const string &url);
	inline bool is_relative(const string &url);
	inline bool pretreat(const string &from, string &to);
	inline void pretreat_relative(const string &from, string &to);
	inline void pretreat_absolute(string &to);
	inline bool start_with_upper(const string &to, size_t pos);
	inline bool start_with_current(const string &to, size_t pos);

public:
	html_parser();
	~html_parser();

	void parse(const string &url, const string &body);
};

