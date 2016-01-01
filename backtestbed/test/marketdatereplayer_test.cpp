#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <iostream>
#include "../marketdatareplayer.h"
#include "../marketdatastore.h"
namespace BluesTrading
{
    using ::testing::Invoke;
    using ::testing::InvokeWithoutArgs;
    using ::testing::An;
    using ::testing::_;
    using ::testing::Field;
    using ::testing::Sequence;



    class FakeTickDataConsumer :public ITickDataConsumer
    {
    public:
        FakeTickDataConsumer() :current_time(0){}
        virtual void onMarketData(const TickDataLevel1& tick)
        {
            EXPECT_EQ(1,tick.depthsNum);
            EXPECT_LE(current_time, tick.timeInMS);
            current_time = tick.timeInMS;
        };
        virtual void onMarketData(const TickDataLevel5& tick)
        {
            EXPECT_EQ(5,tick.depthsNum);
            EXPECT_LE(current_time, tick.timeInMS);
            current_time = tick.timeInMS;
        };
        virtual void onMarketData(const TickDataLevel10& tick)
        {
            EXPECT_EQ(10,tick.depthsNum);
            EXPECT_LE(current_time, tick.timeInMS);
            current_time = tick.timeInMS;
        };
        virtual void onMarketData(const TickDataLevel20& tick)
        {
            EXPECT_EQ(20,tick.depthsNum);
            EXPECT_LE(current_time, tick.timeInMS);
            current_time = tick.timeInMS;
        };
        virtual void onOtherLevelsMarketData(const CTickData<1>& tick) {std::cout << "On Depth " << tick.depthsNum << "\n";};// for any other Levels.

        uint32_t current_time;
    };
    class MockTickDataConsumer : public ITickDataConsumer
    {
    public:

       template<int N>
       void registerDelegate()
       {
           typedef  void  (FakeTickDataConsumer::* tickFun)(const CTickData<N>&);
           tickFun tifun =  &FakeTickDataConsumer::onMarketData;

           ON_CALL(*this,     onMarketData(An<const CTickData<N>&>()))
               .WillByDefault(Invoke(&fake, tifun));
       }
        MockTickDataConsumer()
        {
            registerDelegate<1>();
            registerDelegate<5>();
            registerDelegate<10>();
            registerDelegate<20>();
            ON_CALL(*this,     onOtherLevelsMarketData(An<const CTickData<1>&>()))
                   .WillByDefault(Invoke(&fake, &FakeTickDataConsumer::onOtherLevelsMarketData));

            ON_CALL(*this, onEndDay(_)).WillByDefault(InvokeWithoutArgs(this, &MockTickDataConsumer::resetFakeTime));
        }


         MOCK_METHOD1(onMarketData, void(const TickDataLevel1&));
         MOCK_METHOD1(onMarketData, void(const TickDataLevel5&));
         MOCK_METHOD1(onMarketData, void(const TickDataLevel10&));
         MOCK_METHOD1(onMarketData, void(const TickDataLevel20&));
         MOCK_METHOD1(onOtherLevelsMarketData, void(const  CTickData<1>&));
         MOCK_METHOD1(onStartDay, void(uint32_t));
         MOCK_METHOD1(onEndDay, void(uint32_t));
         
         void resetFakeTime()
         {
             fake.current_time = 0;
         }
    private:
        FakeTickDataConsumer fake;
    };

    class MockTimerConsumer: public ITimerConsumer
    {
    public:
        MOCK_METHOD2(onTimer,  void(uint32_t eventID, uint32_t currentTime));
    };



    class marketdatareplayerTest : public ::testing::Test {
    protected:
        virtual void SetUp() 
        {
            replayer = NULL;

        }

        template<int N>
        void generateFakeTick(uint32_t inst, uint32_t date,  uint32_t timeInMS_offset = 0, uint32_t tick_num = 10)
        {
            MarketDataStore* pNewStore = new MarketDataStore{inst, date};
           
     
            CTickData<N>* pTickArray = new CTickData<N>[tick_num];

            allocateArray.push_back(pTickArray);
            memset(pTickArray, 0, sizeof(CTickData<N>)* tick_num);

            for (int i = 0 ; i != tick_num; ++i)
            {
                pTickArray[i].depthsNum = N;
                pTickArray[i].instIndex = inst;
                pTickArray[i].timeInMS = i + timeInMS_offset;
                pNewStore->getStore<N>().push_back(&pTickArray[i]);
            }

            fakedata.push_back(pNewStore);
        }

        void packDatatoReplayer()
        {
            if (replayer)  delete replayer;
            replayer = new MarketDataReplayer(fakedata);
        }



        virtual void TearDown() 
        {
            for (auto each : fakedata)
            {
                 //delete [] each->level1Data[0];  TODO delete the array
                 delete each;
            }
          

            for (auto eachArray : allocateArray)
            {
                delete [] eachArray;
            }

            if(replayer) delete replayer;
             
        }

        std::vector<MarketDataStore*> fakedata;
        MarketDataReplayer*  replayer;
        std::vector<void*> allocateArray;
    };


    TEST(testTickData, depthnum_init)
    {
        TickDataLevel1 d1;
        TickDataLevel5 d5;
        TickDataLevel10 d10;
        TickDataLevel20 d20;


        EXPECT_EQ(d1.depthsNum , 1);
        EXPECT_EQ(d5.depthsNum , 5);
        EXPECT_EQ(d10.depthsNum , 10);
        EXPECT_EQ(d20.depthsNum , 20);

        TickDataLevel1* pd5 =  reinterpret_cast<CTickData<1>*>(&d5);

         EXPECT_EQ(pd5->depthsNum, 5);

         EXPECT_EQ(60, sizeof(TickDataLevel1));
    }
    TEST_F(marketdatareplayerTest, singleTickStore)
    {
        const int tickNum = 20;
        generateFakeTick<1>(1,20151230, 0 , tickNum);

        packDatatoReplayer();


        MockTickDataConsumer mockconsumer;
        replayer->subscribeInstrument(1, &mockconsumer);

        using ::testing::_;
        using ::testing::An;
        using ::testing::Field;

        EXPECT_CALL(mockconsumer, onMarketData(An<const TickDataLevel1&>())).Times(tickNum);



        replayer->startReplay(20151230, 20151231);  
    }

    TEST_F(marketdatareplayerTest, singleTickStoreWithSomeDoesNeedTick)
    {
        const int tickNum = 20;
        generateFakeTick<1>(1,20151230, 0 , tickNum);
        generateFakeTick<1>(2,20151230, 0 , tickNum);
        packDatatoReplayer();


        MockTickDataConsumer mockconsumer;
        replayer->subscribeInstrument(1, &mockconsumer);

        using ::testing::_;
        using ::testing::An;
        using ::testing::Field;

        EXPECT_CALL(mockconsumer, onMarketData(An<const TickDataLevel1&>())).Times(tickNum);



        replayer->startReplay(20151230, 20151231);  
    }

    TEST_F(marketdatareplayerTest, multiTickStoreWithSomeDoesNeedTick)
    {
        const int tickNum = 20;
        generateFakeTick<1>(1,20151230, 0 , tickNum);
        generateFakeTick<1>(1,20151230, 5 , tickNum);
        generateFakeTick<1>(2,20151230, 0 , tickNum);
        packDatatoReplayer();


        MockTickDataConsumer mockconsumer;
        replayer->subscribeInstrument(1, &mockconsumer);

        using ::testing::_;
        using ::testing::An;
        using ::testing::Field;

        EXPECT_CALL(mockconsumer, onMarketData(An<const TickDataLevel1&>())).Times(tickNum * 2);



        replayer->startReplay(20151230, 20151231);  
    }


    TEST_F(marketdatareplayerTest, multiTickDifferentDepths)
    {
        const int tickNum = 20;
        generateFakeTick<1>(1,20151230, 0 , tickNum);
        generateFakeTick<5>(1,20151230, 5 , tickNum);
        generateFakeTick<1>(2,20151230, 0 , tickNum);
        packDatatoReplayer();


        MockTickDataConsumer mockconsumer;
        replayer->subscribeInstrument(1, &mockconsumer);

        using ::testing::_;
        using ::testing::An;
        using ::testing::Field;

        EXPECT_CALL(mockconsumer, onMarketData(An<const TickDataLevel1&>())).Times(tickNum);
        EXPECT_CALL(mockconsumer, onMarketData(An<const TickDataLevel5&>())).Times(tickNum);
        replayer->startReplay(20151230, 20151231);  
    }

    TEST_F(marketdatareplayerTest, different_instrument_mixed)
    {
        const int tickNum = 20;
        generateFakeTick<1>(1,20151230, 0 , tickNum);
        generateFakeTick<5>(1,20151230, 5 , tickNum);
        generateFakeTick<1>(2,20151230, 0 , tickNum);
        packDatatoReplayer();


        MockTickDataConsumer mockconsumer;
        replayer->subscribeInstrument(1, &mockconsumer);
        replayer->subscribeInstrument(2, &mockconsumer);

        using ::testing::_;
        using ::testing::An;
        using ::testing::Field;

        EXPECT_CALL(mockconsumer, onMarketData(An<const TickDataLevel1&>())).Times(tickNum * 2);
        EXPECT_CALL(mockconsumer, onMarketData(An<const TickDataLevel5&>())).Times(tickNum);
        replayer->startReplay(20151230, 20151231);  
    }

    TEST_F(marketdatareplayerTest, unsubscribe_instument)
    {
        const int tickNum = 20;
        generateFakeTick<1>(1,20151230, 0 , tickNum);
        packDatatoReplayer();


        MockTickDataConsumer mockconsumer;
        replayer->subscribeInstrument(1, &mockconsumer);


        EXPECT_CALL(mockconsumer, onMarketData(An<const TickDataLevel1&>())).Times(tickNum );
        
        replayer->startReplay(20151230, 20151231);  

    
        //ensure the recall startReplay will play the tick  
        EXPECT_CALL(mockconsumer, onMarketData(An<const TickDataLevel1&>())).Times(tickNum);
        mockconsumer.resetFakeTime();
        replayer->startReplay(20151230, 20151231);  

        replayer->unSubscribeInstrument(1, &mockconsumer);
        EXPECT_CALL(mockconsumer, onMarketData(An<const TickDataLevel5&>())).Times(0);
        mockconsumer.resetFakeTime();
        replayer->startReplay(20151230, 20151231);  
    }


    TEST_F(marketdatareplayerTest, date_range)
    {
        generateFakeTick<1>(1,20151230, 0 , 10);
        generateFakeTick<1>(2,20151231, 0 , 100);
        generateFakeTick<1>(1,20151231, 0 , 100);
        generateFakeTick<1>(1,20160101, 0 , 1000);
        generateFakeTick<1>(1,20160102, 0 , 10000);
        packDatatoReplayer();


        MockTickDataConsumer mockconsumer;
        replayer->subscribeInstrument(1, &mockconsumer);


        Sequence s1, s2;

        //EXPECT_CALL(mockconsumer, onStartDay(20151230)).InSequence(s1);
        EXPECT_CALL(mockconsumer, onMarketData(An<const TickDataLevel1&>())).Times(10).InSequence(s1);
       // EXPECT_CALL(mockconsumer, onEndDay(20151230)).InSequence(s1);

      
      //  EXPECT_CALL(mockconsumer, onStartDay(20151231)).InSequence(s1);
        EXPECT_CALL(mockconsumer, onMarketData(An<const TickDataLevel1&>())).Times(100).InSequence(s1);
       // EXPECT_CALL(mockconsumer, onEndDay(20151231)).InSequence(s1);

        replayer->startReplay(20151230, 20160101);  


        //EXPECT_CALL(mockconsumer, onStartDay(20151230)).InSequence(s1);
        EXPECT_CALL(mockconsumer, onMarketData(An<const TickDataLevel1&>())).Times(10).InSequence(s1);
        //EXPECT_CALL(mockconsumer, onEndDay(20151230)).InSequence(s1);

        mockconsumer.resetFakeTime();
        replayer->startReplay(20151230, 20151231);  

        //EXPECT_CALL(mockconsumer, onStartDay(20151231)).InSequence(s1);
        EXPECT_CALL(mockconsumer, onMarketData(An<const TickDataLevel1&>())).Times(100).InSequence(s1);
        //EXPECT_CALL(mockconsumer, onEndDay(20151231)).InSequence(s1);


       // EXPECT_CALL(mockconsumer, onStartDay(20160101)).InSequence(s1);
        EXPECT_CALL(mockconsumer, onMarketData(An<const TickDataLevel1&>())).Times(1000).InSequence(s1);
       // EXPECT_CALL(mockconsumer, onEndDay(20160101)).InSequence(s1);
        mockconsumer.resetFakeTime();
        replayer->startReplay(20151231, 20160102); 
    }

}
