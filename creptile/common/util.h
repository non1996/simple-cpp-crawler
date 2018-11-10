#pragma once

#include "afx.h"

namespace util {
	bool read_entire_file(const char *path, const char *mode, char **content, uint32_t *len);

	bool read_file(FILE *handle, char *buffer, size_t to_read, size_t *readed);

	bool file_size(const char *path, uint64_t *size);

	char *cstr_copy(const char *src);

	uint64_t rs_hash(const char *str, uint32_t length);

	uint64_t elf_hash(const char* str, uint32_t length);

	std::pair<std::string, std::string> string_split_pair(const std::string &str, char split);
	void string_strip(std::string &str, char ch);
	bool string_equal_nocase(const std::string &str1, const std::string &str2);
	size_t string_htod(const std::string &str, size_t start, size_t end);
	void string_to_upper(std::string &str);
	void string_to_lower(std::string &str);
}
