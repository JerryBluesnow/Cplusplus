/** 
 * boost multi index container test 
 * platform: win32, visual studio 2005/2010; Linux, gcc4.1.2 
 */  
#include <iostream>  
#include <boost/multi_index_container.hpp>  
#include <boost/multi_index/composite_key.hpp>  
#include <boost/multi_index/member.hpp>  
#include <boost/multi_index/ordered_index.hpp>  
#include "../DM_V4V6IPADDRESS.h"

#include <boost/lambda/lambda.hpp> 

using namespace std;  
using namespace boost::multi_index;  
using boost::multi_index_container;  

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

// define object to be indexed
class acl_list_db
{
   public:
    unsigned int           id;
    acl_list_address_index myIndex;
    acl_list_attributes    myData;

   public:
    acl_list_db(unsigned int                  tbl_id,
                range_index<DM_V4V6IPADDRESS> ip_range,
                range_index<unsigned int>     port_rang,
                unsigned int                  _vlan_id,
                acl_list_attributes           acl_list_attr,
                unsigned int                  _id)
    {
        myIndex.acl_tabl_id = tbl_id;
        myIndex.ip_range    = ip_range;
        myIndex.port_range  = port_rang;
        myIndex.vlan_id     = _vlan_id;
        myData              = acl_list_attr;
        id                  = _id;
    }

    ~acl_list_db() { print(", destructed"); }

    void
    print(const char *prompt = "") const
    {
        cout << "(" << myIndex.acl_tabl_id << ", " << myIndex.ip_range.index_start << ", "
             << myIndex.ip_range.index_end << ", " << myIndex.port_range.index_start << ", "
             << myIndex.port_range.index_end << ") ---";
        cout << prompt << endl;
        //cout << "(" << myData.a << ", " << myData.b << ")" << prompt << endl;
    }

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

typedef nth_index<MyContainer_T, 0>::type MyContainer_X_T;  
typedef nth_index<MyContainer_T, 1>::type MyContainer_XY_T;  
typedef MyContainer_T::index<Index_X_Tag>::type MyContainerIndex_X_T;  
typedef MyContainer_T::index<Index_X_Tag>::type::iterator MyContainerIterator_X_T;  
typedef std::pair<MyContainerIterator_X_T, bool> MyContainerPair_X_T;  
typedef MyContainer_T::index<Index_XY_Tag>::type MyContainerIndex_XY_T;  
typedef MyContainer_T::index<Index_XY_Tag>::type::iterator MyContainerIterator_XY_T;  
typedef std::pair<MyContainerIterator_XY_T, bool> MyContainerPair_XY_T;

class ACLContainer
{
   private:
    MyContainer_T     mycontainer;
    MyContainer_X_T & mycontainerx;
    MyContainer_XY_T &mycontainerxy;

   public:
    ACLContainer()
        : mycontainerx(mycontainer), mycontainerxy(get<1>(mycontainer))
    {
    }

    void
    insert(acl_list_db *acl_db_ptr);
    void
    find(acl_list_address_index my_index);
    void 
    find(acl_list_address_index my_index, unsigned int my_id);
    void
    print();
    void
    free();
    void 
    erase(acl_list_address_index my_index, unsigned int my_id);
};

bool
operator<(const acl_list_address_index &lhs, const acl_list_address_index &rhs)
{
    cout << "using operator < to compare........" << endl;

    cout << "LEFT data to be compared" << endl;
    cout << dec << lhs.acl_tabl_id << ", ";
    cout << lhs.ip_range.index_start << ", " << lhs.ip_range.index_end << endl;
    cout << lhs.port_range.index_start << ", " << dec << lhs.port_range.index_end << endl;

    cout << "RIGHT data to be compared" << endl;
    cout << dec << rhs.acl_tabl_id << ", ";
    cout << rhs.ip_range.index_start << ", " << rhs.ip_range.index_end << endl;
    cout << rhs.port_range.index_start << ", " << dec << rhs.port_range.index_end << endl;

    if (lhs.acl_tabl_id < rhs.acl_tabl_id)
        return true;
        
    if (lhs.acl_tabl_id > rhs.acl_tabl_id)
        return false;

    cout << "acl_tabl_id equal, then compare ip_suffix"<<endl;

    if (lhs.ip_range < rhs.ip_range)
        return true;

    if (lhs.ip_range > rhs.ip_range)
        return false;

    cout << "ip_suffix equal, then compare port"<< endl;

    if (lhs.port_range < rhs.port_range)
        return true;

    if (lhs.port_range > rhs.port_range)
        return false;
    
    cout << "port equal, then compare vlan_id"<< endl;

    if (lhs.vlan_id < rhs.vlan_id)
        return true;

    if (lhs.vlan_id > rhs.vlan_id)
        return false;

    cout << "------EQUAL-------"<< endl;
 
    return false;
}

bool
operator>(const acl_list_address_index &lhs, const acl_list_address_index &rhs)
{
    cout << "using operator > to compare........" << endl;
    cout << "LEFT data to be compared" << endl;
    cout << dec << lhs.acl_tabl_id << ", ";
    cout << lhs.ip_range.index_start << ", " << lhs.ip_range.index_end << endl;
    cout << lhs.port_range.index_start << ", " << dec << lhs.port_range.index_end << endl;

    cout << "RIGHT data to be compared" << endl;
    cout << dec << rhs.acl_tabl_id << ", ";
    cout << rhs.ip_range.index_start << ", " << rhs.ip_range.index_end << endl;
    cout << rhs.port_range.index_start << ", " << dec << rhs.port_range.index_end << endl;

    if (lhs.acl_tabl_id > rhs.acl_tabl_id)
        return true;
        
    if (lhs.acl_tabl_id < rhs.acl_tabl_id)
        return false;

    cout << "acl_tabl_id equal, then compare ip_suffix"<<endl;

    if (lhs.ip_range > rhs.ip_range)
        return true;

    if (lhs.ip_range < rhs.ip_range)
        return false;

    cout << "ip_suffix equal, then compare port"<< endl;

    if (lhs.port_range > rhs.port_range)
        return true;

    if (lhs.port_range < rhs.port_range)
        return false;
    
    cout << "port equal, then compare vlan_id"<< endl;

    if (lhs.vlan_id > rhs.vlan_id)
        return true;

    if (lhs.vlan_id < rhs.vlan_id)
        return false;

    cout << "------EQUAL-------"<< endl;
 
    return false;
}

bool
operator==(const acl_list_address_index &lhs, const acl_list_address_index &rhs)
{
    cout << "using operator == to compare........" << endl;  
    
    return ( (!(lhs > rhs)) && (!(lhs < rhs)));
}

void
ACLContainer::insert(acl_list_db *acl_db)
{
    if (acl_db == NULL)
    {
        return;
    }
    cout << "ADD: " << acl_db->id << ", " << acl_db->myIndex.acl_tabl_id << endl;
    cout << acl_db->myIndex.ip_range.index_start << endl;
    cout << acl_db->myIndex.ip_range.index_end << endl;
    cout << acl_db->myIndex.port_range.index_start << endl;
    cout << acl_db->myIndex.port_range.index_end << endl;

    mycontainer.insert(acl_db);
}

void ACLContainer::find(acl_list_address_index my_index)  
{  
    MyContainerIterator_X_T it0, it1;  
    boost::tie(it0, it1) = mycontainerx.equal_range(boost::make_tuple(my_index));  
      
    if (*it0 == *it1)  
    {
        cout << my_index.acl_tabl_id <<", (" <<my_index.ip_range.index_start<<", "
             << my_index.ip_range.index_end<<"), (" << my_index.port_range.index_start<<", "
             << my_index.port_range.index_end<<") " << "- not found" << endl;
        return;  
    }  
    
    while (it0 != it1)  
    {
        /* NOTE-START
         * the code is added to make sure the returned value is the right one 
         */
        if ((*it0)->myIndex == my_index)
        {
            (*it0)->print(", found");
        }
        /* NOTE-END */

        ++it0;
    }  
}  

void ACLContainer::find(acl_list_address_index my_index, unsigned int my_id) 
{
   MyContainerIterator_XY_T it0, it1;
   boost::tie(it0, it1) = mycontainerxy.equal_range(boost::make_tuple(my_index, my_id));

   if (*it0 == *it1)
   {
        cout << my_id << endl ;
        cout << my_index.acl_tabl_id << endl;
        cout << my_index.ip_range.index_start << endl;
        cout << my_index.ip_range.index_end << endl;
        cout << my_index.port_range.index_start << endl;
        cout << my_index.port_range.index_end << endl;
        cout << "- not found..." << endl;
       return;
   }

   while (it0 != it1)
   {
       (*it0)->print(", found...");

       ++it0;
   }
}  

void ACLContainer::erase(acl_list_address_index my_index, unsigned int my_id) 
{
   MyContainerIterator_XY_T it0, it1;
   boost::tie(it0, it1) = mycontainerxy.equal_range(boost::make_tuple(my_index, my_id));

   if (*it0 == *it1)
   {
        cout << my_id << endl ;
        cout << my_index.acl_tabl_id << endl;
        cout << my_index.ip_range.index_start << endl;
        cout << my_index.ip_range.index_end << endl;
        cout << my_index.port_range.index_start << endl;
        cout << my_index.port_range.index_end << endl;
        cout << "- not found..." << endl;
       return;
   }

   (*it0)->print(", found and erase.....");
   mycontainerxy.erase(it0);

   return;
}  

void ACLContainer::print()  
{  
    //obtain a reference to the index tagged by Tag  
    const typename boost::multi_index::index<MyContainer_T, Index_X_Tag>::type& indexSet = get<Index_X_Tag>(mycontainer);  
    typedef typename MyContainer_T::value_type value_type;  
    //dump the elements of the index to cout  
    std::copy(indexSet.begin(), indexSet.end(), std::ostream_iterator<value_type>(cout));
    return;
}  

std::ostream& operator<<(std::ostream& os, const acl_list_db* acl_db_ptr)  
{
    os << dec << acl_db_ptr->id << endl;
    os << dec << acl_db_ptr->myIndex.acl_tabl_id << endl;
    os << acl_db_ptr->myIndex.ip_range.index_start << endl;
    os << acl_db_ptr->myIndex.ip_range.index_end << endl;
    os << dec << acl_db_ptr->myIndex.port_range.index_start << endl;
    os << acl_db_ptr->myIndex.port_range.index_end << "- " << endl;
    return os;  
}  

void ACLContainer::free()  
{

    typedef typename MyContainer_T::value_type value_type;
    while (!mycontainer.empty())
    {
        typename MyContainer_T::iterator iter = mycontainer.begin();
        if (NULL == (*iter))
        {
            mycontainer.erase(iter);
            continue;
        }
        value_type pobj = *iter;
        mycontainer.erase(iter);
        delete pobj;
    }

    return;
}

ACLContainer *acl;
#define ACL_INDEX_OFF_SET 4096
void test_insert()  
{
    //MyContainer_T mycontainer;
    cout << "Allocate a new container for ACL list......" << endl;
    acl = new ACLContainer();
    acl->insert(new acl_list_db(14, 
        range_index<DM_V4V6IPADDRESS>(DM_V4V6IPADDRESS(IPV6_IP, "30000000000000000000000000000001"), 
                                      DM_V4V6IPADDRESS(IPV6_IP, "3fff000000000000000000000000ffff")), 
        range_index<unsigned int>(1000, 1200), 
        0, 
        acl_list_attributes(), 
        0));

    acl->insert(new acl_list_db(14, 
        range_index<DM_V4V6IPADDRESS>(DM_V4V6IPADDRESS(IPV6_IP, "20000000000000000000000000000001"), 
                                      DM_V4V6IPADDRESS(IPV6_IP, "3fff000000000000000000000000ffff")), 
        range_index<unsigned int>(1230, 1240), 
        0, 
        acl_list_attributes(), 
        1));  
    /*
    acl->insert(new acl_list_db(14, 
        range_index<DM_V4V6IPADDRESS>(DM_V4V6IPADDRESS(IPV6_IP, "30000000000000000000000000000001"), 
                                      DM_V4V6IPADDRESS(IPV6_IP, "3fff000000000000000000000000ffff")), 
        range_index<unsigned int>(1000, 1200), 
        0, 
        acl_list_attributes(), 
        0 + ACL_INDEX_OFF_SET));

    acl->insert(new acl_list_db(14, 
        range_index<DM_V4V6IPADDRESS>(DM_V4V6IPADDRESS(IPV6_IP, "20000000000000000000000000000001"), 
                                      DM_V4V6IPADDRESS(IPV6_IP, "3fff000000000000000000000000ffff")), 
        range_index<unsigned int>(1230, 1240), 
        0, 
        acl_list_attributes(), 
        1 + ACL_INDEX_OFF_SET));     
    */
    acl->print(); 
}

void
test_search()
{
    int table_id = 0;
    int ip     = 0;
    int port   = 0;

    while (1)
    {
        cout << "DB has below data: " << endl;
        acl->print();

        cout << "Please input table_id(table_id 999999 means break):";

        cin >> dec >> table_id;
        if (table_id == 999999)
            break;

        char ip[MAX_IPv6_ADDR_LEN*2];
        cout << "Please input ip to be searched:";

        cin >> ip;

        cout << "Please input port to be searched:";

        cin >> dec >> port;

        cout << "search ";

        acl->find(acl_list_address_index(table_id, 
                    range_index<DM_V4V6IPADDRESS>(DM_V4V6IPADDRESS(IPV6_IP, ip),  DM_V4V6IPADDRESS(IPV6_IP, ip)),
                    range_index<unsigned int>(port, port), 0));
    }
    return;
}

int main()  
{

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
