#pragma once

#include "afx.h"

class url_pair {
private:
	std::string referrer;
	std::string self;
	
	void to_absolute();
	bool is_absolute();

public:
	url_pair(const std::string &referrer, const std::string &self);
	url_pair(const std::string &index);
	~url_pair();

	inline const std::string &get_referrer() {
		return referrer;
	}

	const std::string &get_self() {
		return self;
	}
};

