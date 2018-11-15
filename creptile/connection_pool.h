#pragma once

#include "afx.h"
#include "my_signal.h"
#include <list>
#include <vector>
#include <memory>
using std::list;
using std::vector;
using std::shared_ptr;
using std::string;

class connection_http;
class bitset;

class connection_pool {
public:
	shared_ptr<signal<const string&, const string&>> http_come = 
		make_shared<signal<const string&, const string&>>();

private:
	vector<shared_ptr<connection_http>> conns;
	list<connection_http*> ready;

	void connection_close(size_t id);

public:
	connection_pool(size_t count);
	~connection_pool();

	bool dispatch_mission(const string &url);
	bool has_ready();
	bool has_busy();
};

