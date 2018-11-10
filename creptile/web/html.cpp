#include "html.h"
#include <regex>

#define r_start "^"
#define r_end "$"

const char *html::url_regex = r_start "<a\\s+.*href\\s*=\\s*\".+\".*>" r_end;

html::html() {
}


html::~html() {
}

bool html::parse_url(const std::string & body, std::vector<std::string> &result) {
	std::regex reg(url_regex);
	auto res_begin = std::sregex_iterator(body.begin(), body.end(), reg);
	auto res_end = std::sregex_iterator();

	if (res_begin == res_end)
		return false;

	for (auto iter = res_begin; iter != res_end; ++iter)
		result.emplace_back((*iter).str());

	return true;
}
