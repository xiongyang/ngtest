#include "testConfigureManager.h"
#include <fstream>


namespace BluesTrading
{

    void TestConfigureManager::registerConfigurable(IConfigureable* para)
    {
        test_strategy = para;
    }

    void TestConfigureManager::readConfigure(const std::string& configureFile)
    {
        std::fstream configFileStream(configureFile, std::ios_base::in);
       
        for(std::string linebuf; std::getline(configFileStream,linebuf);)
        {
            std::string insertLine;
            insertLine.swap(linebuf);
            configureLines.push_back(std::move(insertLine));
        }

    }

    void TestConfigureManager::configurInstance()
    {
        for (auto each : configureLines)
        {
              test_strategy->onMessage(each);
        }
    }

}