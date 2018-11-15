#pragma once

#include "afx.h"
#include <list>
#include <memory>
using std::list;
using std::string;
using std::unique_ptr;

class persistor;
class bloom_filter;
class connection_pool;
class http_parser;

class crawler {
private:
	list<string> waiting;

	unique_ptr<bloom_filter> filter;
	unique_ptr<persistor> persist;
	unique_ptr<connection_pool> pool;

	size_t max_page, count;
	bool is_init;
	bool done;

	void waiting_list_append(const string &url);
	void resolve_body(const string &url_from, const string &body);

	

public:
	crawler();
	crawler(int argc, char **argv);
	~crawler();

	void resolve_html(const string &url, const string &body);
	void run(const string &entry, int count);
	void check();
};
