#include "log.h"
#include "util.h"
#include <time.h>

#if defined(WIN32)
#define SLASH '\\'
#elif defined(__linux__)
#define SLASH '/'
#endif

static inline const char *month_to_string(int32_t month) {
	static const char *month_str[] = {
		"Jan", "Feb", "Mar", "Apr",
		"May", "Jun", "Jul", "Aug",
		"Sep", "Oct", "Nov", "Dec" };
	return month_str[month];
}

singleton<logger> logger::ginstance;

logger::logger() {
	logfiles.push_back(new logcmd(log_level::NOTICE));
}

const char * logger::level_to_str(log_level level) {
	assert(level <= log_level::SILENCE);
	switch (level) {
	case log_level::DEBUG:
		return "[debug]";
	case log_level::INFO:
		return "[info]";
	case log_level::NOTICE:
		return "[notice]";
	case log_level::WARM:
		return "[warm]";
	case log_level::ERR:
		return "[error]";
	default:
		return "???";
	}
}

const char *logger::get_time() {
	static char time_buf[128];
	time_t curr_time;
	struct tm *t;

	curr_time = time(nullptr);
	t = localtime(&curr_time);
	sprintf(time_buf, "%dth %s %d %d:%d:%d", t->tm_mday, month_to_string(t->tm_mon),
		t->tm_year + 1900, t->tm_hour, t->tm_min, t->tm_sec);
	return time_buf;
}

void logger::fn(log_level level, const char * file, uint32_t line, const char * func, const char * fomat, va_list args) {
	const char *time;
	const char *level_t;
	if (do_log) {
		time = get_time();
		level_t = level_to_str(level);

		for (auto l : logfiles)
			if (l->lowest_level <= level)
				l->log(time, level_t, file, line, func, fomat, args);
	}
}

logger::~logger() {
	for (auto l : logfiles)
		delete l;
}

bool logger::add_file(const char * filename, log_level level) {
	FILE *file;
	if ((file = fopen(filename, "w")) == nullptr) 
		return false;
	logfile *new_logger = new logfile(level, filename, file);
	logfiles.push_back(new_logger);
	return true;
}

void logger::set_default_level(log_level level) {
	logfiles.front()->lowest_level = level;
}

void logger::set_silence() {
	set_default_level(log_level::SILENCE);
}

void logger::enable() {
	do_log = true;
}

void logger::disable() {
	do_log = false;
}

void logger::error_fn(const char * file, uint32_t line, const char * func, const char * fomat, ...) {
	va_list args;
	va_start(args, fomat);
	fn(log_level::ERR, file, line, func, fomat, args);
	va_end(args);
}

void logger::warm_fn(const char * file, uint32_t line, const char * func, const char * fomat, ...) {
	va_list args;
	va_start(args, fomat);
	fn(log_level::WARM, file, line, func, fomat, args);
	va_end(args);
}

void logger::notice_fn(const char * file, uint32_t line, const char * func, const char * fomat, ...) {
	va_list args;
	va_start(args, fomat);
	fn(log_level::NOTICE, file, line, func, fomat, args);
	va_end(args);
}

void logger::info_fn(const char * file, uint32_t line, const char * func, const char * fomat, ...) {
	va_list args;
	va_start(args, fomat);
	fn(log_level::INFO, file, line, func, fomat, args);
	va_end(args);
}

void logger::debug_fn(const char * file, uint32_t line, const char * func, const char * fomat, ...) {
	va_list args;
	va_start(args, fomat);
	fn(log_level::DEBUG, file, line, func, fomat, args);
	va_end(args);
}

logger::logcmd::logcmd(log_level lowest) {
	lowest_level = lowest;
}

void logger::logcmd::log(const char *time, const char *level, const char * file, 
	uint32_t line, const char * func, const char * fomat, va_list args) {
	printf("%s, %s %s(): ", time, level, func);
	vprintf(fomat, args);
	printf("\tfile: %s, line: %d", strrchr(file, SLASH) ? strrchr(file, SLASH) + 1 : file, line);
	putchar('\n');
}

logger::logfile::logfile(log_level lowest, const char * filename, FILE * file)
	:logcmd(lowest){
	this->filename = util::cstr_copy(filename);
	this->file = file;
}

logger::logfile::~logfile() {
	delete[] filename;
	fclose(file);
}

void logger::logfile::log(const char *time, const char *level, const char * file,
	uint32_t line, const char * func, const char * fomat, va_list args) {
	fprintf(this->file, "%s, %s %s(): ", time, level, func);
	vfprintf(this->file, fomat, args);
	fprintf(this->file, "\tfile: %s, line: %d", strrchr(file, SLASH) ? strrchr(file, SLASH) + 1 : file, line);
	fprintf(this->file, "\n");
}
