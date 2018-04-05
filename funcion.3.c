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

int main()  
{

using namespace boost::lambda;

typedef multi_index_container<double> double_set;

double_set s;

s.insert(99);
s.insert(100);
s.insert(106);
s.insert(110);
s.insert(140);
s.insert(190);
s.insert(200);
s.insert(200.01);

std::pair<double_set::iterator,double_set::iterator> p=
  s.range(100.0<=boost::lambda::_1,boost::lambda::_1<200);

for_each(p.first, p.second, std::cout << boost::lambda::_1 << ' ');
    return 0;  
}  
