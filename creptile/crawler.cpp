#include "crawler.h"
#include "http.h"
#include "bloom_filter.h"
#include "persistor.h"
#include "connection.h"
#include "util.h"
#include "log.h"
#include "ev_mainloop.h"
#include <regex>

string crawler::get_ip_by_host(const string & host) {
	hostent* phost = gethostbyname(host.c_str());
	return phost ? inet_ntoa(*(in_addr *)phost->h_addr_list[0]) : "";
}

bool crawler::connection_connect_host(connection *conn, const string & host) {
	auto ip = get_ip_by_host(host);
	if (ip.empty()) {
		singleton<logger>::instance()->warm_fn(__FILE__, __LINE__, __func__, "Could not resolve dns %s.", host.c_str());
		return false;
	}

	return conn->connect(ip, 80);
}

bool crawler::connection_send_request(connection *conn, const string & host, const string & get) {
	string http
		= "GET " + get + " HTTP/1.1\r\n"
		+ "HOST: " + host + "\r\n"
		+ "Connection: close\r\n\r\n";

	singleton<logger>::instance()->debug_fn(__FILE__, __LINE__, __func__, 
		"Send http request, host: %s, get: %s.", host.c_str(), get.c_str());

	return conn->send(http.c_str(), http.size());
}

void crawler::waiting_list_append(const string & url) {
	if (!filter->contains(url)) {
		waiting.push_back(url);
		filter->add(url);
	}
}

void crawler::resolve_url(const string & url_from, const string & body) {
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
	curr_url = waiting.front();
	auto pair = util::string_split_pair(curr_url, '/');

	count++;
	singleton<logger>::instance()->notice_fn(__FILE__, __LINE__, __func__, "New mission %dth.", count);
	connection_connect_host(conn, pair.first);
	singleton<logger>::instance()->debug_fn(__FILE__, __LINE__, __func__, "Waiting Urls: %d.", waiting.size());
}

void crawler::connection_connected_cb(void * craw, void *_conn) {
	crawler *self = static_cast<crawler*>(craw);
	connection *conn = static_cast<connection*>(_conn);

	auto pair = util::string_split_pair(self->curr_url, '/');
	connection_send_request(conn, pair.first, "/" + pair.second);
}

void crawler::connection_close_cb(void * craw, void *_conn) {
	crawler *self = static_cast<crawler*>(craw);
	connection *conn = static_cast<connection*>(_conn);

	if (self->parser->is_type_close()) {
		auto &status_line = self->parser->get_status_line();
		auto &body = self->parser->get_body();
		auto &url = self->waiting.front();

		singleton<logger>::instance()->debug_fn(__FILE__, __LINE__, __func__, 
			"Receive http reply, body length: %d, status: %s.", body.size(), status_line.substr(9, 3).c_str());

		self->resolve_url(url, body);
		self->parser->reset();
	}
	self->waiting.pop_front();
}

void crawler::connection_read_cb(void * craw, void * _conn) {
	crawler *self = static_cast<crawler*>(craw);
	connection *conn = static_cast<connection*>(_conn);

	size_t size;
	char *buf;

	size = self->conn->inbuf_size();
	buf = new char[size];

	self->conn->recv(buf, &size);

	if (self->parser->parse(buf, size)) {
		auto &status_line = self->parser->get_status_line();
		auto &body = self->parser->get_body();
		auto &url = self->waiting.front();

		singleton<logger>::instance()->debug_fn(__FILE__, __LINE__, __func__,
			"Receive http reply, body length: %d, status: %s.", body.size(), status_line.substr(9, 3).c_str());

		self->resolve_url(url, body);
		self->waiting.pop_front();
		self->conn->close();
		self->parser->reset();
	}

	delete [] buf;
}

void crawler::check(void * craw) {
	crawler *self = static_cast<crawler*>(craw);

	if (!self->conn->is_close())
		return;

	if (self->waiting.size() == 0 || self->max_page <= self->count) {
		singleton<ev_mainloop>::instance()->mark_for_close();
		return;
	}

	self->mission_dispatch();
}

crawler::crawler() {
	filter = new bloom_filter(2000);
	persist = new persistor("result\\", "url_relations.txt", 200);
	conn = new connection(0);
	parser = new http_parser;
	is_init = true;
	
	count = 0;
	max_page = 0;

	conn->set_cb(connection_read_cb, nullptr, nullptr, connection_close_cb, connection_connected_cb, this);
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
