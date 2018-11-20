#pragma once

#include "afx.h"
#include <memory>
#include <list>
using std::list;
using std::string;
using std::unique_ptr;

class bloom_filter;

class waiting_queue {
	list<string> urls;
	unique_ptr<bloom_filter> filter;

public:
	waiting_queue(bloom_filter *_filter);
	~waiting_queue();

	const string &front() {
		return urls.front();
	}

	void pop() {
		urls.pop_front();
	}

	inline size_t size() {
		return urls.size();
	}

	inline bool empty() {
		return urls.empty();
	}

	void append(const string &url);
};

