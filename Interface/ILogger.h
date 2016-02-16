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
        virtual void Log(LogLevel level, const char* buf) = 0;		// log to file 
        virtual void LogEOD(const std::string& header, const std::string& row) = 0;	// Log Result of back test or Position and¡¡PNL
    };

}