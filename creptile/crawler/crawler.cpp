#include "crawler.h"
#include "http.h"
#include "html.h"
#include "persistor.h"
#include "connection_pool.h"
#include "bloom_filter.h"
#include "waiting_queue.h"
#include "util.h"
#include "log.h"
#include "ev_mainloop.h"

#include <regex>

void crawler::check() {
	if ((waiting->empty() || max_pages <= count) && !pool->has_busy()) {
		singleton<ev_mainloop>::instance()->mark_for_close();
		logger::notice("No more missions, mark for close.");
		return;
	}

	while (max_pages > count && pool->has_ready() && !waiting->empty()) {
		count++;
		logger::notice("New mission %dth, Waiting Urls: %d.", count, waiting->size() - 1);
		pool->dispatch_mission(waiting->front());
		waiting->pop();
	}
}

crawler::crawler(const string & _entry, const string & output)
	:waiting(new waiting_queue(new bloom_filter(200000))),
	persist(new persistor("result\\", output, 200)),
	pool(new connection_pool(5)),
	parser(new html_parser),
	entry(_entry),
	count(0),
	max_pages(200000){

	pool->http_come->connect(std::bind(&html_parser::parse, parser.get(), _1, _2));
	parser->to_fetch->connect(std::bind(&waiting_queue::append, waiting.get(), _1));
	parser->to_persist->connect(std::bind(&persistor::append, persist.get(), _1, _2));
	singleton<ev_mainloop>::instance()->period->connect(std::bind(&crawler::check, this));

	is_init = true;
}

crawler::~crawler() {

}

void crawler::run() {
	if (!is_init)
		return;

	logger::add_file("debug.log", logger::log_level::DEBUG);
	logger::add_file("info.log", logger::log_level::INFO);
	logger::set_default_level(logger::log_level::INFO);

	waiting->append(entry);
	logger::notice("Start, entry is %s, max pages: %d.", entry.c_str(), max_pages);
	singleton<ev_mainloop>::instance()->loop();
	logger::notice("Mission complete.");
}

void crawler_work(const string & url, const string & output) {
	crawler c(url, output);
	c.run();
}

void crawler_work(const string & ip, uint16_t port, const string & output) {
	char buf[16];
	crawler_work(ip + ":" + itoa(port, buf, 10), output);
}
