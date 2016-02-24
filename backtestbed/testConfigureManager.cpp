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
        readConfigure(configFileStream);
    }

    void TestConfigureManager::readConfigure(std::istream& inputstream)
    {
        for (std::string line; std::getline(inputstream, line); ) 
        {
            //sum += atoi(line.c_str());
            configureLines.push_back(line);
        }
    }


    void TestConfigureManager::notifyConfigurableUpdate(IConfigureable*)
    {

    }



    void TestConfigureManager::configurInstance()
    {
        for (auto each : configureLines)
        {
            test_strategy->onMessage(each);
        }
    }

}