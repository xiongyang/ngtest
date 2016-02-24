#pragma once

#include "IConfigureManager.h"

#include <string>
#include <vector>
#include <istream>

namespace BluesTrading
{
    class TestConfigureManager:public IConfigureManager
    {
    public:
        TestConfigureManager():test_strategy(nullptr) {}
        virtual void registerConfigurable(IConfigureable*) override;
        virtual void notifyConfigurableUpdate(IConfigureable*) override;

    public:
        void readConfigure(const std::string& configureFile);
        void readConfigure(std::istream& inputstream);
        void configurInstance();

        IConfigureable* test_strategy;
        std::vector<std::string> configureLines;
    };
}