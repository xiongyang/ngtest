#pragma once
#include <cstdint>

#include <vector>
//#include <iosforward>

#define CheckArrayIndex

#pragma pack(push, 4)
namespace BluesTrading
{

#ifdef CheckArrayIndex
    struct TickDataInvalidIndex{};
#endif

    struct CTickData {

        struct Depth
        {
            double price;
            uint32_t size;
            //uint32_t bidsize;
            //uint32_t asksize;
            //double bidprice;
            //double askprice;
        };

        uint32_t	instIndex;
        uint32_t	timeInMS;
        uint32_t	tot_vol;		
        uint32_t	openinterest;

        double		last_price;
        double		turnover;
        uint16_t    bidLevels;
        uint16_t    askLevels;


        std::vector<Depth>  depths; //first bid then ask        // custom allocator
        //Depth*		depths[MAX_LEVEL];

        CTickData():instIndex(-1), bidLevels(0), askLevels(0) {}
      //  CTickData()   //move constr

        Depth& getBidDepth(int index)  //start base 1
        {
            return depths[index - 1];
        }

        Depth& getAskDepth(int index)
        {
            return depths[index - 1 + bidLevels];
        }

        //double& BidPrice(int index)
        //{
        //    return depths[index - 1].price;
        //  //  return getDepth(index).bidprice;
        //}

        //double& AskPrice(int index)
        //{
        //     return depths[bidLevels + index - 1].price;
        //   // return getDepth(index).askprice;
        //}

        //uint32_t& BidSize(int index)
        //{
        //     return depths[index - 1].size;
        //  //  return getDepth(index).bidsize;
        //}

        //uint32_t& AskSize(int index)
        //{
        //    return getDepth(index).asksize;
        //}
    };
    //typedef CTickData<1> TickDataLevel1;
    //typedef CTickData<5> TickDataLevel5;
    //typedef CTickData<10> TickDataLevel10;
    //typedef CTickData<20> TickDataLevel20;

}

#pragma pack(pop)




