#ifndef LOG_H
#define LOG_H

#include "afx.h"
#include "singleton.h"
#include <list>

class logger {
public:
	enum log_level {
		DEBUG,
		INFO,
		NOTICE,
		WARM,
		ERR,
		SILENCE
	};

private:
	struct logcmd {
		log_level lowest_level;

		logcmd(log_level lowest);

		virtual void log(const char *time, const char *level, const char * file,
			uint32_t line, const char * func, const char * fomat, va_list args);
	};

	struct logfile : public logcmd{
		char *filename;
		FILE *file;

		logfile(log_level lowest, const char *filename, FILE *file);
		~logfile();

		virtual void log(const char *time, const char *level, const char * file,
			uint32_t line, const char * func, const char * fomat, va_list args);
	};

	static singleton<logger> ginstance;
	friend class singleton<logger>;

	std::list<logcmd*> logfiles;
	bool do_log;
	log_level level;

	logger();
	
	void fn(log_level level, const char *file, uint32_t line, const char *func, const char *fomat, va_list args);

	inline static const char *level_to_str(log_level level);
	inline static const char *get_time();

public:
	~logger();

	static bool add_file(const char *filename, log_level level);

	static void set_default_level(log_level level);

	static void set_silence();

	static void enable();

	static void disable();

	static void error_fn(const char *file, uint32_t line, const char *func, const char *fomat, ...);

	static void warm_fn(const char *file, uint32_t line, const char *func, const char *fomat, ...);

	static void notice_fn(const char *file, uint32_t line, const char *func, const char *fomat, ...);

	static void info_fn(const char *file, uint32_t line, const char *func, const char *fomat, ...);

	static void debug_fn(const char *file, uint32_t line, const char *func, const char *fomat, ...);
};

#define error(format, ...) error_fn(__FILE__, __LINE__, __func__, format, ##__VA_ARGS__)
#define warm(format, ...) warm_fn(__FILE__, __LINE__, __func__, format, ##__VA_ARGS__)
#define notice(format, ...) notice_fn(__FILE__, __LINE__, __func__, format, ##__VA_ARGS__)
#define info(format, ...) info_fn(__FILE__, __LINE__, __func__, format, ##__VA_ARGS__)
#define debug(format, ...) debug_fn(__FILE__, __LINE__, __func__, format, ##__VA_ARGS__)

#endif // !LOG_H
