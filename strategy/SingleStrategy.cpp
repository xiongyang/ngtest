#include "SingleStrategy.h"
#include "util.h"
#include "bluemessage.pb.h"
#include <string.h>
#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>

using namespace BluesTrading;
extern "C"
{
    BluesTrading::IStrategy* createStrategy(const char* name, ILogger* logger, IConfigureManager* configureManager,
        IMarketDataProvider* dataProvider, ITimerProvider* timerProvider, IOrderManger* orderManager, IPositionManager* posMgr)
    {
        return new BluesTrading::SingleStrategy(name, logger, configureManager, dataProvider, timerProvider, orderManager, posMgr);
    }
};



namespace BluesTrading
{
    SingleStrategy::SingleStrategy(const char* name, ILogger* logger, IConfigureManager* configureManager, 
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

    SingleStrategy::~SingleStrategy()
    {

    }

    void SingleStrategy::onTimer(uint32_t eventID, uint32_t currentTime)
    {
        //std::cout << "EventID:" << eventID << " Time:" << currentTime << std::endl;
        if(eventID == 1)
        {
            timerprovider_->setTimer(this, 2, 500, true); // target time 
		
        }
        else if(eventID == 2)
        {
			updateMA();
        }
    }

    void SingleStrategy::onStartDay(uint32_t date)
    {

        //PositionItem yst_1_pos {2200, 1000, true,false};
        //positionManager_->getPosition(1).addPosition(yst_1_pos);


		//positionManager_->getAccountInfo().cash = 100000;

        std::cout << "start day " << date << "\n";

        //TODO order Mask
        orderManager_->subscribeOrderUpdate(0,this); 

        dataprovider_->subscribeInstrument(1, this);
        dataprovider_->subscribeInstrument(2, this);
        uint32_t now = timerprovider_->getCurrentTimeMsInDay();
        uint32_t targetTime = 9 * 3600 * 1000 + 0 * 60 * 1000 - 500;     // 09:00:00

        timerprovider_->setTimer(this, 1, targetTime - now, false); // target time 
        //  timerprovider_->setTimer(this, 2, 60000, true);
		for(int i = 0 ; i != num_of_instr_; ++ i)
		{
			SingleStrategy::initInstrument(pInstruments_[i]);
		}
    }
	void SingleStrategy::initInstrument(Instrument &p)
	{
		p.mid_price_ = 0;
		p.self_bp_ = 0;
		if(!globalParas_.is_ma_overnight_)
		{
			p.MA_ = -1;
			p.MA_by10K_ = 0;
		}
		//continuous
		if( !globalParas_.is_over_night_ )
		{
			p.last_px_ = 0;
			p.longOpenPositionQty_ = 0;
			p.shortOpenPositionQty_ = 0;
			p.cum_long_price_ = 0;
			p.cum_short_price_ = 0;
			p.opp_counter_ = 0;
			p.opp_counter_self_ = 0;
		}
		p.pnl_ = 0;
		p.net_pnl_ = 0;
		p.num_trades_ = 0;
		p.stoploss_ = false;
		//-----------------------
		p.vol_ = 0;
		p.vol_MA_ = 0;
		p.ask_price_ = 0;
		p.bid_price_ = 0;
		p.pre_ask_size_ = 0;
		p.pre_bid_size_ = 0;
		p.pre_ask_opp_size_ = 0;
		p.pre_bid_opp_size_ = 0;
		p.pre_trade_timeInMS_ = 0;
		p.pre_opp_timeInMS_ = 0;
		p.pre_opp_exit_timeInMS_ = 0;
		p.volume_ = 0;
		p.pre_volume_ = 0;
		p.avg_size_ = 0;
		p.ask_size_ = 0;
		p.bid_size_ = 0;	
		p.num_cancel_ = 0;
		p.stop_enter = false;
		p.can_long_ = true;
		p.can_short_ = true;
		p.slip_ticks_ = 0;
		//volume
		p.volume_ = -1;
		p.pre_volume_ = -1;
		p.vol_ = -1;
		p.vol_MA_ = -1;
		p.log_vol_divide_MA_ = 0;

		p.volatility_ = 0;
		p.numTickUpdates_ = 0;
		p.counter2_ = 0;
	}
    void SingleStrategy::onEndDay(uint32_t date)
    {
       
        //fmt % date %
		//std::string result = "xxx," + boost::lexical_cast<std::string>(count);


		for(int i = 0 ; i != num_of_instr_; ++ i)
		{

			     //   double getPositionPnl() const {return  getPositionPnl(lastPrice_);}
       
        //double getRealizedPnL() const {return realizedPnL_;}

        //void setUintMultiplier(uint32_t multiplier) {unitMultiplier_ = multiplier;}
			Instrument& instrument = pInstruments_[i];
			boost::format fmt("%1%,%2%,%3%,%4%,%5%,%6%");
			double pnl = positionManager_->getPosition( instrument.inst_index_).getPositionPnl();
			int trade = positionManager_->getPosition( instrument.inst_index_).getTradeCount();
			std::cout << "onEndDay 1 \n" ;
			fmt % instrument.inst_index_ %  instrument.num_trades_ % instrument.pnl_ % instrument.opp_counter_self_ % trade % pnl;
			std::string result = fmt.str();
			std::cout << "onEndDay 2 \n" ;
			        logger_->LogEOD("date,par1,para2,para3,tradenum,profit", result);
					std::cout << "onEndDay 3 \n" ;
		}

    }

	SingleStrategy::Instrument * SingleStrategy::getInstrumentLeg(uint32_t instIndex)
	{
		for(int i = 0; i != num_of_instr_; ++i)
		{
			if(pInstruments_[i].inst_index_ == instIndex)
				return &pInstruments_[i];
		}

		return nullptr;
	}

    void SingleStrategy::onMarketData(const CTickData& data)
    {
		//std::cout << "tick " << data.timeInMS  << "\n" ;
        static int count = 0;
        count ++ ;
		Instrument *pInst = getInstrumentLeg(data.instIndex);
		Instrument &instrument = *pInst;
		//checkInstrumentActive();
		if(!instrument.is_valid_)
			return;
		/*static const unsigned int TimeInMSAt0850 = getTimeInMS(8, 50, 0);
		static const unsigned int TimeInMSAt0900 = getTimeInMS(8, 59, 59);
		static const unsigned int TimeInMSAt2300 = getTimeInMS(2, 59, 59);
		static const unsigned int TimeInMSAt2330 = getTimeInMS(3, 29, 59);
		static const unsigned int TimeInMSAt0100 = getTimeInMS(4, 59, 59);
		static const unsigned int TimeInMSAt0230 = getTimeInMS(6, 29, 59);
		static const unsigned int TimeInMSAt2100 = getTimeInMS(0, 59, 59);*/

		/*if( this->trading_time_flag_ == LIQUIDATE && globalParas_.is_over_night_)
		{
			if( pInst->longOpenPositionQty_ > 0 || pInst->shortOpenPositionQty_ != 0)
				liquidate(*pInst, data);
		}*/

		if(data.getBidDepth(1).price == 0 || data.getAskDepth(1).price == 0 || data.getAskDepth(1).price <= data.getBidDepth(1).price || data.getBidDepth(1).price < 10 * pInst->tick_size_ || data.getAskDepth(1).price < 10 * pInst->tick_size_)
			return;

		//if(data.timeInMS >= TimeInMSAt2100)

        /*if (data.instIndex == 1)
        {
            lastprice_ = data.last_price;
        }*/
		
		pInst->mid_price_ = (data.getBidDepth(1).price + data.getAskDepth(1).price) / 2;
		if(pInst->MA_ <= 0)
		{
			pInst->MA_ = pInst->mid_price_;
			pInst->MA_by10K_ = pInst->MA_/10000;
			pInst->last_px_ = pInst->mid_price_;
			return;
		}
		if (globalParas_.is_ma_overnight_ && pInst->last_px_ < 1)
		{
			pInst->last_px_ = pInst->mid_price_;
			pInst->MA_by10K_ = pInst->MA_/10000;
			return;
		}
		const CTickData& pre_record =  pInst->pre_tick ? (*pInst->pre_tick) : data;
		pInst->pre_tick = & data;

		if(data.tot_vol < pre_record.tot_vol)
		{
			return;//////
		}
		if(abs(data.getBidDepth(1).price - pInst->MA_)/data.getBidDepth(1).price > 0.2 || abs(data.getAskDepth(1).price - pInst->MA_)/data.getAskDepth(1).price > 0.2 || abs(data.getAskDepth(1).price - data.getBidDepth(1).price)/data.getBidDepth(1).price > 0.1 || abs(data.last_price - data.getBidDepth(1).price)/data.getBidDepth(1).price)
		{
			return;
		}
		if( globalParas_.is_over_night_ && globalParas_.is_equity_ && (abs(data.getAskDepth(1).price - pre_record.getAskDepth(1).price)/data.getAskDepth(1).price > 0.1 || abs(data.getBidDepth(1).price - pre_record.getBidDepth(1).price)/data.getBidDepth(1).price > 0.1))
		{
			pInst->longOpenPositionQty_ = 0;
			pInst->shortOpenPositionQty_ = 0;
			pInst->cum_long_price_ = 0;
			pInst->cum_short_price_ = 0;
		}
		if(pInst->MA_by10K_ > 0)
		{
			pInst->self_bp_ = (pInst->mid_price_ - pInst->MA_) / pInst->MA_by10K_;
		}
		pInst->ask_price_ = data.getAskDepth(1).price;
		pInst->bid_price_ = data.getBidDepth(1).price;
		pInst->ask_size_ = data.getAskDepth(1).size;
		pInst->bid_size_ = data.getBidDepth(1).size;
		pInst->volume_ = data.tot_vol;

		//checkTradeTime();
		bool isValidOppTime;
		bool isValidEnterTime;
		bool isValidExitTime;
		bool isLiquidateTime;
		isValidOppTime = (timerprovider_->getCurrentTimeMsInDay() > globalParas_.opp_start_time_ && timerprovider_->getCurrentTimeMsInDay() < globalParas_.pee_time1_)
			|| (timerprovider_->getCurrentTimeMsInDay() > globalParas_.pee_time_end1_ && timerprovider_->getCurrentTimeMsInDay() < globalParas_.pee_time2_)
			|| (timerprovider_->getCurrentTimeMsInDay() > globalParas_.pee_time_end2_ && timerprovider_->getCurrentTimeMsInDay() < globalParas_.liquidate_time_);
		isValidEnterTime = (timerprovider_->getCurrentTimeMsInDay() > globalParas_.opening_time_ && timerprovider_->getCurrentTimeMsInDay() < globalParas_.pee_time1_)
			|| (timerprovider_->getCurrentTimeMsInDay() > globalParas_.pee_time_end1_ && timerprovider_->getCurrentTimeMsInDay() < globalParas_.pee_time2_)
			|| (timerprovider_->getCurrentTimeMsInDay() > globalParas_.pee_time_end2_ && timerprovider_->getCurrentTimeMsInDay() < globalParas_.exit_time_);
		isValidExitTime = (timerprovider_->getCurrentTimeMsInDay() > globalParas_.opening_time_ && timerprovider_->getCurrentTimeMsInDay() < globalParas_.pee_time1_)
			|| (timerprovider_->getCurrentTimeMsInDay() > globalParas_.pee_time_end1_ && timerprovider_->getCurrentTimeMsInDay() < globalParas_.pee_time2_)
			|| (timerprovider_->getCurrentTimeMsInDay() > globalParas_.pee_time_end2_ && timerprovider_->getCurrentTimeMsInDay() < globalParas_.liquidate_time_);
		isLiquidateTime = (timerprovider_->getCurrentTimeMsInDay() > globalParas_.liquidate_time_ /*&& timerprovider_->getCurrentTimeMsInDay() <= '15:00:00'*/);
		if(!isValidOppTime && !isValidEnterTime && !isLiquidateTime && !isValidExitTime)
			return;
		//updateMA();
		if(isValidOppTime)
			updateOpp(data, globalParas_.trading_interval_in_MSec_);
		if(isValidEnterTime)
			doEnter(data, globalParas_.trading_interval_in_MSec_);
		if(isValidExitTime)
			doExit(data, globalParas_.trading_interval_in_MSec_);
		if(isLiquidateTime)
			liquidate(data, globalParas_.trading_interval_in_MSec_);
    }

    void SingleStrategy::onUpdateOrder(OrderDataDetail* orderData)
    {
       // printOrder(std::cout, *orderData);
		std::cout <<orderData->orderID <<  " orderStatus " << orderData->cffex_order.common.orderStatus   << " ErrorCode " << orderData->cffex_order.common.orderErrorCode 
			<< " Price " << orderData->cffex_order.price << "\n";
    }
	
	void SingleStrategy::doEnter(const CTickData &data, uint32_t TradingIntvlInMS)
	{
		Instrument *pInst = getInstrumentLeg(data.instIndex);
		Instrument &instrument = *pInst;
		bool isReadyToTrade = instrument.isReadyToTrade(timerprovider_->getCurrentTimeMsInDay(), globalParas_.trading_interval_in_MSec_);
		if(instrument.stoploss_)
			return;
		else
		{
			//short positionManager_.getPosition(data.instIndex).getPositions(LongToday)
			if(instrument.shortOpenPositionQty_ < instrument.max_pos_ && instrument.longOpenPositionQty_ == 0 && instrument.can_short_)
			{
				if(instrument.self_bp_ >= 0 
					&& instrument.opp_counter_self_ <= -globalParas_.threshold_self_ 
					&& (globalParas_.is_using_size_contrast_? instrument.ask_size_ >= instrument.bid_size_ : true)
					)
				{
					//SecurityNewOrderRequest request;
					SHFE_NewOrderRequest request;
					request.instrumentID = data.instIndex;
					request.longshortflag = LongShortFlag_Short;
					request.orderqty = int(pInst->ppl_ + 0.5);
					request.opencloseflag = OpenCloseFlag_Open;
					request.price = instrument.bid_price_;
					//request.priceType = 0; // only limit order
					submitRequest(request, orderManager_);
					instrument.pre_trade_timeInMS_ = timerprovider_->getCurrentTimeMsInDay();
					instrument.shortOpenPositionQty_ += request.orderqty;
					instrument.num_trades_ += request.orderqty;
					instrument.cum_short_price_ += request.price * request.orderqty;
					std::cout<<instrument.instrument_<<",enter short,"<<request.price<<std::endl;
				}
			}
			//long
			else if(instrument.longOpenPositionQty_ < instrument.max_pos_ && instrument.shortOpenPositionQty_ == 0 && instrument.can_long_)
			{
				if(instrument.self_bp_ <= 0 
					&& instrument.opp_counter_self_ >= globalParas_.threshold_self_ 
					&& (globalParas_.is_using_size_contrast_? instrument.bid_size_ >= instrument.ask_size_ : true)
					)
				{
					SHFE_NewOrderRequest request;
					request.instrumentID = data.instIndex;
					request.longshortflag = LongShortFlag_Long;
					request.orderqty = int(pInst->ppl_ + 0.5);
					request.opencloseflag = OpenCloseFlag_Open;
					request.price = instrument.ask_price_;
					//request.priceType = 0; // only limit order
					submitRequest(request, orderManager_);
					instrument.pre_trade_timeInMS_ = timerprovider_->getCurrentTimeMsInDay();
					instrument.longOpenPositionQty_ += request.orderqty;
					instrument.num_trades_ += request.orderqty;
					instrument.cum_long_price_ += request.price * request.orderqty;
					std::cout<<instrument.instrument_<<",enter long,"<<request.price<<std::endl;
				}
			}
		}
	}

	void SingleStrategy::doExit(const CTickData &data, uint32_t TradingIntvlInMS)
	{
		Instrument *pInst = getInstrumentLeg(data.instIndex);
		Instrument &instrument = *pInst;
		bool isReadyToTrade = instrument.isReadyToTrade(timerprovider_->getCurrentTimeMsInDay(), globalParas_.trading_interval_in_MSec_);
		if(instrument.longOpenPositionQty_ != 0 && (instrument.stoploss_ || (instrument.self_bp_ >= 0 && instrument.opp_counter_self_ <= 0 
			&& (globalParas_.is_using_size_contrast_? instrument.ask_size_ >= instrument.bid_size_ : true))))
		{
			SHFE_NewOrderRequest request;
			request.instrumentID = data.instIndex;
			request.longshortflag = LongShortFlag_Short;
			request.orderqty = int(std::min(instrument.ppl_ + 0.5, double(instrument.longOpenPositionQty_)));
			request.opencloseflag = OpenCloseFlag_Close;
			request.price = instrument.bid_price_;
			//request.priceType = 0; // only limit order
			submitRequest(request, orderManager_);
			calculatePnl( true, request.orderqty, request.price,instrument );
			instrument.pre_trade_timeInMS_ = timerprovider_->getCurrentTimeMsInDay();
			instrument.longOpenPositionQty_ -= request.orderqty;
			std::cout<<instrument.instrument_<<",exit long,"<<request.price<<std::endl;
		}
		else if(instrument.shortOpenPositionQty_ != 0 && (instrument.stoploss_ || (instrument.self_bp_ <= 0 && instrument.opp_counter_self_ >= 0 
			&& (globalParas_.is_using_size_contrast_? instrument.bid_size_ >= instrument.ask_size_ : true))))
		{
			SHFE_NewOrderRequest request;
			request.instrumentID = data.instIndex;
			request.longshortflag = LongShortFlag_Long;
			request.orderqty = int(std::min(instrument.ppl_ + 0.5, double(instrument.shortOpenPositionQty_)));
			request.opencloseflag = OpenCloseFlag_Close;
			request.price = instrument.ask_price_;
			//request.priceType = 0; // only limit order
			submitRequest(request, orderManager_);
			calculatePnl( false, request.orderqty, request.price,instrument );
			instrument.pre_trade_timeInMS_ = timerprovider_->getCurrentTimeMsInDay();
			instrument.shortOpenPositionQty_ -= request.orderqty;
			std::cout<<instrument.instrument_<<",exit short,"<<request.price<<std::endl;
		}
	}

	void SingleStrategy::liquidate(const CTickData &data, uint32_t TradingIntvlInMS)
	{
		Instrument *pInst = getInstrumentLeg(data.instIndex);
		Instrument &instrument = *pInst;
		bool isReadyToTrade = instrument.isReadyToTrade(timerprovider_->getCurrentTimeMsInDay(), globalParas_.trading_interval_in_MSec_);
		if(instrument.longOpenPositionQty_ != 0 )
		{
			SHFE_NewOrderRequest request;
			request.instrumentID = data.instIndex;
			request.longshortflag = LongShortFlag_Short;
			request.orderqty = int(std::min(instrument.ppl_ + 0.5, double(instrument.longOpenPositionQty_)));
			request.opencloseflag = OpenCloseFlag_Close;
			request.price = instrument.bid_price_;
			//request.priceType = 0; // only limit order
			submitRequest(request, orderManager_);
			calculatePnl( true, request.orderqty, request.price,instrument );
			instrument.pre_trade_timeInMS_ = timerprovider_->getCurrentTimeMsInDay();
			instrument.longOpenPositionQty_ -= request.orderqty;
			std::cout<<instrument.instrument_<<",liq long,"<<request.price<<std::endl;
		}
		else if(instrument.shortOpenPositionQty_ != 0 )
		{
			SHFE_NewOrderRequest request;
			request.instrumentID = data.instIndex;
			request.longshortflag = LongShortFlag_Long;
			request.orderqty = int(std::min(instrument.ppl_ + 0.5, double(instrument.shortOpenPositionQty_)));
			request.opencloseflag = OpenCloseFlag_Close;
			request.price = instrument.ask_price_;
			//request.priceType = 0; // only limit order
			submitRequest(request, orderManager_);
			calculatePnl( false, request.orderqty, request.price,instrument );
			instrument.pre_trade_timeInMS_ = timerprovider_->getCurrentTimeMsInDay();
			instrument.shortOpenPositionQty_ -= request.orderqty;
			std::cout<<instrument.instrument_<<",liq short,"<<request.price<<std::endl;
		}
	}

	void SingleStrategy::calculatePnl( bool isExitLongPosition, int exitSize, double exitPx, Instrument &instrument )
	{
		double pnl = 0;
		if( isExitLongPosition && exitSize > 0 )
		{
			double avgPx = instrument.cum_long_price_/instrument.longOpenPositionQty_;
			pnl = exitSize * (exitPx - avgPx )/instrument.tick_size_*instrument.tick_price_;
			std::cout<<"calcPNL:"<<instrument.tick_size_<<","<<instrument.tick_price_<<","<<instrument.cum_long_price_<<","<<instrument.longOpenPositionQty_<<","<<avgPx
				<<","<<exitPx<<","<<exitSize<<","<<pnl<<std::endl;
			instrument.pnl_ += pnl;
			if(globalParas_.is_equity_)
				instrument.net_pnl_ += pnl - exitSize * instrument.mid_price_ * 1.8/1000;
			else
				instrument.net_pnl_ += pnl - exitSize * instrument.commission_;
			instrument.cum_long_price_ -= exitSize * avgPx;
		}
		else if( !isExitLongPosition && exitSize > 0 )
		{
			double avgPx = instrument.cum_short_price_/instrument.shortOpenPositionQty_;
			pnl = exitSize * (avgPx - exitPx )/instrument.tick_size_*instrument.tick_price_;
			std::cout<<"calcPNL:"<<instrument.tick_size_<<","<<instrument.tick_price_<<","<<instrument.cum_short_price_<<","<<instrument.shortOpenPositionQty_<<","<<avgPx
				<<","<<exitPx<<","<<exitSize<<","<<pnl<<std::endl;
			instrument.pnl_ += pnl;
			if(globalParas_.is_equity_)
				instrument.net_pnl_ += pnl - exitSize * instrument.mid_price_ * 1.8/1000;
			else
				instrument.net_pnl_ += pnl - exitSize * instrument.commission_;
			instrument.cum_short_price_ -= exitSize * avgPx;
		}
	}

	void SingleStrategy::updateOpp(const CTickData &data, uint32_t TradingIntvlInMS)
	{
		Instrument *pInst = getInstrumentLeg(data.instIndex);
		Instrument &instrument = *pInst;
		bool isReadyOpp = instrument.isReadyToOpp(timerprovider_->getCurrentTimeMsInDay(), globalParas_.trading_interval_in_MSec_);
		//short
		if( instrument.self_bp_ < std::min(instrument.shape_factor_ * instrument.log_vol_divide_MA_ - instrument.enter_sig_*std::max(1.0,std::pow(instrument.volatility_MA_,instrument.volMultiplier_)), -instrument.enter_protection_)
			&& instrument.log_vol_divide_MA_ > instrument.volume_protection_
			&& (globalParas_.is_checking_size_before_trade_?instrument.bid_size_ != instrument.pre_bid_opp_size_:1)
			&& isReadyOpp)
		{
		
			if(instrument.longOpenPositionQty_ < 0.5 && instrument.shortOpenPositionQty_ < 0.5 && instrument.opp_counter_self_ > -globalParas_.threshold_self_)
			{
				instrument.opp_counter_self_ -= 1;
				instrument.pre_bid_opp_size_ = instrument.bid_size_;
				instrument.pre_opp_timeInMS_ = timerprovider_->getCurrentTimeMsInDay();
			}
			else if(instrument.longOpenPositionQty_ > 0.5 && instrument.opp_counter_self_ > 0)
			{
				instrument.opp_counter_self_ -= 1;
				instrument.pre_bid_opp_size_ = instrument.bid_size_;
				instrument.pre_opp_timeInMS_ = timerprovider_->getCurrentTimeMsInDay();
			}
			else if(instrument.shortOpenPositionQty_ > 0.5 && instrument.opp_counter_self_ > -globalParas_.threshold_self_)
			{
				instrument.opp_counter_self_ -= 1;
				instrument.pre_bid_opp_size_ = instrument.bid_size_;
				instrument.pre_opp_timeInMS_ = timerprovider_->getCurrentTimeMsInDay();
			}
		}
		//long
		else if( instrument.self_bp_ > std::min(instrument.shape_factor_ * instrument.log_vol_divide_MA_ + instrument.enter_sig_*std::max(1.0,std::pow(instrument.volatility_MA_,instrument.volMultiplier_)), instrument.enter_protection_)
			&& instrument.log_vol_divide_MA_ > instrument.volume_protection_
			&& (globalParas_.is_checking_size_before_trade_?instrument.ask_size_ != instrument.pre_ask_opp_size_:1)
			&& isReadyOpp)
		{
		
			if(instrument.shortOpenPositionQty_ < 0.5 && instrument.longOpenPositionQty_ < 0.5 && instrument.opp_counter_self_ < globalParas_.threshold_self_)
			{
				instrument.opp_counter_self_ += 1;
				instrument.pre_ask_opp_size_ = instrument.ask_size_;
				instrument.pre_opp_timeInMS_ = timerprovider_->getCurrentTimeMsInDay();
			}
			else if(instrument.shortOpenPositionQty_ > 0.5 && instrument.opp_counter_self_ < 0)
			{
				instrument.opp_counter_self_ += 1;
				instrument.pre_ask_opp_size_ = instrument.ask_size_;
				instrument.pre_opp_timeInMS_ = timerprovider_->getCurrentTimeMsInDay();
			}
			else if(instrument.longOpenPositionQty_ > 0.5 && instrument.opp_counter_self_ < globalParas_.threshold_self_)
			{
				instrument.opp_counter_self_ += 1;
				instrument.pre_ask_opp_size_ = instrument.ask_size_;
				instrument.pre_opp_timeInMS_ = timerprovider_->getCurrentTimeMsInDay();
			}
		}
		/*std::cout<<instrument.instrument_<<",bp:"<<instrument.self_bp_<<",ma:"<<instrument.MA_<<",mid:"<<instrument.mid_price_
			<<",enter:"<<instrument.enter_sig_<<",opp:"<<instrument.opp_counter_self_<<",lp:"<<instrument.longOpenPositionQty_<<",sp:"<<instrument.shortOpenPositionQty_
			<<",cl:"<<instrument.can_long_<<",cs:"<<instrument.can_short_<<",ppl:"<<instrument.ppl_<<",maxpos:"<<instrument.max_pos_<<std::endl;*/
	}

	void SingleStrategy::updateMA()
	{
		for(int curInstID = 0; curInstID < num_of_instr_; ++curInstID)
		{
			//Instrument *pInst =;///////
			Instrument &instrument =  pInstruments_[curInstID];
			if(!instrument.is_valid_)
				continue;
			if ( instrument.MA_ > 0 && instrument.mid_price_ > 0)
			{
				instrument.MA_ = instrument.MA_ * (1 - globalParas_.MA_weight_) + instrument.mid_price_ * globalParas_.MA_weight_;
				instrument.MA_by10K_ = instrument.MA_ / 10000;
				
				instrument.self_bp_ = ( instrument.mid_price_ - instrument.MA_ ) / instrument.MA_by10K_;
				if(instrument.last_px_ > 0 && globalParas_.counter > 100)
				{
					instrument.volatility_ += abs(instrument.last_px_ - instrument.mid_price_)/instrument.MA_by10K_;
					instrument.last_px_ = instrument.mid_price_;
				}
			}
			//cout<<instrument.instrument_<<","<<data.time_.hour<<"-"<<data.time_.minute<<","<<data.time_.second<<" "<<instrument.is_valid_<<endl;
			globalParas_.total_pos_ += (instrument.shortOpenPositionQty_ + instrument.longOpenPositionQty_);
			//volume
			if(instrument.pre_volume_ > 0 )
			{
				instrument.vol_ = instrument.volume_ - instrument.pre_volume_;
			}
			instrument.pre_volume_ = instrument.volume_;
			if( instrument.vol_MA_ < 0 && instrument.vol_ > 0)
			{
				instrument.vol_MA_ = 50;
			}
			else
			{
				instrument.vol_MA_ = instrument.vol_MA_ * (1 - 0.01) + instrument.vol_ * 0.01;
				instrument.log_vol_divide_MA_ = std::log(instrument.vol_ / std::max(1.0,instrument.vol_MA_) + 1);
			}
		}	
		if(globalParas_.counter > 100)
		{
			globalParas_.counter = 0;
			for(int curInstID = 0; curInstID < num_of_instr_; ++curInstID)
			{
				Instrument &instrument = pInstruments_[curInstID];
				if(instrument.is_valid_ && instrument.volume_ > 0)
				{
					instrument.counter2_ ++ ;
				}
			}
		}
		else
		{
			globalParas_.counter ++;
		}

		for(int curInstID = 0; curInstID < num_of_instr_; ++curInstID)
		{
			//Instrument *pInst = getInstrumentLeg(curInstID);///////
			Instrument &instrument = pInstruments_[curInstID];
			if(!instrument.is_valid_)
				continue;
			if(instrument.volatility_ > 0)
			{
				instrument.volatility_MA_ = instrument.volatility_MA_ * (1 - globalParas_.volMA_weight_) + instrument.volatility_/std::max(1.0,instrument.counter2_*1.0) * globalParas_.volMA_weight_;
			}
		}
	}

    void SingleStrategy::onMessage(const std::string& propName)
    {
        StrategyConfig msg;
        msg.ParseFromString(propName);

        for (auto& prop_pair: msg.props())
        {
            const std::string& propName = prop_pair.propname();
            const std::string& propValue = prop_pair.value();

            if(propName == "OppStartTime")
            {
				globalParas_.opp_start_time_ = getTime(propValue);
            }
            else if (propName == "OpeningTime")
            {
				globalParas_.opening_time_ = getTime(propValue);
            }
            else if (propName == "ExitTime")
            {
				globalParas_.exit_time_ = getTime(propValue);
            }
            else if (propName == "LiquidateTime")
            {
				globalParas_.liquidate_time_ = getTime(propValue);
            }
            else if (propName == "PeeTime1")
            {
				globalParas_.pee_time1_ = getTime(propValue);
            }
            else if (propName == "PeeTimeEnd1")
            {
				globalParas_.pee_time_end1_ = getTime(propValue);
            }
			else if (propName == "PeeTime2")
			{
				globalParas_.pee_time2_ = getTime(propValue);
			}
			else if (propName == "PeeTimeEnd2")
			{
				globalParas_.pee_time_end2_ = getTime(propValue);
			}
			else if (propName == "MAWeight")
			{
				convert(propValue, globalParas_.MA_weight_);
			}
			else if (propName == "VolMAWeight")
			{
				convert(propValue, globalParas_.volMA_weight_);
			}
			else if (propName == "UpdateMAInterval")
			{
				convert(propValue, globalParas_.updateMAInterval_);
			}
			else if (propName == "CapRatio")
			{
				convert(propValue, globalParas_.cap_ratio_);
			}
			else if (propName == "IsCheckingSizeBeforeTrade")
			{
				convert(propValue, globalParas_.is_checking_size_before_trade_);
			}
			else if (propName == "IsOverNight")
			{
				convert(propValue, globalParas_.is_over_night_);
			}
			else if (propName == "IsMAOverNight")
			{
				convert(propValue, globalParas_.is_ma_overnight_);
			}
			else if (propName == "IsEquity")
			{
				convert(propValue, globalParas_.is_equity_);
			}
			else if (propName == "IsOneSide")
			{
				convert(propValue, globalParas_.is_one_side_);
			}
			else if (propName == "IsZeroOppExit")
			{
				convert(propValue, globalParas_.is_zero_opp_exit_);
			}
			else if (propName == "IsStopBP")
			{
				convert(propValue, globalParas_.is_stop_bp_);
			}
			else if (propName == "IsUsingSizeContrast")
			{
				convert(propValue, globalParas_.is_using_size_contrast_);
			}
			else if (propName == "IsOppBeforeDoEnter")
			{
				convert(propValue, globalParas_.is_opp_before_doenter_);
			}
			else if (propName == "TotalPos")
			{
				convert(propValue, globalParas_.total_pos_);
			}
			else if (propName == "Counter")
			{
				convert(propValue, globalParas_.counter);
			}
			else if (propName == "IsVolAdj")
			{
				convert(propValue, globalParas_.is_vol_adj_);
			}
			else if (propName == "Vol")
			{
				convert(propValue, globalParas_.vol);
			}
			else if (propName == "VolSum")
			{
				convert(propValue, globalParas_.volSum);
			}
			else if (propName == "VolMA")
			{
				convert(propValue, globalParas_.volMA);
			}
			else if (propName == "AllInstrumentActive")
			{
				convert(propValue, globalParas_.all_instruments_active_);
			}
			else if (propName == "ThresholdSelf")
			{
				convert(propValue, globalParas_.threshold_self_);
			}
			else if (propName == "ThresholdSelfExit")
			{
				convert(propValue, globalParas_.threshold_self_exit_);
			}
			else if (propName == "StopBP")
			{
				convert(propValue, globalParas_.stop_bp_);
			}
			else if (propName == "BackThreshold")
			{
				convert(propValue, globalParas_.back_threshold_);
			}
			else if (propName == "TradingIntervalInMsec")
			{
				convert(propValue, globalParas_.trading_interval_in_MSec_);
			}
			else if (propName == "ADDInstrument")
            {
				std::vector<std::string> legs;
				boost::split(legs, propValue, boost::is_any_of("#"));
				AddInstrument(legs);
		
            }
			else if (propName == "Weights")
            {
				std::vector<std::string> legs;
				boost::split(legs, propValue, boost::is_any_of("#"));
				for(auto& inst : legs)
				{
					std::vector<std::string> leg_value;
					boost::split(leg_value, inst, boost::is_any_of(":"));

					SetWeights(leg_value[0], leg_value[1]);
				}
            }
            else
            {
                std::cout << "ERR UNKNOWN PROPNAME " <<  propName << std::endl;
            }
		}
	}


	void	SingleStrategy::AddInstrument( std::vector<std::string> insts)
	{
		num_of_instr_ = insts.size();
		pInstruments_ = new Instrument[num_of_instr_];

		for( int i = 0; i < num_of_instr_; ++i )
		{
			Instrument& p = pInstruments_[i];

			strcpy(p.instrument_, insts[i].c_str());
			p.inst_index_ = getInstrumentIndex(insts[i]);
			p.pre_settlement_ = 0;
			p.weight_ = 1;
			p.ppl_ = 1;
			p.max_pos_ = 1;
			p.volMultiplier_ = 0;
			p.enter_sig_ = 1;
			p.enter_protection_ = 0;
			p.shape_factor_ = 0.5;
			p.tick_size_ = 1;
			p.tick_price_ = 15;
			p.commission_ = 5;
			p.limit_ = 0.05;
		}
	}
	void	SingleStrategy::SetWeights(const std::string& leg, const std::string& value)
	{
	}

}

