#include "crawler.h"
#include "url.h"
#include "util.h"
#include "log.h"

crawler::crawler(int argc, char ** argv) {
	if (argc < 3) {
		log_warm("Usage: %s [url] [output file].");
		is_init = false;
		return;
	}

	url_pair *index = new url_pair(argv[1]);

	waiting.push_back(index);

	c_pool = new connection_pool(100);
	output_file = util::cstr_copy(argv[2]);
	is_init = true;
}

crawler::~crawler() {
}

void crawler::work() {
	if (!is_init)
		return;

	while (!done) {
		c_pool->dispatch(waiting);
		ml->loop();
		check();
	}

	log_notice("Work done.");
}
