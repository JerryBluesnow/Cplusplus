/** 
 * boost multi index container test 
 * platform: win32, visual studio 2005/2010; Linux, gcc4.1.2 
 */  
#include <iostream>  
#include <boost/multi_index_container.hpp>  
#include <boost/multi_index/composite_key.hpp>  
#include <boost/multi_index/member.hpp>  
#include <boost/multi_index/ordered_index.hpp>  
using namespace std;  
using namespace boost::multi_index;  
using boost::multi_index_container;  

#define MAX_LENGTH 16
#define MAX_IPv6_ADDR_LEN MAX_LENGTH

typedef enum {
        IPV4_IP = 0,
        IPV6_IP = 1
} IP_ADDRESS_TYPE;

typedef struct DM_V4V6IPADDRESS{
    IP_ADDRESS_TYPE ip_type;
    unsigned char addr[MAX_IPv6_ADDR_LEN];
    DM_V4V6IPADDRESS(const char* input_addr){
        for (int ix=0,iy=0; iy<MAX_IPv6_ADDR_LEN; ix++,iy++)
        {
            char data_tmp[3];
            data_tmp[2] = 0;
            data_tmp[0] = input_addr[ix];
            ix++;
            data_tmp[1] = input_addr[ix];

            for (int i = 0; i < 2; i++)
            {
                if (data_tmp[i] >= '0' && data_tmp[i] <= '9')
                {
                    data_tmp[i] = data_tmp[i] - '0';
                }
                else if (data_tmp[i] >= 'a' && data_tmp[i] <= 'z')
                {
                    data_tmp[i] = data_tmp[i] - 'a';
                }
            }

            addr[iy] = data_tmp[0] * 16 + data_tmp[1];
        }
    }
}DM_V4V6IPADDRESS;


inline int IPcompare( DM_V4V6IPADDRESS &c1, DM_V4V6IPADDRESS &c2)
{
        //IPv4 < IPv6
        if(c1.ip_type < c2.ip_type) return -1;
        if(c1.ip_type > c2.ip_type) return 1;

        return memcmp(c1.addr, c2.addr, MAX_IPv6_ADDR_LEN);;
}

inline bool operator ==( DM_V4V6IPADDRESS &c1,DM_V4V6IPADDRESS &c2)
{
        return IPcompare(c1, c2) == 0;
}

inline bool operator >( DM_V4V6IPADDRESS &c1,DM_V4V6IPADDRESS &c2)
{
        return IPcompare(c1, c2) > 0;
}

inline bool operator <( DM_V4V6IPADDRESS &c1,DM_V4V6IPADDRESS &c2)
{
        return IPcompare(c1, c2) < 0;
}

inline ostream & operator << (ostream &output, DM_V4V6IPADDRESS &data){
                output << "ip_type is "<<(int)data.ip_type << endl;
                output << "ip_addr is [";
                for(int i =0; i< MAX_IPv6_ADDR_LEN; i++){
                        output<< hex << (int)data.addr[i]<<dec <<":";
                }
                output<<"]"<<endl;
                return output;
}

template<class T>
class range_index{
public:
    T index_start;
    T index_end;

    range_index() : index_start(0), index_end(0)
    {
        //
    }
    range_index(T start, unsigned int end) : index_start(start), index_end(end)
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
    unsigned int              acl_tabl_id;
    range_index<DM_V4V6IPADDRESS> ip_suffix_range;
    range_index<unsigned int> port_range;
    unsigned int              vlan_id;
    acl_list_address_index()
    {
        acl_tabl_id                 = 0;
       // ip_suffix_range.index_start = 0;
       // ip_suffix_range.index_end   = 0;
        port_range.index_start      = 0;
        port_range.index_end        = 0;
        vlan_id                     = 0;
    }

    acl_list_address_index(unsigned int tbl_id, range_index<DM_V4V6IPADDRESS> ip_rang, range_index<unsigned int> port_rang, unsigned vl_id)
        : acl_tabl_id(tbl_id), ip_suffix_range(ip_rang), port_range(port_rang), vlan_id(vl_id)
    {

    }

    void
    print(char *prompt) const
    {
        cout << acl_tabl_id << ", "
             << "(" << ip_suffix_range.index_start << ", " << ip_suffix_range.index_end << ")"
             << "-(" << port_range.index_start << ", " << port_range.index_end << ")"
             << " - " << prompt << endl;
    }
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
    acl_list_db(unsigned int              tbl_id,
                range_index<unsigned int> ip_suf_rang,
                range_index<unsigned int> port_rang,
                unsigned int              _vlan_id,
                acl_list_attributes       acl_list_attr,
                unsigned int              _id)
    {
        myIndex.acl_tabl_id     = tbl_id;
        myIndex.ip_suffix_range = ip_suf_rang;
        myIndex.port_range      = port_rang;
        myIndex.vlan_id         = _vlan_id;
        myData                  = acl_list_attr;
        id                      = _id;
    }

    ~acl_list_db() { print(", destructed"); }

    void
    print(const char *prompt = "") const
    {
        cout << "(" << myIndex.acl_tabl_id << ", " << myIndex.ip_suffix_range.index_start << ", "
             << myIndex.ip_suffix_range.index_end << ", " << myIndex.port_range.index_start << ", "
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
        ordered_unique<tag<Index_XY_Tag>, xy_key, composite_key_result_less<xy_key::result_type> >
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
    if (lhs.acl_tabl_id < rhs.acl_tabl_id)
        return true;
        
    if (lhs.acl_tabl_id > rhs.acl_tabl_id)
        return false;

    cout << "acl_tabl_id equal, then compare ip_suffix"
         << "," << lhs.acl_tabl_id << "," << rhs.acl_tabl_id << "," << lhs.ip_suffix_range.index_start
         << "," << rhs.ip_suffix_range.index_start << "," << lhs.port_range.index_start << ","
         << rhs.port_range.index_start << endl;

    if (lhs.ip_suffix_range < rhs.ip_suffix_range)
        return true;

    if (lhs.ip_suffix_range > rhs.ip_suffix_range)
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

    return false;
}

void
ACLContainer::insert(acl_list_db *acl_db)
{
    if (acl_db == NULL)
    {
        return;
    }

    mycontainer.insert(acl_db);
    cout << "ADD: " << acl_db->id << ", " << acl_db->myIndex.acl_tabl_id << ", "
         << acl_db->myIndex.ip_suffix_range.index_start << ", "
         << acl_db->myIndex.ip_suffix_range.index_end << ", "
         << acl_db->myIndex.port_range.index_start << ", " << acl_db->myIndex.port_range.index_end
         << endl;
}

void ACLContainer::find(acl_list_address_index my_index)  
{  
    MyContainerIterator_X_T it0, it1;  
    boost::tie(it0, it1) = mycontainerx.equal_range(boost::make_tuple(my_index));  
      
    if (*it0 == *it1)  
    {
        cout << my_index.acl_tabl_id <<", (" <<my_index.ip_suffix_range.index_start<<", "
             << my_index.ip_suffix_range.index_end<<"), (" << my_index.port_range.index_start<<", "
             << my_index.port_range.index_end<<") " << "- not found" << endl;
        return;  
    }  
    while (it0 != it1)  
    {  
        (*it0)->print(", found");  
        ++it0;  
    }  
}  

void ACLContainer::find(acl_list_address_index my_index, unsigned int my_id) 
{
   MyContainerIterator_XY_T it0, it1;
   boost::tie(it0, it1) = mycontainerxy.equal_range(boost::make_tuple(my_index, my_id));

   if (*it0 == *it1)
   {
       cout << my_id << ", " << my_index.acl_tabl_id << ", (" << my_index.ip_suffix_range.index_start
            << ", " << my_index.ip_suffix_range.index_end << "), ("
            << my_index.port_range.index_start << ", " << my_index.port_range.index_end << ") "
            << "- not found..." << endl;
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
       cout << my_id << ", " << my_index.acl_tabl_id << ", (" << my_index.ip_suffix_range.index_start
            << ", " << my_index.ip_suffix_range.index_end << "), ("
            << my_index.port_range.index_start << ", " << my_index.port_range.index_end << ") "
            << "- not found" << endl;
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
    // mytest->print("");  //this clause can work
    os << acl_db_ptr->id << ", " << acl_db_ptr->myIndex.acl_tabl_id << ", ("
       << acl_db_ptr->myIndex.ip_suffix_range.index_start << ", "
       << acl_db_ptr->myIndex.ip_suffix_range.index_end << "), ("
       << acl_db_ptr->myIndex.port_range.index_start << ", "
       << acl_db_ptr->myIndex.port_range.index_end << ") "
       << "- " << endl;
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

void test_insert()  
{
    //MyContainer_T mycontainer;
    // cout << "Allocate a new container for ACL list......" << endl;
    // acl = new ACLContainer();
    // acl->insert(new acl_list_db(14, range_index<unsigned int>(252, 253), range_index<unsigned int>(1000, 1200), 0, acl_list_attributes(), 1));
    // acl->insert(new acl_list_db(14, range_index<unsigned int>(250, 255), range_index<unsigned int>(1230, 1240), 0, acl_list_attributes(), 2));
    // acl->insert(new acl_list_db(14, range_index<unsigned int>(250, 255), range_index<unsigned int>(0, 1239), 0, acl_list_attributes(), 3));
    // acl->insert(new acl_list_db(14, range_index<unsigned int>(250, 255), range_index<unsigned int>(1, 2490), 0, acl_list_attributes(), 4));
    // acl->insert(new acl_list_db(14, range_index<unsigned int>(250, 255), range_index<unsigned int>(2490, 2500), 0, acl_list_attributes(), 5));
    // acl->print(); 
}

void
test_search()
{
    // int table_id = 0;
    // int ip     = 0;
    // int port   = 0;

    // while (1)
    // {
    //     cout << "DB has below data: " << endl;
    //     acl->print();

    //     cout << "Please input table_id(table_id 999999 means break):";

    //     cin >> table_id;
    //     if (table_id == 999999)
    //         break;

    //     cout << "Please input ip to be searched:";

    //     cin >> ip;

    //     cout << "Please input port to be searched:";

    //     cin >> port;

    //     cout << "search ";

    //     acl->find(acl_list_address_index(table_id, range_index<unsigned int>(ip, ip),
    //                                      range_index<unsigned int>(port, port), 0));
    // }
    return;
}

int main()  
{
    test_insert();

    test_search();

    system("pause");
    return 0;  
}  
