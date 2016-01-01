#pragma once
#include <string>

namespace BluesTrading
{

    class IConfigureable
    {
    public:
        virtual void onMessage(const std::string& propName) = 0;	//Receive ProtoBuf Message From Console
        virtual std::string getDisplayMessage() = 0;		//ProtoBuf Message For Display
        virtual std::string getConfigureableMessage() = 0;	//ProtoBuf Message For Configure
        virtual std::string getName() = 0;						//for Manager Dispatch message
    };

    class IConfigureManager
    {
    public:
        virtual void registerConfigureable(IConfigureable*) = 0;		
        virtual void notifyConfiguableUpdate(IConfigureable*) = 0;		// for configureable instance notify mananger it changed
    };

}