#pragma once

#include "afx.h"
#include <list>

class url_pair;
class connection_crawler;
class connection_pool;
class mainloop;

class crawler {
private:
	std::list<url_pair*> waiting;
	std::list<url_pair*> fetched;
	mainloop *ml;
	connection_pool *c_pool;
	char *output_file;
	bool is_init;
	bool done;

public:
	crawler(int argc, char **argv);
	~crawler();

	void work();
	void check();
};

