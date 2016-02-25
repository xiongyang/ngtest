
#include "SimpleStrategy.h"
using namespace BluesTrading;
extern "C"
{
    BluesTrading::IStrategy* createStrategy(const std::string& name, ILogger* logger, IConfigureManager* configureManager, IMarketDataProvider* dataProvider, ITimerProvider* timerProvider, IOrderManger* orderManager)
    {
        return new BluesTrading::SimpleStrategy(name, logger, configureManager, dataProvider, timerProvider, orderManager);
    }
};



namespace BluesTrading
{
    SimpleStrategy::SimpleStrategy(const std::string& name, ILogger* logger, IConfigureManager* configureManager, IMarketDataProvider* dataProvider, ITimerProvider* timerProvider, IOrderManger* orderManager)
    {
        std::cout <<"create SimpleStrategy" << std::endl;
    }

    SimpleStrategy::~SimpleStrategy()
    {

    }

    void SimpleStrategy::onTimer(uint32_t eventID, uint32_t currentTime)
    {

    }

    void SimpleStrategy::onMarketData(const CTickData&)
    {
        //std::cout << "On Data\n";
    }

}

