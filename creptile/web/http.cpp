#include "http.h"
#include "util.h"

singleton<http_parser::state> http_parser::s_complete;
singleton<http_parser::state_line> http_parser::s_line;
singleton<http_parser::state_headers> http_parser::s_headers;
singleton<http_parser::state_body_chunk> http_parser::s_body_chunk;
singleton<http_parser::state_body_close> http_parser::s_body_close;
singleton<http_parser::state_body_fixed> http_parser::s_body_fixed;

namespace {
	const char *transfer_coding_str = "transfer-encoding";
	const char *content_length_str = "content-length";
};

http_parser::http_parser() {
	content_length = 0;
	next_chunk_size = 0;
	type = trans_type::close;
	s = singleton<state_line>::instance();
	offset = 0;
}

http_parser::~http_parser() {
}

void http_parser::reset() {
	line.clear();
	headers.clear();
	body.clear();
	buf.clear();
	s = singleton<state_line>::instance();
	content_length = 0;
	next_chunk_size = 0;
	type = trans_type::close;
	offset = 0;
}

bool http_parser::is_type_close() {
	return type == trans_type::close;
}

const string & http_parser::get_status_line() {
	return line;
}

const vector<pair<string, string>>& http_parser::get_headers() {
	return headers;
}

const string & http_parser::get_body() {
	return body;
}

bool http_parser::parse(const char * _buf, size_t len) {
	buf.append(_buf, len);

	while (s->parse(this));

	if (s == singleton<state>::instance())
		return true;
	return false;
}

bool http_parser::state_line::parse(http_parser *hp) {
	size_t end;
	if ((end = hp->buf.find("\r\n", hp->offset)) != string::npos) {
		hp->line = hp->buf.substr(hp->offset, end);
		hp->offset = end + 2;
		hp->set_state(singleton<state_headers>::instance());
		return true;
	}
	return false;
}

bool http_parser::state_headers::parse(http_parser * hp) {
	size_t end;
	while ((end = hp->buf.find("\r\n", hp->offset)) != string::npos) {
		if (end == hp->offset) {
			if (hp->type == trans_type::chunk)
				hp->set_state(singleton<state_body_chunk>::instance());
			else if (hp->type == trans_type::fixed)
				hp->set_state(singleton<state_body_fixed>::instance());
			else
				hp->set_state(singleton<state_body_close>::instance());
			hp->offset += 2;
			return true;
		}
			
		string header = hp->buf.substr(hp->offset, end - hp->offset);
		pair<string, string> header_p = util::string_split_pair(header, ':');

		util::string_strip(header_p.first, ' ');
		util::string_strip(header_p.second, ' ');

		hp->headers.push_back(header_p);

		if (util::string_equal_nocase(header_p.first, transfer_coding_str))
			hp->type = trans_type::chunk;
		if (util::string_equal_nocase(header_p.first, content_length_str)) {
			hp->type = trans_type::close;
			hp->content_length = std::stoi(header_p.second);
		}

		hp->offset = end + 2;
	}
	return false;
}

bool http_parser::state_body_chunk::parse(http_parser * hp) {
	size_t end;
	while (true) {
		if (hp->next_chunk_size == 0) {
			if ((end = hp->buf.find("\r\n", hp->offset)) == string::npos)
				return false;

			hp->next_chunk_size = util::string_htod(hp->buf, hp->offset, end - 1);
			hp->offset = end + 2;

			if (hp->next_chunk_size == 0) {
				hp->set_state(singleton<state>::instance());
				return true;
			}
		}
		if (hp->buf.size() - hp->offset >= hp->next_chunk_size + 2) {
			hp->content_length += hp->next_chunk_size;
			hp->body.append(hp->buf.substr(hp->offset, hp->next_chunk_size));
			hp->offset += hp->next_chunk_size + 2;
			hp->next_chunk_size = 0;
		}
		else
			return false;
	}
	return false;
}

bool http_parser::state::parse(http_parser * hp) {
	return false;
}

bool http_parser::state_body_fixed::parse(http_parser * hp) {
	if (hp->buf.size() - hp->offset < hp->content_length)
		return false;
	
	hp->body.append(hp->buf.substr(hp->offset, hp->content_length));
	hp->set_state(singleton<state>::instance());

	return true;
}

bool http_parser::state_body_close::parse(http_parser * hp) {
	if (hp->buf.size() > hp->offset)
		hp->body.append(hp->buf.c_str() + hp->offset, hp->buf.size() - hp->offset);
	hp->offset = hp->buf.size();

	return false;
}
