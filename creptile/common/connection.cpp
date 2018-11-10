#include "connection.h"
#include "log.h"

#include <event2/event.h>
#include <event2/buffer.h>
#include <event2/bufferevent.h>

void connection::set_state(state s) {
	switch (s) {
	case state::connecting:
		enable(read | write);
		break;
	case state::working:
		enable(read | write);
		break;
	case state::closed:
		disable(read | write);
		break;
	default:
		assert(0);
	}

	this->s = s;
	//if (linked_connection && is_master) {
	//	linked_connection->set_state(s);
	//}
}

void connection::setup_bev(evutil_socket_t fd) {
	assert(0);
	struct event_base *evbase;

	bev = bufferevent_socket_new(evbase, fd, BEV_OPT_CLOSE_ON_FREE);
}

void connection::sockaddr_construct(sockaddr_in * sin, const char * address, uint16_t port) {
	sin->sin_family = AF_INET;
#ifdef WIN32
	sin->sin_addr.S_un.S_addr = inet_addr(address);
#endif
#ifdef __linux__
	sin->sin_addr.s_addr = inet_addr(address);
#endif
	sin->sin_port = htons(port);
}

connection::connection(uint32_t global_id) {
	bytes_read = 0;
	bytes_write = 0;

	this->global_id = global_id;

	bev = nullptr;
	s = state::closed;

	//linked_connection = nullptr;

	remote_port = 0;
	local_port = 0;

	//is_master = true;
	to_close = false;
}

connection::~connection() {
	if (bev)
		bufferevent_free(bev);
}

bool connection::connect(const std::string & ip, uint16_t port) {
	if (is_to_close())
		return false;

	if (/*linked_connection || */s != state::closed)
		return true;

	struct sockaddr_in sin;
	sockaddr_construct(&sin, ip.c_str(), port);

	setup_bev(-1);

	if (bufferevent_socket_connect(bev, (struct sockaddr*)&sin, sizeof(sin)) < 0) {
		log_info("Connection %lu connect to %s:%d failed.", global_id, ip, port);
		return false;
	}

	set_state(state::connecting);
	remote_address = ip;
	remote_port = port;

	log_info("Connection %lu launch connect to %s:%d successfully, wait for connected.", global_id, ip, port);

	return true;
}

void connection::close() {
	if (s == state::closed)
		return;

	//if (linked_connection && is_master)
	//	linked_connection->close();
	
	bufferevent_free(bev);

	bytes_read = 0;
	bytes_write = 0;
	s = state::closed;
	remote_address.clear();
	remote_port = 0;
	local_address.clear();
	local_port = 0;
	to_close = false;
}

bool connection::recv(char * buf, size_t * len) {
	int32_t rlen;

	rlen = bufferevent_read(bev, buf, *len);
	*len = rlen;
	bytes_read += *len;
	return true;
}

bool connection::send(const char * buf, size_t len) {
	if (len == 0)
		return true;

	if (bufferevent_write(bev, buf, len) < 0) {
		set_state(state::closed);
		return false;
	}
	bytes_write += len;
	return true;
}

void connection::enable(uint8_t mode) {
	bufferevent_enable(bev, mode);
}

void connection::disable(uint8_t mode) {
	bufferevent_disable(bev, mode);
}

size_t connection::inbuf_size() {
	return size_t();
}

size_t connection::outbuf_size() {
	return size_t();
}
