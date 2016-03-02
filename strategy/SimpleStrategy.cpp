#include "SimpleStrategy.h"
#include "util.h"

using namespace BluesTrading;
extern "C"
{
    BluesTrading::IStrategy* createStrategy(const char* name, ILogger* logger, IConfigureManager* configureManager,
        IMarketDataProvider* dataProvider, ITimerProvider* timerProvider, IOrderManger* orderManager, IPositionManager* posMgr)
    {
        return new BluesTrading::SimpleStrategy(name, logger, configureManager, dataProvider, timerProvider, orderManager, posMgr);
    }
};



namespace BluesTrading
{
    SimpleStrategy::SimpleStrategy(const char* name, ILogger* logger, IConfigureManager* configureManager, 
        IMarketDataProvider* dataProvider, ITimerProvider* timerProvider, IOrderManger* orderManager, IPositionManager* posmgr)
     :logger_(logger),
    configManager_(configureManager),
    dataprovider_(dataProvider),
    timerprovider_(timerProvider),
    orderManager_(orderManager),
    positionManager_(posmgr)
    {
        timerProvider->registerTimerConsumer(this);
    }

    SimpleStrategy::~SimpleStrategy()
    {

    }

    void SimpleStrategy::onTimer(uint32_t eventID, uint32_t currentTime)
    {
        std::cout << "EventID:" << eventID << " Time:" << currentTime << std::endl;
        if(eventID == 1)
        {
            SSE_SecurityNewOrderRequest request;
            request.instrumentID = 1;
            request.isBuy = true;
            request.orderqty = 100;
            request.orderType = 0; //only limit order
            request.price = lastprice_;
            request.priceType = 0; // only limit order
            std::cout << "send order at time " <<lastprice_ << " currentTime:"  << currentTime << "\n";
            submitRequest(request, orderManager_);
           //  timerprovider_->setTimer(this, 3, 60000, true); // target time 
        }
    }

    void SimpleStrategy::onStartDay(uint32_t date)
    {
        std::cout << "start day " << date << "\n";
        orderManager_->regsiterOrderDataConsumer(this);

        dataprovider_->subscribeInstrument(1, this);
        dataprovider_->subscribeInstrument(2, this);
        uint32_t now = timerprovider_->getCurrentTimeMsInDay();
        uint32_t targetTime = 9 * 3600 * 1000 + 50 * 60 * 1000;     // 09:50:00

        timerprovider_->setTimer(this, 1, targetTime - now, false); // target time 
      //  timerprovider_->setTimer(this, 2, 60000, true);
        
    }

    void SimpleStrategy::onMarketData(const CTickData& data)
    {
        static int count = 0;
        count ++ ;
      //   std::cout << "onMarketData\n";
        if(data.instIndex == 1)
        {
              lastprice_  = data.last_price;
        }
      
        if (count % 100 == 0)
        {
            SSE_SecurityNewOrderRequest request;
            request.instrumentID = data.instIndex;
            request.isBuy = true;
            request.orderqty = 100;
            request.orderType = 0; //only limit order
            request.price = data.last_price;
            request.priceType = 0; // only limit order
            std::cout << "send order " << data.last_price << "\n";
           submitRequest(request, orderManager_);
        }
    }

    void SimpleStrategy::onUpdateOrder(OrderDataDetail* orderData)
    {
        printOrder(std::cout, *orderData);
    }

}

