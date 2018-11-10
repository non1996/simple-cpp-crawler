#include "persistor.h"
#include <fstream>

using std::ofstream;

void persistor::persist_urls() {
	ofstream fout(saving_path + urls_filename, std::ios_base::app);
	size_t index = count - urls.size();
	for (auto iter = urls.begin(); iter != urls.end(); ++iter) {
		fout << index++ << " " << iter->first << " " << iter->second << std::endl;
	}
}

void persistor::persist_html(const string & body) {
	static char buf[32];
	if (should_persist_html && body.size()) {
		ofstream fout(saving_path + itoa(count, buf, 10), std::ios_base::app);

		if (!fout.is_open()) {
			return;
		}
		fout.write(body.c_str(), body.size());
		fout.close();
	}
}

persistor::persistor(const string & _saving_path, const string & _urls_filename, size_t _threshold)
	:saving_path(_saving_path), urls_filename(_urls_filename), threshold(_threshold){
	urls.reserve(threshold);
	count = 0;
}

persistor::~persistor() {
}

void persistor::append(const pair<string, string>& url_pair, const string & body) {
	urls.push_back(url_pair);
	count++;

	if (urls.size() == threshold) {
		persist_urls();
		urls.clear();
	}

	persist_html(body);
}
