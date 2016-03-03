#include "testpositionmanager.h"
#include <algorithm>
#include <iterator>
#include "util.h"

namespace BluesTrading
{


    CPosition& testPositionManger::getPosition(uint32_t inst)
    {
        return allPositions_[inst];
    }

    AccountInfo& testPositionManger::getAccountInfo()
    {
        return accountInfo;
    }

    IPositionManager::PositionContainer& testPositionManger::getAllPosition()
    {
        return allPositions_;
    }

    void testPositionManger::updateOrder(const OrderDataDetail* order)
    {
        const SSE_OrderDetail& sseOrder = order->sse_order;
         CPosition&  pos = getPosition(sseOrder.instrumentID);
         if (SSE_OrderDetail::SSE_OrderTraded == sseOrder.orderStatus)
         {
             if (order->sse_order.isbuy)
             {
                 TradeInfo trade;
                 trade.is_open  = true;
                 trade.is_long = true;
                 trade.is_today = true;
                 trade.price = sseOrder.tradeprice;
                 trade.qty = sseOrder.filledQty;
                 pos.addTrade(trade);

                 accountInfo.cash -=   trade.price *   trade.qty;
             }
             else
             {
                 TradeInfo trade;
                 trade.is_open  = false;
                 trade.is_long = false;
                 trade.is_today = false;        //SSE order only sell yst position
                 trade.price = sseOrder.tradeprice;
                 trade.qty = sseOrder.filledQty;
                 pos.addTrade(trade);

                 accountInfo.cash +=   trade.price *   trade.qty;
             }
         } 
    }


    void testPositionManger::resetPositionTodayToYst()
    {
        for (auto each_pair : allPositions_)
        {
           CPosition& pos = each_pair.second;
           CPosition::PositionItemContainer& todayPos = pos.getPositions(CPosition::LongToday);
           CPosition::PositionItemContainer& ystPos = pos.getPositions(CPosition::LongYst);
           std::transform(todayPos.begin(), todayPos.end(),std::back_inserter(ystPos), [&](PositionItem& item){item.is_today =false; return item;});
    
           todayPos.clear();
        }
    }

    void testPositionManger::updatePrice(const CTickData& data)
    {
      auto iter =  allPositions_.find(data.instIndex);
      if (iter != allPositions_.end())
      {
          iter->second.updatePrice(data.last_price);
      }
    }

    void testPositionManger::printPnl(uint32_t date)
    {
        std::cout << "printPnl " << date << std::endl;
        auto printPos = [](auto& posPair)
        {
            CPosition& pos = posPair.second;
            CPosition::PositionItemContainer& todayPos = pos.getPositions(CPosition::LongToday);
            CPosition::PositionItemContainer& ystPos = pos.getPositions(CPosition::LongYst);

            const std::pair<uint32_t, double> zero_qty_ammount {0, 0.0};

            auto accumulate = [](const auto& sum, const auto& positem)
            {
                auto  sumplus = sum;
                sumplus.first +=  positem.qty;
                sumplus.second += positem.qty * positem.price;
                return sumplus;
            };
            std::pair<uint32_t, double> todayPosSum  = std::accumulate(todayPos.begin(), todayPos.end(), zero_qty_ammount , accumulate);
            std::pair<uint32_t, double>  ystPosSum  = std::accumulate(ystPos.begin(), ystPos.end(), zero_qty_ammount , accumulate);
            double avgPrice = (todayPosSum.second + ystPosSum.second) / (todayPosSum.first + ystPosSum.first);
            std::cout << boost::format("Inst:%1% Position[%2%(yst) + %3%(today) = %4%] AvgPrice %5% [RelizedPnl: %6%] [PositionPnl: %7%] \n")
                % posPair.first % ystPosSum.first % todayPosSum.first % (todayPosSum.first + ystPosSum.first) % avgPrice % pos.getRealizedPnL() % pos.getPositionPnl()
                << std::endl;
        };

        std::for_each(allPositions_.begin(), allPositions_.end(), printPos);
      
    }

    void testPositionManger::onMarketData(const CTickData& tick)
    {
        updatePrice(tick);
    }

    void testPositionManger::onStartDay(uint32_t date)
    {
        std::cout << "PositionManager OnStart Day" << date << std::endl;
        resetPositionTodayToYst();
    }

    void testPositionManger::onEndDay(uint32_t date)
    {
        std::cout << "PositionManager onEndDay Day" << date << std::endl;
        printPnl(date);
        removeEmptyPosition();
    }

    void testPositionManger::removeEmptyPosition()
    {
        auto is_empty_pos = [](auto& posPair)
        {
            CPosition& pos = posPair.second;
            CPosition::PositionItemContainer& todayPos = pos.getPositions(CPosition::LongToday);
            CPosition::PositionItemContainer& ystPos = pos.getPositions(CPosition::LongYst);
            return (todayPos.empty() && ystPos.empty());
        };

        remove_if_map(allPositions_,is_empty_pos);

        //for(auto& iter = allPositions_.begin(); iter != allPositions_.end(); ++ iter)
        //{
        //    if (is_empty_pos(*iter))
        //    {
        //        allPositions_.erase(iter++);
        //    }
        //}
    }

}