#include "afx.h"
#include "crawler.h"

using namespace std;

void network_startup() {
	WSADATA wsadata;
	WSAStartup(MAKEWORD(2, 0), &wsadata);
}

inline void network_cleanup() {
	WSACleanup();
}

int main() {
	network_startup();

	crawler c;

	c.run("www.hao123.com", 20);

	network_cleanup();
	return 0;
}
