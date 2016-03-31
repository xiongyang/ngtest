#pragma once

#include "ILogger.h"
#include <vector>
#include <string>
#include <mutex>

namespace BluesTrading
{
    class nullLogger :public ILogger
    {
    public:
        virtual void Log(LogLevel level, const char* buf) override {}
        virtual void LogEOD(const std::string& header, const std::string& row) override {
            std::lock_guard<std::mutex> guard(mutex);
            result.push_back(row);
        }

        std::vector<std::string> getResult() { return result;}
    private:
        std::vector<std::string> result;
        std::mutex mutex;
    };
}