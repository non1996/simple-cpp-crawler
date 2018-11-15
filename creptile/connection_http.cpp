#include "connection_http.h"
#include "http.h"
#include "util.h"
#include "log.h"
#include "evhead.h"

inline string connection_http::dns_parse(const string & host) {
	hostent *hostt = gethostbyname(host.c_str());
	//auto res = getaddrinfo(host.c_str(), nullptr, &hints, );
	static char str[128];
	return hostt ? inet_ntop(hostt->h_addrtype, hostt->h_addr_list[0], str, sizeof(str)) : "";
}

void connection_http::read_ev(bufferevent * bev, void * _conn) {
	connection_http *self = static_cast<connection_http*>(_conn);
	size_t len;
	char *buf;

	len = self->inbuf_size();
	buf = new char[len];
	self->recv(buf, &len); 

	if (self->parser->parse(buf, len))
		self->http_come->emit(self->url, self->parser->get_body());

	delete[] buf;
}

void connection_http::event_ev(bufferevent * bev, short events, void * conn) {
	connection_http *self = static_cast<connection_http*>(conn);
	if (events & BEV_EVENT_CONNECTED) {
		logger::info("Connection %d connected.", self->get_id());
		self->set_state(state::working);
		self->send_req();
		return;
	}
	if (events & BEV_EVENT_EOF) {
		logger::info("Connection %d meet eof, it will be close.", self->get_id());
			
		if (self->parser->is_type_close())
			self->http_come->emit(self->url, self->parser->get_body());

		self->closed->emit(self->get_id());

		self->close();
		return;
	}

	connection::event_ev(bev, events, conn);
}

void connection_http::setup_bev(evutil_socket_t fd) {
	connection::setup_bev(fd);
	bufferevent_setcb(get_bev(), connection_http::read_ev, nullptr, connection_http::event_ev, this);
}

connection_http::connection_http(uint32_t global_id)
	: connection(global_id), parser(new http_parser){

}

connection_http::~connection_http() {
}

const string & connection_http::get_url() {
	return url;
}

bool connection_http::connect(const string & _url) {
	auto pair = util::string_split_pair(_url, '/');
	host = pair.first;
	resource = "/" + pair.second;
	url = _url;

	std::string ip = dns_parse(host);
	if (ip.empty())
		return false;

	parser->reset();

	return connection::connect(ip, 80);
}

void connection_http::close() {
	url.clear();
	host.clear();
	resource.clear();
	connection::close();
}

bool connection_http::send_req() {
	string req
		= "GET " + resource + " HTTP/1.1\r\n"
		+ "HOST: " + host + "\r\n"
		+ "Connection: close\r\n\r\n";

	logger::debug("Send http request, host: %s, get: %s.", 
		host.c_str(), resource.c_str());

	return connection::send(req.c_str(), req.size());
}

http_parser * connection_http::recv_rply() {
	return parser.get();
}
