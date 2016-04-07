#pragma once
#include "IStrategy.h"

extern "C"
{
    using namespace BluesTrading;
    IStrategy* createStrategy(const char* name, ILogger* logger, 
        IConfigureManager* configureManager, IMarketDataProvider* dataProvider, ITimerProvider* timerProvider, IOrderManger* orderManager,
        IPositionManager* posMgr);
};


namespace BluesTrading
{

    class SingleStrategy : public IStrategy
    {
    public:
        SingleStrategy(const char* name,  ILogger* logger, IConfigureManager* configureManager,
            IMarketDataProvider* dataProvider, ITimerProvider* timerProvider, IOrderManger* orderManager, IPositionManager* position);

        virtual ~SingleStrategy();

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
            uint32_t	opp_start_time_;
			uint32_t	opening_time_;
			uint32_t	exit_time_;
			uint32_t	liquidate_time_;
			uint32_t	pee_time1_;
			uint32_t	pee_time_end1_;
			uint32_t	pee_time2_;
			uint32_t	pee_time_end2_;
			double	MA_weight_;
			double	volMA_weight_;
			double	updateMAInterval_;
			double	cap_ratio_;
			bool	is_checking_size_before_trade_;
			bool	is_over_night_;
			bool	is_ma_overnight_;
			bool	is_equity_;
			bool	is_one_side_;
			bool	is_zero_opp_exit_;
			bool	is_stop_bp_;
			bool	Is_Cap_Control_;
			bool	is_using_size_contrast_;
			bool	is_opp_before_doenter_;
			double	total_pos_;
			int		counter;
			bool	is_vol_adj_;
			double	vol;
			double	volSum;
			double	volMA;
			bool	all_instruments_active_;
			int		threshold_self_;
			int		threshold_self_exit_;
			double	stop_bp_;
			int		back_threshold_;
			int		trading_interval_in_MSec_;
        };

        strategyParameters globalParas_;

		struct Instrument
		{
			//self defined
			char		instrument_[64];
			double		pre_settlement_;
			double		weight_;
			double		ppl_;
			double		max_pos_;
			double      volMultiplier_;
			double		enter_sig_;
			double		enter_protection_;
			double		shape_factor_;
			double		shape_factor1_;
			double		volume_protection_;
			double		limit_;

			//read from db
			double		tick_size_;
			double		tick_price_;
			double		commission_;

			//used to remember last index;
			const CTickData*			pre_tick;
            CTickData tickFromLastDay;
			int			numTickUpdates_;

			//real time vars
			double	mid_price_;
			double	self_bp_;
			double	MA_;
			double	vma_;
			int counter2_;
			double	MA_by10K_;
			int	longOpenPositionQty_;
			int	shortOpenPositionQty_;
			double cum_long_price_;
			double cum_short_price_;

			double bid_price_;
			double ask_price_;
			int	bid_size_;
			int	ask_size_;
			int pre_bid_size_;
			int pre_ask_size_;
			double pre_bid_price_;
			double pre_ask_price_;
			unsigned long long pre_trade_timeInMS_;
			unsigned long long pre_opp_timeInMS_;
			unsigned long long pre_opp_exit_timeInMS_;
			int pre_bid_opp_size_;
			int pre_ask_opp_size_;

			double volatility_;
			double volatility_MA_;

			int volume_;
			int pre_volume_;
			int total_volume_;
			int pre_total_volume_;
			double avg_size_;
			int vol_;
			double vol_MA_;
			double log_vol_divide_MA_;
			double pnl_;
			double net_pnl_;
			int		num_trades_;
			
			double  num_cancel_;
			bool stop_enter;
			bool isTraded;
			bool can_long_;
			bool can_short_;
			double slip_ticks_;
			int opp_counter_;
			int opp_counter_self_;
			bool stoploss_;
			double last_px_;
			bool is_valid_;
			uint32_t inst_index_;
			Instrument()
			{

				inst_index_ = 0;
				pre_settlement_ = 0;
				weight_ = 0;
				ppl_ = 1;
				max_pos_ = 1;
				volMultiplier_ = 0;
				enter_sig_ = 1;
				enter_protection_ = 0;
				shape_factor_ = 1;
				shape_factor1_ = 1;
				volume_protection_ = 0.5;
				limit_ = 0.06;

				//read from db
				tick_size_ = 0.01;
				tick_price_ = 1;
				commission_ = 1;

				//used to remember last index = ;
				pre_tick = nullptr;
				numTickUpdates_ = 1;

				//real time vars
				mid_price_ = 0;
				self_bp_ = 0;
				MA_ = -1;
				vma_ = 0;
				counter2_ = 0;
				MA_by10K_ = -1;
				longOpenPositionQty_ = 0;
				shortOpenPositionQty_ = 0;
				cum_long_price_ = 0;
				cum_short_price_ = 0;

				bid_price_ = 0;
				ask_price_ = 0;
				bid_size_ = 0;
				ask_size_ = 0;
				pre_bid_size_ = 0;
				pre_ask_size_ = 0;
				pre_bid_price_ = 0;
				pre_ask_price_ = 0;
				pre_trade_timeInMS_ = 0;
				pre_opp_timeInMS_ = 0;
				pre_opp_exit_timeInMS_ = 0;
				pre_bid_opp_size_ = 0;
				pre_ask_opp_size_ = 0;

				volatility_ = 5;
				volatility_MA_ = 5;

				volume_ = 0;
				pre_volume_ = 0;
				total_volume_ = 0;
				pre_total_volume_ = 0;
				avg_size_ = 0;
				vol_ = 5;
				vol_MA_ = 5;
				log_vol_divide_MA_ = 1;
				pnl_ = 0;
				net_pnl_ = 0;
				num_trades_ = 0;
			
				num_cancel_ = 0;
				stop_enter = 0;
				isTraded = 0;
				can_long_ = true;
				can_short_ = true;
				slip_ticks_ = 0;
				opp_counter_ = 0;
				opp_counter_self_ = 0;
				stoploss_ = false;
				last_px_ = 0;
				is_valid_ = true;
			}
			bool isReadyToTrade(uint32_t curTimeInMS, uint32_t minTimeInMS)
			{
				return (curTimeInMS >= pre_trade_timeInMS_ + minTimeInMS || curTimeInMS <= pre_trade_timeInMS_ + minTimeInMS - 100000);
			}
			bool isReadyToOpp(uint32_t curTimeInMS, uint32_t minTimeInMS)
			{
				return (curTimeInMS >= pre_opp_timeInMS_ + minTimeInMS || curTimeInMS <= pre_opp_timeInMS_ + minTimeInMS - 100000);
			}
		};

        Instrument instrument;

	protected:

		void	doEnter(const CTickData &data, uint32_t TradingIntvlInMS);
		void	doExit(const CTickData &data, uint32_t TradingIntvlInMS);
		void	updateOpp(const CTickData &data, uint32_t TradingIntvlInMS);
		void	liquidate(const CTickData &data, uint32_t TradingIntvlInMS);
		void	calculatePnl( bool isExitLongPosition, int exitSize, double exit_px, Instrument &param );
		void	initInstrument(Instrument &p);
		void	updateMA();
		void	AddInstrument( std::vector<std::string>);
		void	SetWeights(const std::string& leg, const std::string& value);
		Instrument*	getInstrumentLeg(uint32_t instIndex);
;
		enum TIME_FLAG{
			DO_ENTER = 0,
			DO_EXIT,
			LIQUIDATE,
			NO_ACTION,
			PEE_TIME,
			ONLYOPP_TIME
		};
		Instrument* pInstruments_;	//product map
		int	 num_of_instr_;	
		TIME_FLAG  trading_time_flag_;
    };
}