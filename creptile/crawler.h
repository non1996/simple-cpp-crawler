#pragma once

#include "afx.h"
#include <list>
using std::list;
using std::string;

class persistor;
class bloom_filter;
class connection_http;
class http_parser;

class crawler {
private:
	list<string> waiting;

	bloom_filter *filter;
	persistor *persist;

	connection_http *conn;

	size_t max_page, count;

	bool is_init;
	bool done;

	void waiting_list_append(const string &url);
	void resolve_body(const string &url_from, const string &body);
	void mission_dispatch();

	static void connection_connected_cb(void *craw, void *conn);
	static void connection_read_cb(void *craw, void *conn);
	static void check(void *craw);

public:
	crawler();
	crawler(int argc, char **argv);
	~crawler();

	void run(const string &entry, int count);
};
