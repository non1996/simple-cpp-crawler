#include "html.h"
#include "log.h"
#include <regex>
#include <iostream>

#define r_start "^"
#define r_end "$"

const char *html_parser::url_regex = r_start "<a\\s+.*href\\s*=\\s*\".+\".*>" r_end;

inline bool html_parser::ignore(const string & url) {
	return url.find("js") != string::npos || url.find("css") != string::npos;
}

inline bool html_parser::is_relative(const string & url) {
	return url.find("http") == string::npos && url.find("https") == string::npos;
}

inline bool html_parser::pretreat(const string & from, string &to) {
	if (to.find("#") != string::npos) {
		logger::debug("Inner page url, ignore this.");
		return false;
	}

	try {
		if (is_relative(to))
			pretreat_relative(from, to);
		else
			pretreat_absolute(to);
	}
	catch (std::logic_error &e) {
		(void)e;
		logger::warm("Invalid url %s, from %s, ignore.", to.c_str(), from.c_str());
		return false;
	}
	return true;
}

inline void html_parser::pretreat_relative(const string & from, string & to) {
	string from_cp = from;

	if (from_cp.find('/') == string::npos)
		from_cp += '/';

	size_t pos_from = from_cp.find_last_of('/'), pos_to = 0;

	if (to.front() == '/') {			//to is root
		pos_from = from_cp.find_first_of('/');
		to = from_cp.substr(0, pos_from) + to;
		return;
	}

	while (true) {
		if (start_with_upper(to, pos_to)) {		//upper
			if ((pos_from = from_cp.find_last_of("/", pos_from)) == string::npos)
				throw std::logic_error("");
			pos_from--;
			if ((pos_from = from_cp.find_last_of("/", pos_from)) == string::npos)
				throw std::logic_error("");
			pos_to += 3;
			continue;
		}
		if (start_with_current(to, pos_to)) {		//current
			pos_from = from_cp.find_last_of('/');
			pos_to += 2;
			continue;
		}
		break;
	}
	logger::debug("pos_from: %d, pos_to: %d", pos_from, pos_to);
	to = from_cp.substr(0, pos_from + 1) + to.substr(pos_to);
}

inline void html_parser::pretreat_absolute(string & to) {
	size_t pos = to.find("//");
	if (pos == string::npos)
		throw std::logic_error("");
	to = to.substr(pos + 2);
}

inline bool html_parser::start_with_upper(const string & to, size_t pos) {
	return to.size() - pos >= 3 && to.substr(pos, 3) == "../";
}

inline bool html_parser::start_with_current(const string & to, size_t pos) {
	return to.size() - pos >= 2 && to.substr(pos, 2) == "./";
}

html_parser::html_parser() {
	fout.open("all_parsed.txt");
}

html_parser::~html_parser() {
	fout.close();
}

void html_parser::parse(const string & url, const string & body) {
	logger::debug("Resolve html from url: %s", url.c_str());

	if (body.empty()) {
		return;
	}
	std::smatch result;
	auto curIter = body.begin();
	auto endIter = body.end();
	size_t count = 0, valid = 0;
	while (std::regex_search(curIter, endIter, result, std::regex("(href|src|action)=\"\\S+\"", std::regex::icase))) {
		auto new_url = std::regex_replace(result[0].str(), std::regex("(href|src|action)=\"(\\S+)\""), "$2");
		count++;
		curIter = result[0].second;

		if (ignore(new_url))
			continue;

		//logger::debug("Get new url: %s", new_url.c_str());
		if (!pretreat(url, new_url))
			continue;
			
		to_fetch->emit(new_url);
		to_persist->emit(url, new_url);

		logger::debug("Url from: %s, Url to: %s", url.c_str(), new_url.c_str());
	}
	logger::info("%d urls in html from %s", count, url.c_str());

	fout << url << " " << count << std::endl << std::endl;
	//fout << body << std::endl << "===========================================" << std::endl;
}
