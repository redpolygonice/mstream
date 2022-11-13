#include "log.h"
#include "common.h"

LoggerPtr Log::_instance = nullptr;

Log::Log()
	: _active(false)
	, _verb(Level::Debug)
{
	string logName = "log-" + currentTime() + ".txt";
	_stream.open(logName, std::ios_base::out | std::ios_base::trunc);
	if (!_stream.is_open())
		return;

	_active = true;
	_stream << "[" << currentTimeMs() << "] " << "Start"  << std::endl;
}

Log::~Log()
{
	_active = false;
	if (_stream.is_open())
	{
		_stream << "[" << currentTimeMs() << "] " << "Quit"  << std::endl;
		_stream.close();
	}
}

void Log::write(const std::string &text, Log::Level level)
{
	if (level > _verb)
		return;

	string line = createLine(text, level);
	std::cout << line << std::endl;
	_stream << line << std::endl;
	_stream.flush();
}

std::string Log::createLine(const std::string &text, Log::Level level)
{
	string prefix = "";
	switch (level)
	{
	case Log::Level::Info:
		prefix = "Info!    ";
		break;
	case Log::Level::Warning:
		prefix = "Warning! ";
		break;
	case Log::Level::Error:
		prefix = "Error!   ";
		break;
	case Log::Level::Critical:
		prefix = "Critical!!! ";
		break;
	case Log::Level::Debug:
		prefix = "Debug!   ";
		break;
	}

	string line = "[" + currentTimeMs() + "] " + prefix + text;
	return line;
}
