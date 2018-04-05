/** 
 * boost multi index container test 
 * platform: win32, visual studio 2005/2010; Linux, gcc4.1.2 
 */  
#include <iostream>  
#include <boost/multi_index_container.hpp>  
#include <boost/multi_index/composite_key.hpp>  
#include <boost/multi_index/member.hpp>  
#include <boost/multi_index/ordered_index.hpp>  
//#include "../DM_V4V6IPADDRESS.h"

#include <boost/lambda/lambda.hpp> 

#include <boost/lambda/lambda.hpp> 

using namespace std;  
using namespace boost::multi_index;  
using boost::multi_index_container;  

<<<<<<< HEAD
template<class T>
class range_index{
public:
    T index_start;
    T index_end;

    range_index() : index_start(0), index_end(0)
    {
        //
    }
    range_index(T start, T end) : index_start(start), index_end(end)
    {
        //
    }
    range_index(const range_index &obj)
    {
        this->index_start = obj.index_start;
        this->index_end   = obj.index_end;
    }

    range_index operator=(const range_index &obj)
    {
        this->index_start = obj.index_start;
        this->index_end   = obj.index_end;
        return *this;
    }
};


// redefine <, the overlap will be taken as euqal 
template<class T>
bool
operator<(const range_index<T> &lhs, const range_index<T> &obj)  //重载 <
{
    return (lhs.index_end < obj.index_start);
}

template<class T>
bool
operator>(const range_index<T> &lhs, const range_index<T> &obj)  //重载 >
{
    return (lhs.index_start > obj.index_end);
}

template<class T>
bool
operator==(const range_index<T> &lhs, const range_index<T> &obj)  //重载 >
{
    return ((!(lhs < obj)) && (!(lhs > obj)));
}

class acl_list_address_index
{
   public:
    unsigned int                  acl_tabl_id;
    range_index<DM_V4V6IPADDRESS> ip_range;
    range_index<unsigned int>     port_range;
    unsigned int                  vlan_id;

    acl_list_address_index(unsigned int tbl_id, range_index<DM_V4V6IPADDRESS> ip_rang, range_index<unsigned int> port_rang, unsigned vl_id)
        : acl_tabl_id(tbl_id), ip_range(ip_rang), port_range(port_rang), vlan_id(vl_id)
    {

    }

    void
    print(char *prompt) const
    {
        cout << "-----------------start---------------------------" << endl;
        cout << acl_tabl_id << endl;
        cout << ip_range.index_start << endl;
        cout << ip_range.index_end << endl;
        cout << port_range.index_start << ", " << port_range.index_end;
        cout << " - ";
        cout << prompt;
        cout << endl;
        cout << "-----------------end-----------------------------"<<endl;
    }

    acl_list_address_index(){};
};

// define data to be indexed
class acl_list_attributes
{
   public:
    char _reserved;
    acl_list_attributes(){}
    acl_list_attributes
    operator=(const acl_list_attributes &obj)
    {
        acl_list_attributes target;
        target._reserved = obj._reserved;
        return (target);
    }
};

=======
>>>>>>> update from workPC
// define object to be indexed
class acl_list_db
{
   public:
    unsigned int     _id; 
    unsigned int     _tbl_id;
    DM_V4V6IPADDRESS _ip_start;
    DM_V4V6IPADDRESS _ip_end;
    unsigned int     _port_start;
    unsigned int     _port_end;
    unsigned int     _vlan_id;

    //acl_list_address_index myIndex;
    //acl_list_attributes    myData;

   public:
    acl_list_db(unsigned int     tbl_id,
                unsigned int     id,
                DM_V4V6IPADDRESS ip_start,
                DM_V4V6IPADDRESS ip_end,
                unsigned int     port_start,
                unsigned int     port_end,
                unsigned int     vlan_id)
    {
        _tbl_id     = tbl_id;
        _id         = id;
        _ip_start   = ip_start;
        _ip_end     = ip_end;
        _port_start = port_start;
        _port_end   = port_end;
        _vlan_id    = vlan_id;
    }

    ~acl_list_db() { print(", destructed"); }

    void
    print(const char *prompt = "") const
    {
       cout << "tbl_id["<<_tbl_id<<"],";
       cout << "id["<<_id<<"],";
       cout << "ip_start["<<_ip_start<<"],";
       cout << "ip_end["<<_ip_end<<"],";
       cout << "port_start["<<_port_start<<"],";
       cout << "port_end["<<_port_end<<"],";
       cout << "vlan_id["<<_vlan_id<<"],";
       cout << endl;
    }

<<<<<<< HEAD
   private:
    acl_list_db(const acl_list_db &);
    acl_list_db &
    operator=(const acl_list_db &);
};

struct x_key:composite_key<  
  acl_list_db,  
  BOOST_MULTI_INDEX_MEMBER(acl_list_db, acl_list_address_index, myIndex)  
>{};
struct xy_key:composite_key<  
  acl_list_db,  
  BOOST_MULTI_INDEX_MEMBER(acl_list_db, acl_list_address_index, myIndex) , 
  BOOST_MULTI_INDEX_MEMBER(acl_list_db, unsigned int, id)
>{};

struct Index_X_Tag{};  
struct Index_XY_Tag{};  
typedef multi_index_container<  
    acl_list_db*,  
    indexed_by<  
        ordered_non_unique<tag<Index_X_Tag>, x_key, composite_key_result_less<x_key::result_type> >,  
        ordered_non_unique<tag<Index_XY_Tag>, xy_key, composite_key_result_less<xy_key::result_type> >
    >  
>MyContainer_T;  
=======
>>>>>>> update from workPC

   private:
    
};

bool
operator<(const acl_list_db &lhs, const acl_list_db &rhs)
{
    if (lhs._tbl_id < rhs._tbl_id)
    {
        return (true);
    }

    if (lhs._tbl_id > rhs._tbl_id)
    {
        return (false);
    }

    if (lhs._id < rhs._id)
    {
        return (true);
    }

    if (lhs._id > rhs._id)
    {
        return (false);
    }

    return (false);
}

struct index_key:composite_key<  
  acl_list_db,  
  BOOST_MULTI_INDEX_MEMBER(acl_list_db, unsigned int, _tbl_id),  
  BOOST_MULTI_INDEX_MEMBER(acl_list_db, unsigned int, _id)  
>{};

struct ip_s_key:composite_key<  
  acl_list_db,  
  //BOOST_MULTI_INDEX_MEMBER(acl_list_db, unsigned int, _tbl_id),  
  //BOOST_MULTI_INDEX_MEMBER(acl_list_db, unsigned int, _id),
  BOOST_MULTI_INDEX_MEMBER(acl_list_db, DM_V4V6IPADDRESS, _ip_start)
>{};

struct ip_e_key:composite_key<  
  acl_list_db,  
  //BOOST_MULTI_INDEX_MEMBER(acl_list_db, unsigned int, _tbl_id),  
  //BOOST_MULTI_INDEX_MEMBER(acl_list_db, unsigned int, _id),
  //BOOST_MULTI_INDEX_MEMBER(acl_list_db, DM_V4V6IPADDRESS, _ip_start),
  BOOST_MULTI_INDEX_MEMBER(acl_list_db, DM_V4V6IPADDRESS, _ip_end)
>{};

struct port_s_key:composite_key<  
  acl_list_db,  
  //BOOST_MULTI_INDEX_MEMBER(acl_list_db, unsigned int, _tbl_id),  
//   BOOST_MULTI_INDEX_MEMBER(acl_list_db, unsigned int, _id),
//   BOOST_MULTI_INDEX_MEMBER(acl_list_db, DM_V4V6IPADDRESS, _ip_start),
//   BOOST_MULTI_INDEX_MEMBER(acl_list_db, DM_V4V6IPADDRESS, _ip_end),
  BOOST_MULTI_INDEX_MEMBER(acl_list_db, unsigned int, _port_start)
>{};

struct port_e_key:composite_key<  
  acl_list_db,  
//   BOOST_MULTI_INDEX_MEMBER(acl_list_db, unsigned int, _tbl_id),  
//   BOOST_MULTI_INDEX_MEMBER(acl_list_db, unsigned int, _id),
//   BOOST_MULTI_INDEX_MEMBER(acl_list_db, DM_V4V6IPADDRESS, _ip_start),
//   BOOST_MULTI_INDEX_MEMBER(acl_list_db, DM_V4V6IPADDRESS, _ip_end),
//   BOOST_MULTI_INDEX_MEMBER(acl_list_db, unsigned int, _port_start),
  BOOST_MULTI_INDEX_MEMBER(acl_list_db, unsigned int, _port_end)
>{};

struct Index_index_tag{};  
struct Index_ip_s_tag{};  
struct Index_ip_e_tag{};  
struct Index_port_s_tag{};  
struct Index_port_e_tag{};  
typedef multi_index_container<  
    acl_list_db,  
    boost::multi_index::indexed_by<  
        ordered_unique<tag<Index_index_tag>, index_key, composite_key_result_less<index_key::result_type> >
        //ordered_non_unique<tag<Index_ip_s_tag>, ip_s_key, composite_key_result_less<ip_s_key::result_type> >,
        //ordered_non_unique<tag<Index_ip_e_tag>, ip_e_key, composite_key_result_less<ip_e_key::result_type> >,
       // ordered_non_unique<tag<Index_port_s_tag>, port_s_key, composite_key_result_less<port_s_key::result_type> >,
        //ordered_non_unique<tag<Index_port_e_tag>, port_e_key, composite_key_result_less<port_e_key::result_type> >
    >  
>MyContainer_T;  

int main()
{
    MyContainer_T s;

<<<<<<< HEAD
using namespace boost::lambda;
typedef multi_index_container<double, double> double_set;
// note: default template parameters resolve to
// multi_index_container<double,indexed_by<unique<identity<double> > > >.

double_set s;

// s.insert(99, 120);
// s.insert(100, 120);
// s.insert(106, 120);
// s.insert(110, 120);
// s.insert(140, 120);
// s.insert(190, 120);
// s.insert(200, 120);
// s.insert(200.01, 120);

// std::pair<double_set::iterator,double_set::iterator> p=
// s.range(100.0<=boost::lambda::_1,boost::lambda::_2<=200); // 100<= x <=200

// for_each(p.first, p.second, std::cout << boost::lambda::_1 << ' ');

// https://blog.csdn.net/playboy1/article/details/7568703
// https://www.boost.org/doc/libs/1_64_0/libs/multi_index/doc/tutorial/basics.html#range
// https://blog.csdn.net/lee353086/article/details/40706669
// https://www.boost.org/doc/libs/1_55_0/libs/multi_index/doc/index.html
// https://blog.csdn.net/lsjseu/article/details/43370707
// https://blog.csdn.net/gongxinheng/article/details/5421914
return 0;
// range [it0,it1) contains the elements in [100,200]

    test_insert();
    // acl->lower_bound(acl_list_address_index(14, 
    //                 range_index<DM_V4V6IPADDRESS>(DM_V4V6IPADDRESS(IPV6_IP, "2511000000020cc30000000000010004"),  DM_V4V6IPADDRESS(IPV6_IP, "2511000000020cc30000000000010004")),
    //                 range_index<unsigned int>(1239, 1239), 0));
    // return 0;
    test_search();

    system("pause");
    return 0;  
}  
=======
    std::pair<MyContainer_T::iterator, MyContainer_T::iterator> p =
        s.range(acl_list_db(1,2,DM_V4V6IPADDRESS(),DM_V4V6IPADDRESS(),2,2,0) <= boost::lambda::_1, boost::lambda::_1 <= acl_list_db(1,2,DM_V4V6IPADDRESS(),DM_V4V6IPADDRESS(),2,2,0));
    return 0;
}
>>>>>>> update from workPC
