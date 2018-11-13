#pragma once

#include "afx.h"
#include <list>
using std::list;
using std::string;

class persistor;
class bloom_filter;
class connection;
class http_parser;

class crawler {
private:
	list<string> waiting;

	bloom_filter *filter;
	persistor *persist;

	http_parser *parser;
	connection *conn;
	string curr_url;

	size_t max_page, count;

	bool is_init;
	bool done;

	void waiting_list_append(const string &url);
	void resolve_url(const string &url_from, const string &body);
	void mission_dispatch();

	static string get_ip_by_host(const string &host);
	static bool connection_connect_host(connection *conn, const string &host);
	static bool connection_send_request(connection *conn, const string &host, const string &get);
	static void connection_connected_cb(void *craw, void *conn);
	static void connection_close_cb(void *craw, void *conn);
	static void connection_read_cb(void *craw, void *conn);
	
	static void check(void *craw);

public:
	crawler();
	crawler(int argc, char **argv);
	~crawler();

	void run(const string &entry, int count);
};
