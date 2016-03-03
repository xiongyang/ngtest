#pragma once
#include "IPositionManager.h"
#include "ITimer.h"
#include "IMarketDataProvider.h"
#include "MarketData.h"
#include "OrderData.h"
#include <map>
namespace BluesTrading
{

    class testPositionManger : public IPositionManager , public ITickDataConsumer, public ITimerConsumer
    {
    public:

        virtual CPosition& getPosition(uint32_t inst) override;

        virtual AccountInfo& getAccountInfo() override;

        virtual PositionContainer& getAllPosition() override;

        void updateOrder(const OrderDataDetail* order);
        void resetPositionTodayToYst();
        void updatePrice(const CTickData& data);

        void printPnl(uint32_t date);
        void setInitCash(double cash) {accountInfo.cash = cash;}

    public:
        virtual void onMarketData(const CTickData& tick) override;
    public:
        virtual void onStartDay(uint32_t date) override;
        virtual void onEndDay(uint32_t date) override ;

    private:
        void removeEmptyPosition();
        private:
            std::map<uint32_t, CPosition> allPositions_;
            AccountInfo    accountInfo;
    };

}