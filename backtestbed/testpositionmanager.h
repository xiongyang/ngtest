#include "IPositionManager.h"

#include <map>
namespace BluesTrading
{
    class testPositionManger : public IPositionManager
    {
    public:

        virtual CPosition& getPosition(uint32_t inst) override
        {
            return allPositions_[inst];
        }

        virtual AccountInfo& getAccountInfo() override
        {
            return accountInfo;
        }

       
        private:
            std::map<uint32_t, CPosition> allPositions_;
             AccountInfo    accountInfo;
    };

}