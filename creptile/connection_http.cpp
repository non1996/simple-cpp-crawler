#include "connection_http.h"
#include "http.h"
#include "log.h"

#include <event2/event.h>
#include <event2/buffer.h>
#include <event2/bufferevent.h>

inline std::string connection_http::dns_parse(const std::string & url) {
	hostent *host = gethostbyname(url.c_str());
	return host ? inet_ntoa(*(in_addr *)host->h_addr_list[0]) : "";
}

void connection_http::read_evcb(bufferevent * bev, void * conn) {
	connection_http *conn_h = static_cast<connection_http*>(conn);
	connection *conn_r = dynamic_cast<connection*>(conn_h);
	size_t len;
	char *buf;

	buf = new char[conn_h->inbuf_size()];
	len = conn_h->inbuf_size();
	conn_h->recv(buf, &len); 
	conn_h->reply->append(buf, len);
	if (conn_h->reply->parse_response())
		assert(0);// entile reply tell others
	delete[] buf;
}

void connection_http::event_evcb(bufferevent * bev, short events, void * conn) {
	connection_http *conn_h = static_cast<connection_http*>(conn);
	if (events & BEV_EVENT_CONNECTED) {
		log_info("Connection %lu connected.", conn_h->get_id());
		conn_h->set_state(state::working);
	}
	if (events & BEV_EVENT_ERROR) {
		log_info("Connection %lu meet an error, %d[%s], it will be close.",
			conn_h->get_id(), EVUTIL_SOCKET_ERROR(), evutil_socket_error_to_string(EVUTIL_SOCKET_ERROR()));
		conn_h->close();
	}
}

void connection_http::setup_bev(evutil_socket_t fd) {
	connection::setup_bev(fd);
	bufferevent_setcb(get_bev(), read_evcb, nullptr, event_evcb, this);
}

connection_http::connection_http(uint32_t global_id):
	connection(global_id) {
	reply = new http;
}


connection_http::~connection_http() {
	delete reply;
}

bool connection_http::connect(const std::string & url) {
	std::string ip = dns_parse(url);

	if (ip.empty())
		return false;

	this->url = url;
	reply->clear();

	return connection::connect(ip, 80);
}

bool connection_http::send_req(const http & request) {
	std::string req = request.to_buf();
	return connection::send(req.c_str(), req.size());
}

bool connection_http::recv_rply(http &reply) {
	reply = *(this->reply);
	return true;
}
