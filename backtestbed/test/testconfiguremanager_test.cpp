#include "../testConfigureManager.h"

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <sstream>
#include <string>

namespace BluesTrading
{
    using ::testing::Invoke;
    using ::testing::InvokeWithoutArgs;
    using ::testing::An;
    using ::testing::_;
    using ::testing::Field;
    using ::testing::Sequence;
    using ::testing::Truly;
    using ::testing::StrEq;

    class MockConfigureable: public IConfigureable
    {
    public:
        MOCK_METHOD1(onMessage,  void(const std::string& propName));
        virtual std::string getDisplayMessage() override {return std::string();};		//ProtoBuf Message For Display
        virtual std::string getConfigurableMessage()  override {return std::string();};	//ProtoBuf Message For Configure
        virtual std::string getName()  override {return std::string();};		
    };


    class testConfigureManagerTest : public ::testing::Test {
    protected:
        virtual void SetUp() 
        {
           std::istringstream input;
           input.str("test1\ntest2\ntest3\n");
           inst.readConfigure(input);
           inst.registerConfigurable(&consumer);
           std::cout << "\n\n\n\n\n\n\n\n\n=================================";
        }

        virtual void TearDown() 
        {
        }
        MockConfigureable consumer;
        TestConfigureManager inst;
    };

    TEST_F(testConfigureManagerTest, TestReadLines)
    {
        std::istringstream input;
        input.str("1\n2\n3\n4\n5\n6\n7\n");
        int sum = 0;
        for (std::string line; std::getline(input, line); )
        {
            sum += atoi(line.c_str());
        }
        EXPECT_EQ(28,sum);
    }

    TEST_F(testConfigureManagerTest, onMessageTest)
    {
        //std::istringstream input;
        //input.str("test1\ntest2\ntest3\n");
        //inst.readConfigure(input);
        EXPECT_EQ(3, inst.configureLines.size());
        Sequence s1;
        EXPECT_CALL(consumer, onMessage(StrEq("test1"))).InSequence(s1);
        EXPECT_CALL(consumer, onMessage(StrEq("test2"))).InSequence(s1);
        EXPECT_CALL(consumer, onMessage(StrEq("test3"))).InSequence(s1);
      
        inst.configurInstance();
    }
}

