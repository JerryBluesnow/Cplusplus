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

    std::pair<MyContainer_T::iterator, MyContainer_T::iterator> p =
        s.range(acl_list_db(1,2,DM_V4V6IPADDRESS(),DM_V4V6IPADDRESS(),2,2,0) <= boost::lambda::_1, boost::lambda::_1 <= acl_list_db(1,2,DM_V4V6IPADDRESS(),DM_V4V6IPADDRESS(),2,2,0));
    return 0;
}
