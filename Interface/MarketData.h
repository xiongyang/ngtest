#pragma once
#include <cstdint>

#define CheckArrayIndex

#pragma pack(push, 4)
namespace BluesTrading
{

#ifdef CheckArrayIndex
    struct TickDataInvalidIndex{};
#endif

    template<int N>
    struct CTickData {

        struct Depth
        {
            uint32_t bidsize;
            uint32_t asksize;
            double bidprice;
            double askprice;
        };

        static const int MAX_LEVEL = N;
        uint32_t	instIndex;
        uint32_t	timeInMS;
        uint32_t	tot_vol;		
        uint32_t	openinterest;

        double		last_price;
        double		turnover;
        uint32_t	depthsNum;

        Depth		depths[MAX_LEVEL];

        CTickData():instIndex(-1), depthsNum(MAX_LEVEL) {}



        template<typename ArrayType>
        inline Depth& getDepth(int i)
        {
#ifdef CheckArrayIndex
            if (i < 0 || i >= MAX_LEVEL)
            {
                throw TickDataInvalidIndex();
            }
#endif
            return depths[i];
        }

        double& BidPrice(int index)
        {
            return getDepth(index).bidprice;
        }

        double& AskPrice(int index)
        {
            return getDepth(index).askprice;
        }

        uint32_t& BidSize(int index)
        {
            return getDepth(index).bidsize;
        }

        uint32_t& AskSize(int index)
        {
            return getDepth(index).asksize;
        }
    };
    typedef CTickData<1> TickDataLevel1;
    typedef CTickData<5> TickDataLevel5;
    typedef CTickData<10> TickDataLevel10;
    typedef CTickData<20> TickDataLevel20;

}

#pragma pack(pop)




