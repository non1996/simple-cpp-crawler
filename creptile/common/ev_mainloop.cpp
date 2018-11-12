#include "ev_mainloop.h"
#include "log.h"
#include "evhead.h"

singleton<ev_mainloop> ev_mainloop::instance;

ev_mainloop::ev_mainloop() {
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

	}

	singleton<logger>::instance()->notice_fn(__FILE__, __LINE__, __func__, "Mainloop exit.");
}
