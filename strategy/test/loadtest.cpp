
#include <iostream>
#include "IStrategy.h"
#include "dynamicloader.h"

int main(int argc, char** argv)
{
	if (argc < 2)
	{
		std::cout << "not enough paras";
		return -1;
	}

	CDynamicLibrary dll;
	dll.Open(argv[1]);
	auto funptr = reinterpret_cast<BluesTrading::StrategyFactoryFun*>(dll.GetProc("createStrategy"));
	BluesTrading::IStrategy* strp =  funptr(NULL,NULL,NULL,NULL,NULL);
	if (strp != NULL)
	{
		std::cout << "Yes Load Ok \n";
		std::cout  << strp->getName() << "   " << strp->getConfigureableMessage() << "   " << strp->getDisplayMessage() << std::endl;
		return 0;
	}
	else
	{
		std::cout << "Load Fail" << std::endl;
		return -1;
	}

}