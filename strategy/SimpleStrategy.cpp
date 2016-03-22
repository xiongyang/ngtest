#include "SimpleStrategy.h"
#include "util.h"
#include "bluemessage.pb.h"

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
            timerprovider_->setTimer(this, 2, 60 * 1000 * 10, true); // target time 
        }
        else
        {
            SSE_SecurityNewOrderRequest request;
            request.instrumentID = 1;
            request.isBuy = false;
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

        PositionItem yst_1_pos {2200, 1000, true,false};
        positionManager_->getPosition(1).addPosition(yst_1_pos);
        std::cout << "start day " << date << "\n";

        //TODO order Mask
        orderManager_->subscribeOrderUpdate(0,this); 

        dataprovider_->subscribeInstrument(1, this);
        dataprovider_->subscribeInstrument(2, this);
        uint32_t now = timerprovider_->getCurrentTimeMsInDay();
        uint32_t targetTime = 9 * 3600 * 1000 + 50 * 60 * 1000;     // 09:50:00

        timerprovider_->setTimer(this, 1, targetTime - now, false); // target time 
        //  timerprovider_->setTimer(this, 2, 60000, true);

    }

    void SimpleStrategy::onEndDay(uint32_t date)
    {
        //boost::format fmt("%1%,%2%,%3%,%4%,%5%,%6%,");
        //fmt % date %
        logger_->LogEOD("date,par1,para2,para3,tradenum,profit","");
    }

    void SimpleStrategy::onMarketData(const CTickData& data)
    {
        static int count = 0;
        count ++ ;

        if (data.instIndex == 1)
        {
            lastprice_ = data.last_price;
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
            std::cout << boost::format("SendOrder %1%, Time:%2% ") %  data.last_price % getTimeStr(timerprovider_->getCurrentTimeMsInDay());

           // std::cout << "send order " << data.last_price << " Time:" <<"\n";
            submitRequest(request, orderManager_);
        }
    }

    void SimpleStrategy::onUpdateOrder(OrderDataDetail* orderData)
    {
        printOrder(std::cout, *orderData);
    }

    void SimpleStrategy::onMessage(const std::string& propName)
    {
        StrategyConfig msg;
        msg.ParseFromString(propName);

        for (auto& prop_pair: msg.props())
        {
            const std::string& propName = prop_pair.propname();
            const std::string& propValue = prop_pair.value();

            if(propName == "prop1")
            {
                paras_.prop1 =  propValue;
            }
            else if (propName == "prop2")
            {
                  paras_.prop2 =  propValue;
            }
            else if (propName == "prop3")
            {
                paras_.prop3 =  propValue;
            }
            else if (propName == "prop4")
            {
                paras_.prop4 =  propValue;
            }
            else if (propName == "prop5")
            {
                paras_.prop5 =  propValue;
            }
            else if (propName == "prop6")
            {
                paras_.prop6 =  propValue;
            }
            else
            {
                std::cout << "ERR UNKNOWN PROPNAME " <<  propName << std::endl;
            }
        }
    }

}

