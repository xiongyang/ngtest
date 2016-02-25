
#include "../testbed.h"
#include <iostream>
#include "boost/filesystem.hpp"
#include "../marketdatastore.h"
#include <string>
using namespace BluesTrading;


void cookData(const std::string& dirName)
{
    if (!boost::filesystem::exists(dirName))
    {
        std::cout << "dir not exists" << dirName;
    }

    if (boost::filesystem::is_directory(dirName))
    {
        for (boost::filesystem::directory_entry& x : boost::filesystem::directory_iterator(dirName))
        {
            if (boost::filesystem::is_regular_file(x))
            {
                if(".csv" ==  x.path().extension().string())
                {
                    MarketDataStore inst( x.path().string());
                    boost::filesystem::path newpath = x.path();
                    inst.saveToBinFile( newpath.replace_extension(".bin").string());
                }
            }
        }
    }
    else
    {    
        MarketDataStore inst(dirName);
        inst.saveToBinFile(dirName + ".bin");
    }
}

void testBedRun(const std::string& dir, const std::string& strategy)
{

    TestBed inst;
    inst.Init(dir, strategy);
    inst.run();
    return ;
}


int main(int argc, char** argv)
{
    std::string cmd = argv[1];
    std::string dirName = argv[2];
 
    if(cmd == "cook")
    {
        cookData(dirName);
    }
    else if(cmd == "tb")
    {
        testBedRun(dirName,argv[3]);
    }

    return 0;
}


