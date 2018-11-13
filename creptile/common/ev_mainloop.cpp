#include "ev_mainloop.h"
#include "log.h"
#include "evhead.h"

singleton<ev_mainloop> ev_mainloop::instance;

namespace {
	inline void network_startup() {
		WSADATA wsadata;
		WSAStartup(MAKEWORD(2, 0), &wsadata);
	}

	inline void network_cleanup() {
		WSACleanup();
	}
}




ev_mainloop::ev_mainloop() {
	network_startup();
	atexit(network_cleanup);
	evbase = event_base_new();
	done = false;
}

ev_mainloop::~ev_mainloop() {
}

event_base * ev_mainloop::get_base() {
	return evbase;
}

void ev_mainloop::loop() {
	int32_t res;

	singleton<logger>::instance()->notice_fn(__FILE__, __LINE__, __func__, "Mainloop start.");

	while (!done) {
		res = event_base_loop(evbase, EVLOOP_ONCE | EVLOOP_NONBLOCK);
		if (res < 0) {
			mark_for_close();
			continue;
		}
		period_check(arg);
	}

	singleton<logger>::instance()->notice_fn(__FILE__, __LINE__, __func__, "Mainloop exit.");
}

void ev_mainloop::set_period(void(*_period_check)(void *), void *_arg) {
	period_check = _period_check;
	arg = _arg;
}
