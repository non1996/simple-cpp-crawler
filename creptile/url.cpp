#include "url.h"

void url_pair::to_absolute() {
}

bool url_pair::is_absolute() {


	return false;
}

url_pair::url_pair(const std::string &referrer, const std::string &self) {
	this->referrer = referrer;
	this->self = self;

	if (!is_absolute())
		to_absolute();
}

url_pair::url_pair(const std::string & index) {
	self = index;
}


url_pair::~url_pair() {
}
