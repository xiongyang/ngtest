
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
     :logger_(logger),
    configManager_(configureManager),
    dataprovider_(dataProvider),
    timerprovider_(timerProvider),
    orderManager_(orderManager)
    {
        std::cout << "xxxx handler:"<< this << " subscribe inst:" << 2  <<  " onDataSrc:"<< dataprovider_ << std::endl;
        //dataprovider_->subscribeInstrument(1, this);
        dataprovider_->subscribeInstrument(2, this);
        //timerProvider_->setTimer(1, )
    }

    SimpleStrategy::~SimpleStrategy()
    {

    }

    void SimpleStrategy::onTimer(uint32_t eventID, uint32_t currentTime)
    {

    }

    void SimpleStrategy::onMarketData(const CTickData&)
    {
        static int count = 0;
        count ++ ;
         std::cout << "On Data " << count << "\n";
        if (count % 100 == 0)
        {
            std::cout << "On Data " << count << "\n";
        }
    }

    void SimpleStrategy::onUpdateOrder(OrderDataDetail* orderData)
    {

    }

}

