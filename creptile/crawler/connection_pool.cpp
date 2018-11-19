#include "connection_pool.h"
#include "connection_http.h"
#include "bitset.h"

void connection_pool::connection_close(size_t id) {
	ready.push_back(conns.at(id).get());
}

connection_pool::connection_pool(size_t count) {
	if (count == 0)
		throw std::logic_error("At least one connection");

	for (size_t id = 0; id < count; ++id) 
		conns.emplace_back(new connection_http(id));

	for (auto &iter : conns) {
		iter->http_come->connect(this->http_come);
		iter->error->connect(std::bind(&connection_pool::connection_close, this, _1));
		iter->closed->connect(std::bind(&connection_pool::connection_close, this, _1));
		ready.push_back(iter.get());
	}
}

connection_pool::~connection_pool() {
}

bool connection_pool::dispatch_mission(const string & url) {
	if (ready.empty())
		return false;
	
	auto conn = ready.front();

	conn->connect(url);
	ready.pop_front();

	return true;
}

bool connection_pool::has_ready() {
	return !ready.empty();
}

bool connection_pool::has_busy() {
	return ready.size() != conns.size();
}
