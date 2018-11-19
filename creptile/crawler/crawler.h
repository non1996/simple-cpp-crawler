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

	size_t max_pages, count;
	bool is_init;
	bool done;

	void waiting_list_append(const string &url);
	void resolve_html(const string &url, const string &body);

public:
	crawler();
	crawler(int argc, char **argv);
	~crawler();

	
	void run(const string &entry);
	void check();
};
