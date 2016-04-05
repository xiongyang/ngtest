#include "testpositionmanager.h"
#include <algorithm>
#include <iterator>
#include "util.h"

namespace BluesTrading
{


    CPosition& testPositionManger::getPosition(uint32_t inst)
    {
        auto iter = allPositions_.find(inst);
        if(iter != allPositions_.end())
        {
            return iter->second;
        }
        else
        {
              CPosition&  newPos = allPositions_[inst];
              newPos.setUintMultiplier(InstrumentInfoFactory::getInstrumentUnitMultiplier(inst));
              return newPos;
        }
    }

    AccountInfo& testPositionManger::getAccountInfo()
    {
        return accountInfo;
    }

    IPositionManager::PositionContainer& testPositionManger::getAllPosition()
    {
        return allPositions_;
    }

    void testPositionManger::onUpdateOrder(const OrderDataDetail* order)
    {

        if (order->exchangeType  ==  Exch_SSE || order->exchangeType  ==  Exch_SZE)
        {
            handleSecurityOrder(order);
        }
        else
        {
            handleFutureOrder(order);
        }
    }




    void testPositionManger::handleSecurityOrder(const OrderDataDetail* order)
    {
        const SSE_OrderDetail& sseOrder = order->sse_order;
        CPosition&  pos = getPosition(sseOrder.instrumentID);
        if (OrderFilled == sseOrder.common.orderStatus)
        {
            if (LongShortFlag_Long  == sseOrder.longshortflag)
            {
                TradeInfo trade;
                trade.is_open  = true;
                trade.is_long = true;
                trade.is_today = true;
                trade.price = sseOrder.tradeprice;
                trade.qty = sseOrder.filledQty;
                pos.addTrade(trade);
              //  accountInfo.cash -=   trade.price *   trade.qty;
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

               // accountInfo.cash +=   trade.price *   trade.qty;
            }
        } 
    }

    void testPositionManger::handleFutureOrder(const OrderDataDetail* orderp)
    {
        const FutureOrderDetail& order = orderp->cffex_order;
        CPosition&  pos = getPosition(order.instrumentID);
        if (OrderFilled == order.common.orderStatus)
        {

            if(OpenCloseFlag_Open == order.openCloseType )
            {
                  TradeInfo trade;

                  trade.is_open  =  true;
                  trade.is_long =  (LongShortFlag_Long == order.longshortflag);
                  trade.is_today = true;
                  trade.price = order.tradeprice;
                  trade.qty = order.filledQty;
                  pos.addTrade(trade);
                  //accountInfo.cash -=   trade.price *   trade.qty * 
                  //    InstrumentInfoFactory::getInstrumentUnitMultiplier(order.instrumentID) * 
                  //    InstrumentInfoFactory::getInstrumentMarginRate(order.instrumentID);
            }
            else
            {
                TradeInfo trade;

                trade.is_open  =  false;
                trade.is_long =  (LongShortFlag_Long == order.longshortflag);
                trade.is_today =  (OpenCloseFlag_CloseToday  == order.openCloseType);
                trade.price = order.tradeprice;
                trade.qty = order.filledQty;
                pos.addTrade(trade);
                //accountInfo.cash -=   trade.price *   trade.qty * 
                //    InstrumentInfoFactory::getInstrumentUnitMultiplier(order.instrumentID) * 
                //    InstrumentInfoFactory::getInstrumentMarginRate(order.instrumentID);


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
      //  std::cout << "printPnl " << date << std::endl;

        for (auto& posPair : allPositions_)
        {
            CPosition& pos = posPair.second;
            CPosition::PositionItemContainer& todayPos = pos.getPositions(CPosition::LongToday);
            CPosition::PositionItemContainer& ystPos = pos.getPositions(CPosition::LongYst);
            
            uint32_t tpos = 0;
            double todayBasis = 0;
            for (auto& eachTodayPos : todayPos)
            {
                tpos += eachTodayPos.qty;
                todayBasis += eachTodayPos.price * eachTodayPos.qty;
            }

            uint32_t yPos = 0;
            double ystBasis = 0;
            for (auto& eachPos : ystPos)
            {
                yPos += eachPos.qty;
                ystBasis += eachPos.price * eachPos.qty;
            }

            std::cout <<  boost::format("Inst:%1% Position[%2%(yst) + %3%(today) = %4%]") 
                % posPair.first % yPos % tpos % (tpos + yPos) << std::endl;

        }

        //auto printPos = [](auto& posPair)
        //{
        //    CPosition& pos = posPair.second;
        //    CPosition::PositionItemContainer& todayPos = pos.getPositions(CPosition::LongToday);
        //    CPosition::PositionItemContainer& ystPos = pos.getPositions(CPosition::LongYst);

        //    const std::pair<uint32_t, double> zero_qty_ammount {0, 0.0};

        //    auto accumulate = [](const auto& sum, const auto& positem)
        //    {
        //        auto  sumplus = sum;
        //        sumplus.first +=  positem.qty;
        //        sumplus.second += positem.qty * positem.price;
        //        return sumplus;
        //    };
        //    std::pair<uint32_t, double> todayPosSum  = std::accumulate(todayPos.begin(), todayPos.end(), zero_qty_ammount , accumulate);
        //    std::pair<uint32_t, double>  ystPosSum  = std::accumulate(ystPos.begin(), ystPos.end(), zero_qty_ammount , accumulate);
        //    double avgPrice = (todayPosSum.second + ystPosSum.second) / (todayPosSum.first + ystPosSum.first);
        //    std::cout << boost::format("Inst:%1% Position[%2%(yst) + %3%(today) = %4%] AvgPrice %5% [RelizedPnl: %6%] [PositionPnl: %7%] \n")
        //        % posPair.first % ystPosSum.first % todayPosSum.first % (todayPosSum.first + ystPosSum.first) % avgPrice % pos.getRealizedPnL() % pos.getPositionPnl()
        //        << std::endl;
        //};

        //std::for_each(allPositions_.begin(), allPositions_.end(), printPos);
      
    }

    void testPositionManger::onMarketData(const CTickData& tick)
    {
        updatePrice(tick);
    }

    void testPositionManger::onStartDay(uint32_t date)
    {
        //std::cout << "PositionManager OnStart Day" << date << std::endl;
        resetPositionTodayToYst();
        for (auto& each_pos : allPositions_)
        {
           accountInfo.cash += each_pos.second.getPositionPnl();
           each_pos.second.resetPnl();
           each_pos.second.resetTradeCount();
        }

    }

    void testPositionManger::onEndDay(uint32_t date)
    {
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