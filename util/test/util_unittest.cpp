#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "util.h"
#include <sstream>

namespace BluesTrading
{

TEST(parserProps, parsString)
{
   std::stringstream ss;
   ss << "//SimpleStrategy Config\n";
   ss << "prop1=value1" << "\n";
   ss << "prop2=value2" << "\n";
   ss << "prop3=value3" << "\n";
   ss << "prop4=[1,2,0.1]     " << "\n";
   ss << "prop5={value51,value52,value53}    " << "\n";
   ss << "prop6=[1,10,1]     " << "\n";
   std::vector< std::map<std::string, std::string> >  ret =  parserProps(ss);
   EXPECT_EQ(ret.size(),  11 * 3 * 10);

   for (auto& eachParaSet : ret)
   {
       for(auto& eachParaPair: eachParaSet)
       {
            const char* value = eachParaPair.second.c_str();
           if (eachParaPair.first == "prop1")
           {
              
               EXPECT_STREQ(value, "value1");
           }

           if (eachParaPair.first == "prop2")
           {
               EXPECT_STREQ(value, "value2");
           }

           if (eachParaPair.first == "prop3")
           {
               EXPECT_STREQ(value, "value3");
           }
       }
   }

   auto getprop5string= [](int index)
   {
       switch (index)
       {
       case 1:
           return "value51";
       case 2:
           return "value52";
       case 3:
           return "value53";
       }
   };


   auto checkExist = [&] (double prop4, const std::string& prop5_string, int prop6)
   {
       for (auto& eachParaSet : ret)
       {
           double prop4_value = atof(eachParaSet["prop4"].c_str());

           if(abs(prop4_value - prop4) > 0.01 )
           {
               continue;
           }

           const std::string& prop5_value = eachParaSet["prop5"];


           if (prop5_string != prop5_value)
           {
               continue;
           }

           int prop6_value = atoi(eachParaSet["prop6"].c_str());

           if (prop6_value == prop6)
           {
               return true;
           }
           else
           {
               continue;
           }
       }
       //std::cout << prop4 << " " << prop5_string << " " << prop6 <<  "   does not contain \n";
       ADD_FAILURE(); 
       return false;
   };

   for (double prop4 = 1.0f; prop4 <= 2.0 ; prop4 += 0.1)
   {
       for (int prop5index = 1; prop5index <= 3;  ++ prop5index)
       {
           std::string prop5Value = getprop5string(prop5index);
           for (int prop6_value = 1; prop6_value <= 10;  ++ prop6_value)
           {
                 EXPECT_TRUE(checkExist(prop4, prop5Value, prop6_value));
           }
          
       }
   }
}

}