#pragma once

#include "ILogger.h"


namespace BluesTrading
{

    class nullLogger :public ILogger
    {
    public:
        virtual void Log(LogLevel level, const char* buf) override {}
        virtual void LogEOD(const std::string& header, const std::string& row) override {}
    };

}