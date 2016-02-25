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

#ifdef CheckArrayIndex
    struct TickDataInvalidIndex{};
#endif

    struct CTickData {

       // friend class boost::serialization::access;
        // When the class Archive corresponds to an output archive, the
        // & operator is defined similar to <<.  Likewise, when the class Archive
        // is a type of input archive the & operator is defined similar to >>.
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
            //uint32_t bidsize;
            //uint32_t asksize;
            //double bidprice;
            //double askprice;
            template<class Archive>
            void serialize(Archive & ar,  const unsigned int version)
            {
                ar & price;
                ar & size;
            }
        };

 

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

        Depth& getBidDepth(int index)  //start base 1
        {
            return depths[index - 1];
        }

        Depth& getAskDepth(int index)
        {
            return depths[index - 1 + bidLevels];
        }
    };

    //static std::ostream&  operator << (std::ostream& of, const CTickData& tick)
    //{
    //    ar & instIndex;
    //    ar & timeInMS;
    //    ar & tot_vol;		
    //    ar & openinterest;

    //    ar & last_price;
    //    ar & turnover;
    //    of<<    tick.bidLevels;
    //    of<<    tick.askLevels;

    //    for(auto& each :tick.depths)
    //    {
    //        of << each.price << each.size;
    //    }

    //    return of;
    //}
    //static std::istream& operator >> (std::istream& istream, CTickData& tick)
    //{
    //    istream >>	tick.instIndex;
    //    istream >>	tick.timeInMS;
    //    istream >>	tick.tot_vol;		
    //    istream >>	tick.openinterest;

    //    istream >>	tick.last_price;
    //    istream >>	tick.turnover;
    //    istream >>  tick.bidLevels;
    //    istream >>   tick.askLevels;

    //    int depths_size = tick.askLevels + tick.bidLevels;
    //    for (int i = 0; i != depths_size; ++i)
    //    {
    //        double price;
    //        uint32_t size;
    //        istream >> price >> size;

    //        tick.depths.push_back(CTickData::Depth{price, size}) ;
    //    }

    //    return istream;
    //}

}

#pragma pack(pop)




