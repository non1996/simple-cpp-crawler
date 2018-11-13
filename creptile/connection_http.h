#pragma once

#include "connection.h"

class http_parser;

class connection_http :
	public connection {
private:
	string url;
	string host, resource;
	http_parser *parser;

	static inline string dns_parse(const string &url);

	static void read_ev(struct bufferevent *bev, void *conn);
	static void event_ev(struct bufferevent *bev, short events, void *conn);

protected:
	virtual void setup_bev(evutil_socket_t fd);

public:
	connection_http(uint32_t global_id);
	~connection_http();

	const string &get_url();
	bool connect(const string &url);
	virtual void close();
	bool send_req();
	http_parser *recv_rply();
};

