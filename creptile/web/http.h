#pragma once

#include "afx.h"
#include "singleton.h"

#include <vector>
using std::string;
using std::vector;
using std::pair;

class http_parser {
	enum class trans_type {
		chunk,
		fixed,
		close
	};
	class state {
	public:
		virtual bool parse(http_parser *hp);
	};

	class state_line: public state {
	public:
		virtual bool parse(http_parser *hp);
	};

	class state_headers : public state {
	public:
		virtual bool parse(http_parser *hp);
	};

	class state_body_chunk: public state {
	public:
		virtual bool parse(http_parser *hp);
	};

	class state_body_fixed : public state {
	public:
		virtual bool parse(http_parser *hp);
	};

	class state_body_close : public state {
	public:
		virtual bool parse(http_parser *hp);
	};

private:
	static singleton<state> s_complete;
	static singleton<state_line> s_line;
	static singleton<state_headers> s_headers;
	static singleton<state_body_chunk> s_body_chunk;
	static singleton<state_body_close> s_body_close;
	static singleton<state_body_fixed> s_body_fixed;

	string line;
	vector<pair<string, string>> headers;
	string body;
	string buf;
	size_t offset;
	state *s;

	size_t content_length;
	size_t next_chunk_size;
	trans_type type;

	void set_state(state *s) {
		this->s = s;
	}

public:
	http_parser();
	~http_parser();

	//	parse
	const string &get_body();
	bool parse(const char *_buf, size_t len);
};

