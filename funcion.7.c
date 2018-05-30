#include <iostream>
#include <vector>
#include <algorithm>
#include <functional>
#include <stdarg.h>
#include "boost/algorithm/string.hpp"
#include "boost/lambda/lambda.hpp"
#include "boost/lambda/bind.hpp"

#include "IMS_TESTING_macros_redefine.h"
using namespace boost::algorithm;

using namespace std;

void
IBCF_sipplus_add_remove(std::string &input_string, unsigned short sipplus)
{
    // must make sure the input string is "+12345678",
    // NOT "+123456+8",
    // NOT "+"
    // NOT ""
    if (sipplus > 1)
    {
        IMS_DLOG(IMS_IBCF_LOGHIGH, "%s: invalid input sipplus: %d", __func__, sipplus);
        return;
    }

    if (input_string == "+" || input_string == "")
    {
        IMS_DLOG(IMS_IBCF_LOGLOW, "%s: input string: %s, will not process it", __func__, input_string.c_str());
        return;
    }

    IMS_DLOG(IMS_IBCF_LOGLOW, "%s: input string: %s, sipplus(%d)", __func__, input_string.c_str(), sipplus);

    // first, remove all "+" in the input_string, then according to sipplus flag, decide add it back
    // or not.
    std::string::size_type pos = input_string.find('+');
    while(pos != std::string::npos)  
    {  
        input_string.erase(pos, 1);  
        pos = input_string.find('+');
    }  

    // add sipplus
    if (sipplus == 1)
    {
        input_string = "+" + input_string;
    }

    IMS_DLOG(IMS_IBCF_LOGLOW, "%s: after process sipplus, string: %s", __func__, input_string.c_str());

    return;
}

int
main()
{
    std::string usr_name_string("+1605088031111;rn=5081709123;npdi");

    std::string new_usr_string;

    std::string::size_type token_pos = usr_name_string.find(';');
    if (token_pos == std::string::npos)
    {
        new_usr_string.assign(usr_name_string);
    }
    else
    {
        new_usr_string.assign(usr_name_string, 0, token_pos);
    }
    cout << new_usr_string << endl;


    const char* abc="how are your?";
    void*       abc_void = (void*)abc;
    return 0;
}