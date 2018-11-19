#pragma once

#include "afx.h"

#include <event2/util.h>

struct bufferevent;

using std::string;

class connection {
public:
	const static uint8_t read = 0x02u;
	const static uint8_t write = 0x04u;

	enum class state {
		closed,
		connecting,
		working,
	};
	
	signal(connected, size_t);
	signal(writed, size_t);
	signal(readable, size_t, size_t);
	signal(closed, size_t);
	signal(error, size_t);

private:
	uint64_t bytes_read;
	uint64_t bytes_write;

	uint32_t global_id;

	struct bufferevent *bev;

	state s;

	string remote_address, local_address;
	uint16_t remote_port, local_port;

	bool to_close;

protected:
	void set_state(state s);

	inline struct bufferevent *get_bev() {
		return bev;
	}

	virtual void setup_bev(evutil_socket_t fd);

	static void sockaddr_construct(struct sockaddr_in *sin, const char *address, uint16_t port);

	static void event_ev(struct bufferevent *bev, short events, void *conn);
	static void read_ev(struct bufferevent *bev, void *conn);
	static void write_ev(struct bufferevent *bev, void *conn);

public:
	connection(uint32_t global_id);
	~connection();

	inline uint32_t get_id() {
		return global_id;
	}

	inline void mark_for_close() {
		to_close = true;
	}

	inline bool is_to_close() {
		return to_close;
	}

	inline const std::string &get_remote_address() {
		return remote_address;
	}

	inline const std::string &get_local_address() {
		return local_address;
	}

	inline uint16_t get_remote_port() {
		return remote_port;
	}

	inline uint16_t get_local_port() {
		return local_port;
	}

	inline state get_state() {
		return s;
	}

	bool connect(const string &ip, uint16_t port);
	virtual void close();
	bool is_close();

	bool recv(char *buf, size_t *len);

	bool send(const char *buf, size_t len);

	void enable(uint8_t mode);
	void disable(uint8_t mode);

	size_t inbuf_size();
	size_t outbuf_size();
};
