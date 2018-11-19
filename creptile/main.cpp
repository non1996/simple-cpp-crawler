#include "afx.h"
#include "crawler.h"
#include "pagerank.h"

using namespace std;

int main() {
	/*crawler c;

	c.run("www.hao123.com");*/

	pagerank_work("result\\url-relations.txt", "result\\url-top10.txt");

	return 0;
}
