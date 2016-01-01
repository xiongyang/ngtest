#pragma once
#include "IStrategy.h"

extern "C"
{
    using namespace BluesTrading;
    IStrategy* createStrategy(const std::string& name, ILogger* logger, 
        IConfigureManager* configureManager, IMarketDataProvider* dataProvider, ITimerProvider* timerProvider, IOrderManger* orderManager);
};


namespace BluesTrading
{

    class SimpleStrategy : public IStrategy
    {
    public:
        SimpleStrategy(const std::string& name,  ILogger* logger, IConfigureManager* configureManager,
            IMarketDataProvider* dataProvider, ITimerProvider* timerProvider, IOrderManger* orderManager);

        virtual ~SimpleStrategy();

        virtual void onTimer(uint32_t eventID, uint32_t currentTime) override;
        virtual void onStartDay(uint32_t) override {};
        virtual void onEndDay(uint32_t) override {};

        //virtual void onMarketData(const TickDataLevel1&) override{};
        virtual void onMarketData(const TickDataLevel5&) override {};
        //virtual void onMarketData(const TickDataLevel10&) override{};
        //virtual void onMarketData(const TickDataLevel20&) override{};
        //virtual void onOtherLevelsMarketData(const CTickData<1>& tick) override{};		// for any other Levels.

        virtual void onUpdateOrder(OrderDataDetail* orderData) override {};

        virtual void onMessage(const std::string& propName) override {};	//Receive ProtoBuf Message From Console
        virtual std::string getDisplayMessage() override {return "DisplayMessage For Simple";};		//ProtoBuf Message For Display
        virtual std::string getConfigureableMessage() override {return "ConfigureMessage  For Simple";};	//ProtoBuf Message For Configure
        virtual std::string getName() override {return "TestStrategy Name  For Simple";};	
    };
}