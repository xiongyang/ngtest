#pragma once
#include <string>

namespace BluesTrading
{

class ILogger
{
public:
	enum LogLevel
	{
		DEBUG,
		INFO,
		WARNING,
		ERROR,
		FATAL
	};
	void Log(LogLevel level, const char* buf);		// log to file 
	void LogEOD(const std::string& header, const std::string& row);	// Log Result of back test or Position and¡¡PNL
};

}