#pragma once

#include "afx.h"
#include <list>
#include <memory>
using std::list;
using std::string;
using std::unique_ptr;

class waiting_queue;
class persistor;
class connection_pool;
class http_parser;
class html_parser;

class crawler {
private:
	unique_ptr<waiting_queue> waiting;
	unique_ptr<persistor> persist;
	unique_ptr<connection_pool> pool;
	unique_ptr<html_parser> parser;

	string entry;

	size_t max_pages, count;
	bool is_init;
	bool done;

public:
	crawler(const string &entry, const string &output);
	~crawler();

	void run();
	void check();
};

void crawler_work(const string &url, const string &output);
void crawler_work(const string &ip, uint16_t port, const string &output);
