#include "connection.h"
#include "ev_mainloop.h"
#include "log.h"
#include "evhead.h"

void connection::event_ev(bufferevent * bev, short events, void * conn) {
	connection *self = static_cast<connection*>(conn);
	if (events & BEV_EVENT_CONNECTED) {
		logger::info("Connection %d connected.", self->global_id);
		self->set_state(state::working);
		self->connected->emit(self->get_id());
	}
	if (events & BEV_EVENT_ERROR) {
		logger::info("Connection %d meet an error, %d[%s], it will be close.",
			self->global_id, EVUTIL_SOCKET_ERROR(), evutil_socket_error_to_string(EVUTIL_SOCKET_ERROR()));
		self->error->emit(self->get_id());
		self->close();
	}
	if (events & BEV_EVENT_EOF) {
		logger::info("Connection %d meet eof, it will be close.", self->global_id);
		self->closed->emit(self->get_id());
		self->close();
	}
}

void connection::read_ev(bufferevent * bev, void * conn) {
	connection *self = static_cast<connection*>(conn);
	logger::debug("Connection %d received something.", self->global_id);
	self->readable->emit(self->get_id(), self->inbuf_size());
}

void connection::write_ev(bufferevent * bev, void * conn) {
	(void)bev;
	connection *self = static_cast<connection*>(conn);
	logger::debug("Connection %d sent something.", self->global_id);
	self->writed->emit(self->get_id());
}

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
}

void connection::setup_bev(evutil_socket_t fd) {
	struct event_base *evbase = singleton<ev_mainloop>::instance()->get_base();

	bev = bufferevent_socket_new(evbase, fd, BEV_OPT_CLOSE_ON_FREE);

	bufferevent_setcb(bev, read_ev, write_ev, event_ev, this);
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

	remote_port = 0;
	local_port = 0;

	to_close = false;
}

connection::~connection() {
	close();
}

bool connection::connect(const std::string & ip, uint16_t port) {
	if (is_to_close())
		return false;

	if (s != state::closed)
		return true;

	struct sockaddr_in sin;
	sockaddr_construct(&sin, ip.c_str(), port);

	setup_bev(-1);

	if (bufferevent_socket_connect(bev, (struct sockaddr*)&sin, sizeof(sin)) < 0) {
		logger::info("Connection %lu connect to %s:%d failed.", global_id, ip.c_str(), port);
		return false;
	}

	set_state(state::connecting);
	remote_address = ip;
	remote_port = port;

	logger::info("Connection %lu launch connect to %s:%d successfully, wait for connected.", global_id, ip.c_str(), port);

	return true;
}

void connection::close() {
	if (s == state::closed)
		return;
	
	if (bev) {
		bufferevent_free(bev);
		bev = nullptr;
	}


	bytes_read = 0;
	bytes_write = 0;
	s = state::closed;
	remote_address.clear();
	remote_port = 0;
	local_address.clear();
	local_port = 0;
	to_close = false;
}

bool connection::is_close() {
	return s == state::closed;
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
	return evbuffer_get_length(bufferevent_get_input(bev));
}

size_t connection::outbuf_size() {
	return evbuffer_get_length(bufferevent_get_output(bev));
}
