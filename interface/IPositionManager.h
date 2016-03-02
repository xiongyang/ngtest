#pragma once

#include <cstdint>
#include <deque>
#include "boost/format.hpp"

namespace BluesTrading
{

    struct TradeInfo
    {
        double price;
        uint32_t qty;
        bool is_open;
        bool is_long;
        bool is_today;
    };

    struct PositionItem
    {
        double price;
        uint32_t qty;
        bool is_long;
        bool is_today;
    };

    // This class should only do the position and pnl calc
    class CPosition
    {
    public:


        enum PositionType{
            LongToday,
            LongYst,
            ShortToday,
            ShortYst
        };

        typedef std::deque<PositionItem> PositionContainer;


        void updatePrice(double price) {lastPrice_ = price;}

        double getPositionPnl() const {return  getPositionPnl(lastPrice_);}
       
        double getRealizedPnL() const {return realizedPnL_;}

        //=======================   impl for CPosition fun =================
        void addPosition(PositionItem& positionItem)
        {
            if (positionItem.is_long)
            {
                if (positionItem.is_today)
                {
                    longPosition_.push_back(positionItem);
                }
                else
                {
                    longPositionYst_.push_back(positionItem);
                }
            }
            else
            {
                if (positionItem.is_today)
                {
                    shortPosition_.push_back(positionItem);
                }
                else
                {
                    shortPositionYst_.push_back(positionItem);
                }
            }
        }


        void  addTrade(const TradeInfo& trade)
        {
            if (trade.is_open)
            {
                PositionItem item{trade.price, trade.qty, trade.is_long, trade.is_today};
                addPosition(item);
            }
            else
            {

                auto shortCalcPrice = [](double tradePrice, double posPrice) {return posPrice - tradePrice;};
                auto longCalcPrice = [] (double tradePrice, double posPrice){return tradePrice - posPrice;};

                auto tradeMatchPos = [&] (TradeInfo trade, PositionContainer& posQueue, auto calc_diff)
                {
                    while (trade.qty > 0 && !posQueue.empty())
                    {
                        PositionItem& oldest_pos = posQueue.front();
                        uint32_t match_qty  = std::min(oldest_pos.qty,  trade.qty);
                        realizedPnL_ += match_qty * calc_diff(trade.price, oldest_pos.price) * unitMultiplier_;

                        oldest_pos.qty -= match_qty;
                        trade.qty -= match_qty;
                        if (oldest_pos.qty == 0)
                        {
                            posQueue.pop_front();
                        }
                    }

                    if (trade.qty > 0)
                    {
                        std::cout << boost::format(" Close %1% Trade does not match Position  Price:%2%  Qty:%3%") % trade.is_long % trade.price % trade.qty <<std::endl;
                    }
                };


                if (trade.is_long)
                {

                    if(trade.is_today)
                    {
                        tradeMatchPos(trade, shortPosition_, longCalcPrice);
                    }
                    else
                    {
                        tradeMatchPos(trade, shortPositionYst_, longCalcPrice);
                    }
                }
                else
                {
                    if(trade.is_today)
                    {
                        tradeMatchPos(trade, longPosition_, shortCalcPrice);
                    }
                    else
                    {
                        tradeMatchPos(trade, longPositionYst_, shortCalcPrice);
                    }
                }
            }
        }


        double getPositionPnl(double price) const
        {
            double positionPnl = 0;

            auto acculumate = [&](const PositionContainer& positionQueue)
            {
                for (auto each: positionQueue)
                {
                    if (each.is_long)
                    {
                        positionPnl +=  (each.price - price) * each.qty * unitMultiplier_;
                    }
                    else
                    {
                        positionPnl +=  (price - each.price) * each.qty * unitMultiplier_;
                    }
                }
            };

            acculumate(shortPosition_);
            acculumate(shortPositionYst_);
            acculumate(longPosition_);
            acculumate(longPositionYst_);

            return realizedPnL_ + positionPnl;
        }

        const CPosition::PositionContainer&  getPositions(PositionType postype) const
        {
            switch(postype)
            {
            case LongToday:
                return longPosition_;
            case ShortToday:
                return shortPosition_;
            case LongYst:
                return longPositionYst_;
            case ShortYst:
                return shortPositionYst_;
            }
        }

    private:
        uint32_t instrumentID_;
        double lastPrice_;
        uint32_t updateTick_;
        uint32_t unitMultiplier_;
        double realizedPnL_;


        PositionContainer longPosition_;
        PositionContainer longPositionYst_;
        PositionContainer shortPosition_;
        PositionContainer shortPositionYst_;

    };


    struct AccountInfo
    {
        uint32_t accountID;
        double cash;
        double margin;
        double commison;
    };

    class IPositionManager
    {
    public:


        virtual CPosition& getPosition(uint32_t inst) = 0;
        //{
        //    return allPositions_[inst];
        //}

        virtual AccountInfo& getAccountInfo() = 0;


        //private:
        //    std::map<uint32_t, CPosition> allPositions_;
    };
}