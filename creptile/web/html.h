#pragma once

#include <vector>
#include <string>

class html {
private:
	static const char *url_regex;
	html();

public:
	
	~html();

	static bool parse_url(const std::string &body, std::vector<std::string> &res);
};

