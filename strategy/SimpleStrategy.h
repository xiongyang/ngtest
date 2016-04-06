#pragma once
#include "IStrategy.h"

/*extern "C"
{
    using namespace BluesTrading;
    IStrategy* createStrategy(const char* name, ILogger* logger, 
        IConfigureManager* configureManager, IMarketDataProvider* dataProvider, ITimerProvider* timerProvider, IOrderManger* orderManager,
        IPositionManager* posMgr);
};*/


namespace BluesTrading
{

    class SimpleStrategy : public IStrategy
    {
    public:
        SimpleStrategy(const char* name,  ILogger* logger, IConfigureManager* configureManager,
            IMarketDataProvider* dataProvider, ITimerProvider* timerProvider, IOrderManger* orderManager, IPositionManager* position);

        virtual ~SimpleStrategy();

        virtual void onTimer(uint32_t eventID, uint32_t currentTime) override;
        virtual void onStartDay(uint32_t date) override;
        virtual void onEndDay(uint32_t date) override ;

        virtual void onMarketData(const CTickData&) override;
        virtual void onUpdateOrder(OrderDataDetail* orderData) override;

        virtual void onMessage(const std::string& propName) override;	//Receive ProtoBuf Message From Console
        virtual std::string getDisplayMessage() override {return "DisplayMessage For Simple";};		//ProtoBuf Message For Display
        virtual std::string getConfigurableMessage() override {return "ConfigureMessage  For Simple";};	//ProtoBuf Message For Configure
        virtual std::string getName() override {return "TestStrategy Name  For Simple";};	

    private:
        ILogger*    logger_;
        IConfigureManager* configManager_;
        IMarketDataProvider* dataprovider_;
        ITimerProvider* timerprovider_;
        IOrderManger* orderManager_;
        IPositionManager* positionManager_;
        double lastprice_;

        struct  strategyParameters
        {
            std::string prop1;
            std::string prop2;
            std::string prop3;
            std::string prop4;
            std::string prop5;
            std::string prop6;
        };

        strategyParameters paras_;

    };
}