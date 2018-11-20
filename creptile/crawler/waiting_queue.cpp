#include "waiting_queue.h"
#include "bloom_filter.h"

waiting_queue::waiting_queue(bloom_filter * _filter)
	:filter(_filter) {
}

waiting_queue::~waiting_queue() {
}

void waiting_queue::append(const string & url) {
	if (!filter->contains(url)) {
		urls.push_back(url);
		filter->add(url);
	}
}
