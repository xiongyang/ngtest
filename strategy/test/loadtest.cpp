
#include <iostream>
#include "IStrategy.h"
#include "dynamicloader.h"

int main(int argc, char** argv)
{
    std::cout << "[ RUN      ] \n";

    if (argc < 2)
    {
        std::cout << "Test Argc = " << argc << " "<< argv[0];
        std::cout<< "[  FAILED  ] LoadDllTest \n\n";
        return -1;
    }

    auto funptr = GetSharedLibFun<BluesTrading::StrategyFactoryFun>(argv[1],"createStrategy");
    //CDynamicLibrary dll;
    //dll.Open(argv[1]);
    //auto funptr = reinterpret_cast<BluesTrading::StrategyFactoryFun*>(dll.GetProc("createStrategy"));
    BluesTrading::IStrategy* strp =  funptr(NULL,NULL,NULL,NULL,NULL,NULL,NULL);
    if (strp != NULL)
    {
        std::cout << "[       OK ] LoadDllTest\n\n"; 
        return 0;
    }
    else
    {
        std::cout<< "[  FAILED  ] LoadDllTest \n\n";
        return -1;
    }


}