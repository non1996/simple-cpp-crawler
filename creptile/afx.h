#pragma once

#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <cstdbool>
#include <cassert>
#include <cstddef>
#include <string>
#include <iostream>

#if defined(__linux__)
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#elif defined(WIN32)
#include <WinSock2.h>
#pragma comment(lib, "ws2_32.lib")
#endif

#ifdef WIN32
#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#endif

#define LF (char)0x0a

typedef void(*callback_fn)(void*, void*);
#define callback(fn, arg1, arg2) do { if ((fn)) (fn)((arg1), (arg2)); } while(0)