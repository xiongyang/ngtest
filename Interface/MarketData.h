#pragma once
#include <cstdint>

#include <vector>
#include <iostream>

#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

#include <boost/serialization/vector.hpp>


#pragma pack(push, 4)
namespace BluesTrading
{
    struct CTickData {

        template<class Archive>
        void serialize(Archive & ar, const unsigned int version)
        {
            ar & instIndex;
            ar & timeInMS;
            ar & tot_vol;		
            ar & openinterest;

            ar & last_price;
            ar & turnover;
            ar & bidLevels;
            ar & askLevels;
            ar & depths;
        }


        struct Depth
        {
            double price;
            uint32_t size;
            template<class Archive>
            void serialize(Archive & ar,  const unsigned int version)
            {
                ar & price;
                ar & size;
            }
        };

        static constexpr Depth emptyDepth{0.0 , 0};

 

    public:
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
        CTickData(const CTickData&) = default;
        CTickData& operator=(const CTickData&) = default;
        CTickData(CTickData&& ref)
        {
            instIndex = ref.instIndex;
            timeInMS = ref.timeInMS;
            tot_vol = ref.tot_vol;		
            openinterest = ref.openinterest;

            last_price = ref.last_price;
            turnover = ref.turnover;
            bidLevels = ref.bidLevels;
            askLevels = ref.askLevels;
            depths.swap(ref.depths);
        }
         //move constr
        const Depth& getBidDepth(int index)  const//start base 1
        {
            if (index <= bidLevels )
            {
                 return depths[index - 1];
            }
            else
            {
                return emptyDepth;
            }
        }

        const Depth& getAskDepth(int index) const
        {
            if (index <= askLevels)
            {
                return depths[index - 1 + bidLevels];
            }
            else
            {
                 return emptyDepth;
            }
        }
    };
}

#pragma pack(pop)




