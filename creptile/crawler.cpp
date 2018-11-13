#include "crawler.h"
#include "http.h"
#include "bloom_filter.h"
#include "persistor.h"
#include "connection.h"
#include "connection_http.h"
#include "util.h"
#include "log.h"
#include "ev_mainloop.h"
#include <regex>

void crawler::waiting_list_append(const string & url) {
	if (!filter->contains(url)) {
		waiting.push_back(url);
		filter->add(url);
	}
}

void crawler::resolve_body(const string & url_from, const string & body) {
	if (body.empty()) {
		return;
	}
	std::smatch result;
	auto curIter = body.begin();
	auto endIter = body.end();
	while (std::regex_search(curIter, endIter, result, std::regex("href=\"(https?:)?//\\S+\""))) {
		auto new_url = std::regex_replace(result[0].str(), std::regex("href=\"(https?:)?//(\\S+)\""), "$2");
		waiting_list_append(new_url);
		persist->append(url_from, new_url);
		curIter = result[0].second;
	}
}

void crawler::mission_dispatch() {
	auto &url = waiting.front();
	
	count++;
	singleton<logger>::instance()->notice_fn(__FILE__, __LINE__, __func__, "New mission %dth.", count);
	conn->connect(url);
	singleton<logger>::instance()->debug_fn(__FILE__, __LINE__, __func__, "Waiting Urls: %d.", waiting.size());
	waiting.pop_front();
}

void crawler::connection_connected_cb(void * craw, void *_conn) {
	crawler *self = static_cast<crawler*>(craw);
	connection_http *conn = static_cast<connection_http*>(_conn);

	conn->send_req();
}

void crawler::connection_read_cb(void * craw, void * _conn) {
	crawler *self = static_cast<crawler*>(craw);
	connection_http *conn = static_cast<connection_http*>(_conn);

	http_parser *parser = conn->recv_rply();

	auto &status_line = parser->get_status_line();
	auto &body = parser->get_body();
	auto &url = conn->get_url();

	singleton<logger>::instance()->debug_fn(__FILE__, __LINE__, __func__,
		"Receive http reply, body length: %d, status: %s.", body.size(), status_line.substr(9, 3).c_str());

	self->resolve_body(url, body);
	self->persist->persist_body(url, body);
	self->conn->close();
}

void crawler::check(void * craw) {
	crawler *self = static_cast<crawler*>(craw);

	if (!self->conn->is_close())
		return;

	if (self->waiting.empty() || self->max_page <= self->count) {
		singleton<ev_mainloop>::instance()->mark_for_close();
		return;
	}

	self->mission_dispatch();
}

crawler::crawler() {
	filter = new bloom_filter(2000);
	persist = new persistor("result\\", "url_relations.txt", 200);
	conn = new connection_http(0);
	is_init = true;
	
	count = 0;
	max_page = 0;

	conn->set_cb(connection_read_cb, nullptr, nullptr, nullptr, connection_connected_cb, this);
}

crawler::crawler(int argc, char ** argv) {
	if (argc < 3) {
		singleton<logger>::instance()->warm_fn(__FILE__, __LINE__, __func__, "Usage: %s [url] [output file].");
		is_init = false;
		return;
	}

	waiting.emplace_back(argv[1]);
	crawler();
}

crawler::~crawler() {
	delete filter;
	delete persist;
}

void crawler::run(const string &entry, int count) {
	if (!is_init)
		return;

	singleton<logger>::instance()->add_file("debug.log", logger::log_level::DEBUG);
	singleton<logger>::instance()->add_file("info.log", logger::log_level::INFO);
	singleton<logger>::instance()->set_default_level(logger::log_level::DEBUG);
	singleton<logger>::instance()->notice_fn(__FILE__, __LINE__, __func__, "Start, entry is %s, count: %d.", entry.c_str(), count);
	waiting_list_append(entry);
	max_page = count;

	singleton<ev_mainloop>::instance()->set_period(check, this);
	singleton<ev_mainloop>::instance()->loop();

	singleton<logger>::instance()->notice_fn(__FILE__, __LINE__, __func__, "Mission complete.");
}
