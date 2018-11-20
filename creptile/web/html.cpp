#include "html.h"
#include "log.h"
#include <regex>

#define r_start "^"
#define r_end "$"

const char *html_parser::url_regex = r_start "<a\\s+.*href\\s*=\\s*\".+\".*>" r_end;

html_parser::html_parser() {
}


html_parser::~html_parser() {
}

void html_parser::parse(const string & url, const string & body) {
	logger::debug("Resolve html from url: %s", url.c_str());

	if (body.empty()) {
		return;
	}
	std::smatch result;
	auto curIter = body.begin();
	auto endIter = body.end();
	while (std::regex_search(curIter, endIter, result, std::regex("href=\"(https?:)?//\\S+\""))) {
		auto new_url = std::regex_replace(result[0].str(), std::regex("href=\"(https?:)?//(\\S+)\""), "$2");
		to_fetch->emit(new_url);
		to_persist->emit(url, new_url);
		curIter = result[0].second;
	}
}
