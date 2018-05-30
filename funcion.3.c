/** 
 * boost multi index container test 
 * platform: win32, visual studio 2005/2010; Linux, gcc4.1.2 
 */  
#include <iostream>  
#include <boost/multi_index_container.hpp>  
#include <boost/multi_index/composite_key.hpp>  
#include <boost/multi_index/member.hpp>  
#include <boost/multi_index/ordered_index.hpp>  
#include <boost/lambda/lambda.hpp> 

using namespace std;  
using namespace boost::multi_index;  
using boost::multi_index_container;  

typedef struct tginfo{
    BOOL flag_1;
    BOOL flag_2;
    BOOL flag_3;

    unsigned int data1;
    unsigned int data2;

};

int main()  
{
    long abc = 9999999999999;
    std::cout << abc << endl;

tginfo my_tginfo;

my_tginfo.flag_1 = 1;
my_tginfo.flag_2 = 0;
my_tginfo.flag_3 = 1;
my_tginfo.data1= 3;
my_tginfo.data2= 3;
my_tginfo.flag_1 = abc;

my_tginfo;
return 0;  
}  
