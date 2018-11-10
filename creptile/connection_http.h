#pragma once

#include "connection.h"

class http;

class connection_http :
	public connection {
private:
	std::string url;
	http *reply;

	static inline std::string dns_parse(const std::string &url);

	static void read_evcb(struct bufferevent *bev, void *conn);
	static void event_evcb(struct bufferevent *bev, short events, void *conn);

protected:
	virtual void setup_bev(evutil_socket_t fd);

public:
	connection_http(uint32_t global_id);
	~connection_http();

	bool connect(const std::string &url);
	bool send_req(const http &request);
	bool recv_rply(http &reply);
};

