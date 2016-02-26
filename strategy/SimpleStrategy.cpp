#include "SimpleStrategy.h"
using namespace BluesTrading;
extern "C"
{
    BluesTrading::IStrategy* createStrategy(const char* name, ILogger* logger, IConfigureManager* configureManager, IMarketDataProvider* dataProvider, ITimerProvider* timerProvider, IOrderManger* orderManager)
    {
        return new BluesTrading::SimpleStrategy(name, logger, configureManager, dataProvider, timerProvider, orderManager);
    }
};



namespace BluesTrading
{
    SimpleStrategy::SimpleStrategy(const char* name, ILogger* logger, IConfigureManager* configureManager, IMarketDataProvider* dataProvider, ITimerProvider* timerProvider, IOrderManger* orderManager)
     :logger_(logger),
    configManager_(configureManager),
    dataprovider_(dataProvider),
    timerprovider_(timerProvider),
    orderManager_(orderManager)
    {
        dataprovider_->subscribeInstrument(1, this);
        dataprovider_->subscribeInstrument(2, this);
        uint32_t now = timerprovider_->getCurrentTimeMsInDay();
        uint32_t targetTime = 9 * 3600 * 1000 + 50 * 60 * 1000;     // 09:50:00

        timerprovider_->setTimer(this, 1, targetTime - now, false); // target time 
        timerprovider_->setTimer(this, 2, 30000, true);
    }

    SimpleStrategy::~SimpleStrategy()
    {

    }

    void SimpleStrategy::onTimer(uint32_t eventID, uint32_t currentTime)
    {
        std::cout << "EventID:" << eventID << " Time:" << currentTime << std::endl;
        if(eventID == 1)
        {
             timerprovider_->setTimer(this, 3, 60000, true); // target time 
        }
    }

    void SimpleStrategy::onMarketData(const CTickData& data)
    {
        static int count = 0;
        count ++ ;
        if (count % 100 == 0)
        {
            std::cout << "On Data " << count << "\n";
        }
    }

    void SimpleStrategy::onUpdateOrder(OrderDataDetail* orderData)
    {

    }

}

