#pragma once

#include "afx.h"
#include "singleton.h"
struct event_base;

class ev_mainloop {
	struct event_base *evbase;
	bool done;

	ev_mainloop();
	
	static singleton<ev_mainloop> instance;
	friend class singleton<ev_mainloop>;

public:
	
	~ev_mainloop();

	struct event_base *get_base();

	void loop();

	inline void mark_for_close() {
		done = true;
	}
};

