#include "util.h"
#include "log.h"

bool util::read_entire_file(const char *path, const char *mode, char **content, uint32_t *len) {
	FILE *file;
	char *cont = nullptr;
	size_t recv;

	if (!(file = fopen(path, mode))) {
		return false;
	}

	fseek(file, 0, SEEK_END);
	recv = ftell(file);
	rewind(file);

	cont = new char[recv + 1];
	recv = fread(cont, sizeof(char), recv, file);

	if (recv == 0) {
		delete [] cont;
		fclose(file);
		return false;
	}

	*content = cont;
	*len = recv;
	fclose(file);
	return true;
}

bool util::read_file(FILE *handle, char *buffer, size_t to_read, size_t *recv) {
	if (!handle)
		return false;

	*recv = fread(buffer, sizeof(char), to_read, handle);

	return true;
}

bool util::file_size(const char * path, uint64_t * size) {
	FILE *file;

	if (!(file = fopen(path, "rb"))) {
		return false;
	}

	fseek(file, 0, SEEK_END);
	*size = ftell(file);
	fclose(file);
	return true;
}

char * util::cstr_copy(const char * src) {
	if (!src)
		return nullptr;

	char *temp = new char[strlen(src) + 1];
	strcpy(temp, src);
	return temp;
}

uint64_t util::rs_hash(const char * str, uint32_t length) {
	uint64_t b = 378551;
	uint64_t a = 63689;
	uint64_t hash = 0;

	for (uint32_t i = 0; i < length; ++str, ++i) {
		hash = hash * a + (*str);
		a = a * b;
	}

	return hash;
}

uint64_t util::elf_hash(const char * str, uint32_t length) {
	uint64_t hash = 0;
	uint64_t x = 0;

	for (uint32_t i = 0; i < length; ++str, ++i) {
		hash = (hash << 4) + (*str);

		if ((x = hash & 0xF0000000L) != 0) {
			hash ^= (x >> 24);
		}
		hash &= ~x;
	}

	return hash;
}

std::pair<std::string, std::string> util::string_split_pair(const std::string & str, char split) {
	std::pair<std::string, std::string> pr(str, "");
	size_t index;
	if ((index = str.find(split)) != std::string::npos) {
		pr.first = str.substr(0, index);
		pr.second = str.substr(index + 1);
	}
	return pr;
}

void util::string_strip(std::string & str, char ch) {
	if (str.size() == 0)
		return;
	size_t start = str.find_first_not_of(ch);
	size_t end = str.find_last_not_of(ch);

	start = (start == std::string::npos) ? 0 : start;
	end = (end == std::string::npos) ? str.size() - 1 : end;

	if (start == 0 && end == str.size() - 1)
		return;

	str = str.substr(start, end - start + 1);
}

namespace {
	bool char_euqal_nocase(char ch1, char ch2) {
		return	(ch1 == ch2) ||
				(ch1 + 32 == ch2) ||
				(ch1 == ch2 + 32);
	}
}

bool util::string_equal_nocase(const std::string & str1, const std::string & str2) {
	if (str1.size() != str2.size())
		return false;

	for (size_t i = 0; i < str1.size(); ++i) {
		if (!char_euqal_nocase(str1.at(i), str2.at(i)))
			return false;
	}

	return true;
}

size_t util::string_htod(const std::string & str, size_t start, size_t end) {
	size_t count = end - start;
	size_t res = 0, base = 1;

	for (int index = (int)end; index >= (int)start; --index) {
		if (str.at(index) <= '9' && str.at(index) >= '0')
			res += (str.at(index) - '0') * base;
		else
			res += (str.at(index) - 'a' + 10) * base;
		base *= 16;
	}

	return res;
}

void util::string_to_upper(std::string & str) {
	for (auto &ch : str) {
		if (ch > 'z' || ch < 'a')
			continue;
		ch -= 32;
	}
}

void util::string_to_lower(std::string & str) {
	for (auto &ch : str) {
		if (ch > 'Z' || ch < 'A')
			continue;
		ch += 32;
	}
}
