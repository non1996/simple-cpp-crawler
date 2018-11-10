#include <iostream>
#include <vector>
#include <list>
#include <map>
#include <queue>
#include <string>
#include <utility>
#include <regex>
#include <fstream>
#include <WinSock2.h>
#include <Windows.h>
#include "http.h"
#include "log.h"
#include "bloom_filter.h"

#pragma comment(lib, "ws2_32.lib")

bloom_filter *filter;

using namespace std;

void startupWSA() {
	WSADATA wsadata;
	WSAStartup(MAKEWORD(2, 0), &wsadata);
}

inline void cleanupWSA() {
	WSACleanup();
}

inline pair<string, string> binaryString(const string &str, const string &dilme) {
	pair<string, string> result(str, "");
	auto pos = str.find(dilme);
	if (pos != string::npos) {
		result.first = str.substr(0, pos);
		result.second = str.substr(pos + dilme.size());
	}
	return result;
}

//	域名解析
inline string getIpByHostName(const string &hostName) {
	hostent* phost = gethostbyname(hostName.c_str());
	return phost ? inet_ntoa(*(in_addr *)phost->h_addr_list[0]) : "";
}

//	建立连接
inline SOCKET connect(const string &hostName) {
	auto ip = getIpByHostName(hostName);
	if (ip.empty())
		return 0;
	auto sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == INVALID_SOCKET)
		return 0;
	SOCKADDR_IN addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(80);
	addr.sin_addr.s_addr = inet_addr(ip.c_str());
	if (connect(sock, (const sockaddr *)&addr, sizeof(SOCKADDR_IN)) == SOCKET_ERROR)
		return 0;
	return sock;
}

//	构造并发送http请求
//	sock: socket文件描述符
//	host: 指定请求的主机IP和端口/域名
//	get: 请求的url
inline bool sendRequest(SOCKET sock, const string &host, const string &get) {
	string http
		= "GET " + get + " HTTP/1.1\r\n"
		+ "HOST: " + host + "\r\n"
		+ "Connection: close\r\n\r\n";

	singleton<logger>::instance()->debug_fn(__FILE__, __LINE__, __func__, "Send http request, host: %s, get: %s.", host.c_str(), get.c_str());

	return http.size() == send(sock, &http[0], http.size(), 0);
}

//	接收http请求
inline string recvReply(SOCKET sock) {
	static timeval wait = { 2, 0 };
	static auto buffer = string(2048 * 1000, '\0');
	auto recvlen = 0;
	http_parser parser;
	bool complete;

	do {
		fd_set fd = { 0 };
		FD_SET(sock, &fd);
		recvlen = 0;
		if (select(0, &fd, nullptr, nullptr, &wait) > 0) {
			recvlen = recv(sock, &buffer[0], 2048 * 1000, 0);
			if (recvlen > 0)
				complete = parser.parse(buffer.c_str(), recvlen);
			if (recvlen == 0)
				complete = true;
		}
		FD_ZERO(&fd);
	} while (!complete);

	singleton<logger>::instance()->debug_fn(__FILE__, __LINE__, __func__, "Receive http reply, length: %d, status: %s.", buffer.size(), buffer.substr(9, 3).c_str());

	return parser.get_body();
}

static inline void waiting_list_append(queue<string> &urls, const string &url) {
	if (!filter->contains(url)) {
		urls.push(url);
		filter->add(url);
	}
}

//	分析html，找到所有url，加入url队列
inline void extUrl(queue<string> &urlQueue, const string &buffer) {
	if (buffer.empty()) {
		return;
	}
	smatch result;
	string url;
	auto curIter = buffer.begin();
	auto endIter = buffer.end();
	while (regex_search(curIter, endIter, result, regex("href=\"(https?:)?//\\S+\""))) {
		waiting_list_append(urlQueue, regex_replace(result[0].str(), regex("href=\"(https?:)?//(\\S+)\""), "$2"));
		curIter = result[0].second;
	}
}

static inline void persistence(const std::string &url, const std::string &buf, size_t count) {
	static string re("result\\"), suffix(".txt");
	char a[32];
	std::ofstream fout(re + itoa(count, a, 10) + suffix, ios_base::binary);

	if (!fout.is_open())
		singleton<logger>::instance()->warm_fn(__FILE__, __LINE__, __func__, "Could not open file.");
	else {
		fout.write(url.c_str(), url.size());
		fout.write("\n\n\n", 3);
		fout.write(buf.c_str(), buf.size());
	}
	fout.close();
}

void Go(const string &entry, int count) {
	singleton<logger>::instance()->notice_fn(__FILE__, __LINE__, __func__, "Start, entry is %s, count: %d.", entry.c_str(), count);
	
	filter = new bloom_filter(200);

	queue<string> urls;

	waiting_list_append(urls, entry);

	for (auto i = 1; i <= count; ++i) {
		if (!urls.empty()) {
			singleton<logger>::instance()->notice_fn(__FILE__, __LINE__, __func__, "The %dth.", i);

			auto &url = urls.front();
			auto pair = binaryString(url, "/");
			auto sock = connect(pair.first);
			if (sock && sendRequest(sock, pair.first, "/" + pair.second)) {
				auto buffer = move(recvReply(sock));
				persistence(url, buffer, i);
				extUrl(urls, buffer);
			}
			closesocket(sock);
			urls.pop();
			singleton<logger>::instance()->debug_fn(__FILE__, __LINE__, __func__, "Waiting Urls.", urls.size());
		}
	}
}

int main() {
	startupWSA();
	singleton<logger>::instance()->set_default_level(logger::log_level::DEBUG);
	singleton<logger>::instance()->add_file("debug.log", logger::log_level::DEBUG);
	Go("www.hao123.com", 20);
	cleanupWSA();
	return 0;
}
