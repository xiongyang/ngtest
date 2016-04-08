#pragma once

#include "ILogger.h"
#include <vector>
#include <string>
#include <mutex>
#include <memory>

namespace BluesTrading
{
    class nullLogger :public ILogger
    {
    public:
       
        virtual void Log(LogLevel level, const char* buf) override {}
        virtual void LogEOD(const std::string& header, const std::string& row) override {
            headLine = header;
            result.push_back(row);
        }

        std::pair<std::string, std::vector<std::string>> getResult() {  return std::make_pair(std::move(headLine), std::move(result));}
    private:
        std::vector<std::string> result;
        std::mutex mutex;
        std::string headLine;
    };
}