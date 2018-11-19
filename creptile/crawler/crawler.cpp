#include "crawler.h"
#include "http.h"
#include "bloom_filter.h"
#include "persistor.h"
#include "connection_pool.h"
#include "connection_http.h"
#include "util.h"
#include "log.h"
#include "ev_mainloop.h"
#include <regex>

void crawler::waiting_list_append(const string & url) {
	if (!filter->contains(url)) {
		waiting.push_back(url);
		filter->add(url);
	}
}

void crawler::resolve_html(const string &url, const string &body) {
	logger::debug("Resolve html from url: %s", url.c_str());

	if (body.empty()) {
		return;
	}
	std::smatch result;
	auto curIter = body.begin();
	auto endIter = body.end();
	while (std::regex_search(curIter, endIter, result, std::regex("href=\"(https?:)?//\\S+\""))) {
		auto new_url = std::regex_replace(result[0].str(), std::regex("href=\"(https?:)?//(\\S+)\""), "$2");
		waiting_list_append(new_url);
		persist->append(url, new_url);
		curIter = result[0].second;
	}

	persist->persist_body(url, body);
}

void crawler::check() {
	if ((waiting.empty() || max_pages <= count) && !pool->has_busy()) {
		singleton<ev_mainloop>::instance()->mark_for_close();
		logger::notice("No more missions, mark for close.");
		return;
	}

	while (max_pages > count && pool->has_ready() && !waiting.empty()) {
		count++;
		logger::notice("New mission %dth, Waiting Urls: %d.", count, waiting.size() - 1);
		pool->dispatch_mission(waiting.front());
		waiting.pop_front();
	}
}

crawler::crawler()
	:filter(new bloom_filter(2000)), 
	persist(new persistor("result\\", "url_relations.txt", 200)),
	pool(new connection_pool(5)){

	pool->http_come->connect(std::bind(&crawler::resolve_html, this, _1, _2));

	is_init = true;
	count = 0;
	max_pages = 20;
}

crawler::crawler(int argc, char ** argv) {
	if (argc < 3) {
		logger::warm("Usage: %s [url] [output file].", argv[0]);
		is_init = false;
		return;
	}

	waiting.emplace_back(argv[1]);
	crawler();
}

crawler::~crawler() {

}

void crawler::run(const string &entry) {
	if (!is_init)
		return;

	logger::add_file("debug.log", logger::log_level::DEBUG);
	logger::add_file("info.log", logger::log_level::INFO);
	logger::set_default_level(logger::log_level::DEBUG);
	
	waiting_list_append(entry);

	singleton<ev_mainloop>::instance()->period->connect(std::bind(&crawler::check, this));

	logger::notice("Start, entry is %s, max pages: %d.", entry.c_str(), max_pages);

	singleton<ev_mainloop>::instance()->loop();

	logger::notice("Mission complete.");
}
