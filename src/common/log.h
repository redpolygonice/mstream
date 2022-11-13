#ifndef LOG_H
#define LOG_H

#include "types.h"

#include <fstream>
#include <sstream>

typedef std::function<void(const string &text)> LogFunction;

#define LOG(...) { std::stringstream __stream; __stream << __VA_ARGS__; log(__stream.str()); }
#define LOGW(...) { std::stringstream __stream; __stream << __VA_ARGS__; logw(__stream.str()); }
#define LOGE(...) { std::stringstream __stream; __stream << __VA_ARGS__; loge(__stream.str()); }
#define LOGC(...) { std::stringstream __stream; __stream << __VA_ARGS__; logc(__stream.str()); }
#define LOGD(...) { std::stringstream __stream; __stream << __VA_ARGS__; logd(__stream.str()); }

class Log;
typedef std::shared_ptr<Log> LoggerPtr;

// Logging class
class Log
{
public:
	Log();
	~Log();
	enum class Level
	{
		Critical,
		Error,
		Warning,
		Info,
		Debug
	};

private:
	static LoggerPtr _instance;
	std::atomic_bool _active;
	Level _verb;
	std::ofstream _stream;

public:
	void setVerbosity(Level level) { _verb = level; }
	void write(const string &text, Log::Level level = Log::Level::Info);

	static LoggerPtr create()
	{
		if (_instance == nullptr)
			_instance = std::make_shared<Log>();
		return _instance;
	}

	static void close()
	{
		_instance.reset();
		_instance = nullptr;
	}

	static void put(const string &text, Log::Level level)
	{
		_instance->write(text, level);
	}

	static void setVerb(Level level)
	{
		_instance->setVerbosity(level);
	}

private:
	void run();
	string createLine(const string &text, Log::Level level);
};

inline void log(const string &text, Log::Level level = Log::Level::Info) { Log::put(text, level); }
inline void logw(const string &text) { log(text, Log::Level::Warning); }
inline void loge(const string &text) { log(text, Log::Level::Error); }
inline void logc(const string &text) { log(text, Log::Level::Critical); }
inline void logd(const string &text) { log(text, Log::Level::Debug); }

#endif // LOG_H
