#include "afx.h"
#include "crawler.h"
#include "pagerank.h"

using namespace std;

int main() {
#if defined(CRAWLER_MODE)
	crawler_work("news.sohu.com", "url_relations.txt");
#elif defined(PAGERANK_MODE)
	pagerank_work("result\\url_relations.txt", "result\\url-top10.txt");
#endif
	return 0;
}
