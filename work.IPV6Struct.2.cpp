#include <list>
#include <string>

#include <boost/multi_index_container.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/identity.hpp>
#include <boost/multi_index/member.hpp>

#include <boost/lambda/lambda.hpp>
#include <boost/lambda/bind.hpp>
#include <iostream>

#include "DM_V4V6IPADDRESS.h"

#include "./timer/MyTimer.h"

using namespace std;

#define UINT unsigned int

typedef struct ACL_TBL_KEY
{
    UINT tbl_id;
    UINT id;
    ACL_TBL_KEY(UINT tbl_id_, UINT id_)
    {
        tbl_id = tbl_id_;
        id     = id_;
    }
} ACL_TBL_KEY;

bool
operator<(const ACL_TBL_KEY& key_1, const ACL_TBL_KEY& key_2)
{
    if (key_1.tbl_id < key_2.tbl_id)
    {
        return (true);
    }

    if (key_1.tbl_id > key_2.tbl_id)
    {
        return (false);
    }

    if (key_1.id < key_2.id)
    {
        return (true);
    }

    if (key_1.id > key_2.id)
    {
        return (false);
    }

    return (false);
}

bool
operator>(const ACL_TBL_KEY& key_1, const ACL_TBL_KEY& key_2)
{
    if (key_1.tbl_id > key_2.tbl_id)
    {
        return (true);
    }

    if (key_1.tbl_id < key_2.tbl_id)
    {
        return (false);
    }

    if (key_1.id > key_2.id)
    {
        return (true);
    }

    if (key_1.id < key_2.id)
    {
        return (false);
    }

    return (false);
}

bool
operator==(const ACL_TBL_KEY& key_1, const ACL_TBL_KEY& key_2)
{
    return ((!(key_1 < key_2)) && (!(key_1 > key_2)));
}

bool
operator<=(const ACL_TBL_KEY& key_1, const ACL_TBL_KEY& key_2)
{
    return (!(key_1 > key_2));
}

bool
operator>=(const ACL_TBL_KEY& key_1, const ACL_TBL_KEY& key_2)
{
    return (!(key_1 < key_2));
}

struct tbl_key
{
};
// struct id{};
struct ip_start
{
};
struct ip_end
{
};
struct port_start
{
};
struct port_end
{
};
struct vlan_id
{
};

struct acl_list_db
{
    ACL_TBL_KEY      _tbl_key;
    DM_V4V6IPADDRESS _ip_start;
    DM_V4V6IPADDRESS _ip_end;
    UINT             _port_start;
    UINT             _port_end;
    UINT             _vlan_id;

    acl_list_db(ACL_TBL_KEY      tbl_key,
                DM_V4V6IPADDRESS ip_start,
                DM_V4V6IPADDRESS ip_end,
                UINT             port_start,
                UINT             port_end,
                UINT             vlan_id)
        : _tbl_key(tbl_key),
          _ip_start(ip_start),
          _ip_end(ip_end),
          _port_start(port_start),
          _port_end(port_end),
          _vlan_id(vlan_id)
    {
    }

    bool
    operator<(const acl_list_db& e) const
    {
        return _tbl_key < e._tbl_key;
    }

    operator>(const acl_list_db& e) const
    {
        return _tbl_key > e._tbl_key;
    }

    bool
    operator<=(const acl_list_db& e) const
    {
        return _tbl_key <= e._tbl_key;
    }

    bool
    operator>=(const acl_list_db& e) const
    {
        return _tbl_key >= e._tbl_key;
    }

    friend std::ostream&
    operator<<(std::ostream& os, const acl_list_db& dt);
};

std::ostream&
operator<<(std::ostream& os, const acl_list_db& dt)
{
    os << "[" << dt._tbl_key.tbl_id << ',' << dt._tbl_key.id << ',' << dt._ip_start << ','
       << dt._ip_end << ',' << dt._port_start << ',' << dt._port_end << ',' << dt._vlan_id << "]";
    os << endl;
    return os;
}

typedef boost::multi_index::multi_index_container<
    acl_list_db*,
    boost::multi_index::indexed_by<
        boost::multi_index::ordered_unique<boost::multi_index::identity<acl_list_db> >,
        boost::multi_index::ordered_unique<
            boost::multi_index::tag<tbl_key>,
            boost::multi_index::member<acl_list_db, ACL_TBL_KEY, &acl_list_db::_tbl_key> >,
        boost::multi_index::ordered_non_unique<
            boost::multi_index::tag<ip_start>,
            boost::multi_index::member<acl_list_db, DM_V4V6IPADDRESS, &acl_list_db::_ip_start> >,
        boost::multi_index::ordered_non_unique<
            boost::multi_index::tag<ip_end>,
            boost::multi_index::member<acl_list_db, DM_V4V6IPADDRESS, &acl_list_db::_ip_end> >,
        boost::multi_index::ordered_non_unique<
            boost::multi_index::tag<port_start>,
            boost::multi_index::member<acl_list_db, UINT, &acl_list_db::_port_start> >,
        boost::multi_index::ordered_non_unique<
            boost::multi_index::tag<port_end>,
            boost::multi_index::member<acl_list_db, UINT, &acl_list_db::_port_end> >,
        boost::multi_index::ordered_non_unique<
            boost::multi_index::tag<vlan_id>,
            boost::multi_index::member<acl_list_db, UINT, &acl_list_db::_vlan_id> > > >
    acl_list_container;

typedef acl_list_container::index<tbl_key>::type acl_list_by_tbl_key;
typedef acl_list_container::index<ip_start>::type acl_list_by_ip_start;
typedef acl_list_container::index<ip_end>::type acl_list_by_ip_end;
typedef acl_list_container::index<port_start>::type acl_list_by_port_start;
typedef acl_list_container::index<port_end>::type acl_list_by_port_end;

class ACLContainer
{
public:
     acl_list_container  table;


    acl_list_container* table_ptr;
    public:
     ACLContainer()
     // : mycontainerx(mycontainer)
     {

     }

     void
     insert(acl_list_db* acl_db_ptr);

     void
     erase(ACL_TBL_KEY tbl_key);

     unsigned short
     find(ACL_TBL_KEY tbl_key, DM_V4V6IPADDRESS ip, UINT port, UINT vlan_id = 0);
     
     void
     free();

     void
     create_stat();

     void
     print_out_tbl();

     void
     print_out_curr_internal_tbl();

private:
     void
     bind_table(acl_list_container* tbl_ptr);

     void
     unbind_table();

     void
     internal_insert(acl_list_db* acl_db_ptr);

     void
     internal_free();

    private:
     acl_list_container  table_tbl_key;
     acl_list_container  table_ip_start;
     acl_list_container  table_ip_end;
     acl_list_container  table_port_start;
     acl_list_container  table_port_end;
};

void
ACLContainer::insert(acl_list_db* acl_db_ptr)
{
    if (acl_db_ptr == NULL)
    {
        return;
    }

    table.insert(acl_db_ptr);
}

// will not delete the acl_list_db object
void  ACLContainer::internal_free()
{
    typedef typename acl_list_container::value_type value_type;
    while (!table.empty())
    {
        typename acl_list_container::iterator iter = table.begin();
        if (NULL == (*iter))
        {
            table.erase(iter);
            continue;
        }
        table.erase(iter);
    }

    return;
}

// will delete the acl_list_db object
void  ACLContainer::free()
{
    typedef typename acl_list_container::value_type value_type;
    while (!table.empty())
    {
        typename acl_list_container::iterator iter = table.begin();
        if (NULL == (*iter))
        {
            table.erase(iter);
            continue;
        }
        value_type pobj = *iter;
        table.erase(iter);
        delete pobj;
    }

    return;
}

void
ACLContainer::internal_insert(acl_list_db* acl_db_ptr)
{
    if (acl_db_ptr == NULL)
    {
        std::cout << " input data is invalid" << endl;

        return;
    }
    
    if (table_ptr == NULL)
    {
        std::cout << " current operated table is invalid" << endl;

        return;
    }

    table_ptr->insert(acl_db_ptr);
}

void
ACLContainer::bind_table(acl_list_container* tbl_ptr)
{
    table_ptr = tbl_ptr;
}

void
ACLContainer::unbind_table()
{
    table_ptr = NULL;
}

void ACLContainer::erase(ACL_TBL_KEY input_tbl_key) 
{
//     acl_list_db* to_be_delete_obj = new acl_list_db(
//         ACL_TBL_KEY(input_tbl_key.tbl_id, input_tbl_key.id),
//         DM_V4V6IPADDRESS(IPV6_IP, "10000000000000000000000000000001"),
//         DM_V4V6IPADDRESS(IPV6_IP, "2511000000020cc30000000000010003"), 1200, 1400, 0);
//     //int count = table.erase(&to_be_delete_obj);

//     // std::cout << count << " records are deleted" << endl;


//     // may need to find a palce to delete/[new] the pointer
    
//     //     const acl_list_by_tbl_key& tbl_key_index = table.get<tbl_key>();
//     //     acl_list_by_tbl_key::iterator it;  
        
//     //     while ( (it = tbl_key_index.find(input_tbl_key)) != tbl_key_index.end() )  
//     //     {  
//     //         //table.erase(it, it);
//     //     }  

//     std::pair<acl_list_container::iterator, acl_list_container::iterator> p;  
//    // p = table.range( ACL <= boost::lambda::_1, boost::lambda::_1 <= to_be_delete_obj); 
//     //table.erase(p.first, p.second);  

         return;
}  

void ACLContainer::print_out_tbl()
{
    std::cout << "All Data in the table has been dumped below:" << std::endl;
    std::cout << "--------------S-----T-----A-----R-----T--------------" << std::endl;
	const acl_list_by_tbl_key& tbl_key_index = table.get<tbl_key>();

    typedef typename acl_list_container::value_type value_type;  
    
    std::copy(tbl_key_index.begin(), tbl_key_index.end(), std::ostream_iterator<value_type>(cout));
    
    std::cout << "--------------------E-----N-----D--------------------" << std::endl;

}

void ACLContainer::print_out_curr_internal_tbl()
{
    if (table_ptr == NULL)
    {
        std::cout << "current internal table pointer is invalid(table_ptr)" << endl;
        return;
    }
    std::cout << "All Data in the table has been dumped below:" << std::endl;
    std::cout << "--------------S-----T-----A-----R-----T--------------" << std::endl;

	const acl_list_by_tbl_key& tbl_key_index = table_ptr->get<tbl_key>();

    typedef typename acl_list_container::value_type value_type;  
    
    //std::copy(tbl_key_index.begin(), tbl_key_index.end(), std::ostream_iterator<value_type>(cout));
    for (acl_list_by_tbl_key::iterator iter = tbl_key_index.begin(); iter!=tbl_key_index.end(); iter++)
    {
        std::cout << *(*iter);
    }
    
    std::cout << "--------------------E-----N-----D--------------------" << std::endl;
}

unsigned short
ACLContainer::find(ACL_TBL_KEY      input_tbl_key,
                   DM_V4V6IPADDRESS input_ip,
                   UINT             input_port,
                   UINT             input_vlan_id)
{
     acl_list_container* table_tbl_key_ptr    = &table_tbl_key;
     acl_list_container* table_ip_end_ptr     = &table_ip_end;
     acl_list_container* table_ip_start_ptr   = &table_ip_start;
     acl_list_container* table_port_start_ptr = &table_port_start;
     acl_list_container* table_port_end_ptr   = &table_port_end;
     
     {
         acl_list_by_tbl_key& itbl_key_index = table.get<tbl_key>();

         std::pair<acl_list_by_tbl_key::iterator, acl_list_by_tbl_key::iterator> my_pair;

         my_pair = itbl_key_index.range(ACL_TBL_KEY(input_tbl_key.tbl_id,0) <= boost::lambda::_1, boost::lambda::_1 <= ACL_TBL_KEY(input_tbl_key.tbl_id,40960));

         bind_table(table_tbl_key_ptr);

         std::for_each(my_pair.first, my_pair.second, bind(&ACLContainer::internal_insert, this, boost::lambda::_1));
         //print_out_curr_internal_tbl();
    }

    {
        acl_list_by_ip_start& ip_start_index = table_ptr->get<ip_start>();

        std::pair<acl_list_by_ip_start::iterator, acl_list_by_ip_start::iterator> my_pair;
       
        bind_table(table_ip_start_ptr);

        my_pair = ip_start_index.range(input_ip >= boost::lambda::_1, boost::lambda::_1 <= input_ip);
        
        std::for_each(my_pair.first, my_pair.second, bind(&ACLContainer::internal_insert, this, boost::lambda::_1));

        //print_out_tbl(*table_ptr);
    }

    {
        acl_list_by_ip_end& ip_end_index = table_ptr->get<ip_end>();
        
        std::pair<acl_list_by_ip_end::iterator, acl_list_by_ip_end::iterator> my_pair;

        my_pair = ip_end_index.range(input_ip <= boost::lambda::_1, boost::lambda::_1 >= input_ip);

        bind_table(table_ip_end_ptr);

         std::for_each(my_pair.first, my_pair.second, bind(&ACLContainer::internal_insert, this, boost::lambda::_1));
        //print_out_tbl(*table_ptr);
    }

    {
        acl_list_by_port_start& port_start_index = table_ptr->get<port_start>();

        std::pair<acl_list_by_port_start::iterator, acl_list_by_port_start::iterator> my_pair;

        my_pair = port_start_index.range(input_port >= boost::lambda::_1, boost::lambda::_1 <= input_port);

        bind_table(table_port_start_ptr);

         std::for_each(my_pair.first, my_pair.second, bind(&ACLContainer::internal_insert, this, boost::lambda::_1));
        //print_out_tbl(*table_ptr);
    }

    {
        acl_list_by_port_end& port_end_index = table_ptr->get<port_end>();
        
        std::pair<acl_list_by_port_end::iterator, acl_list_by_port_end::iterator> my_pair;

        my_pair = port_end_index.range(input_port <= boost::lambda::_1, boost::lambda::_1 >= input_port);

        bind_table(table_port_end_ptr);

        std::for_each(my_pair.first, my_pair.second, bind(&ACLContainer::internal_insert, this, boost::lambda::_1));

        //print_out_tbl(*table_ptr);
    }

    unsigned short ret_val = -1;

    if (table_ptr->size() != 0)
    {
        typename acl_list_container::iterator iter = table_ptr->begin();
        if (iter != table_ptr->end())
        {
            ret_val = (*iter)->_tbl_key.id;
        }
    }

    bind_table(table_tbl_key_ptr);
    internal_free();
    bind_table(table_ip_start_ptr);
    internal_free();
    bind_table(table_ip_end_ptr);
    internal_free();
    bind_table(table_port_start_ptr);
    internal_free();
    bind_table(table_port_end_ptr);
    internal_free();
    
    return (ret_val);
}

void ACLContainer::create_stat()
{
    for (int table_idx=0; table_idx<40; table_idx++)
    {
        for (int item_idx=0; item_idx<50; item_idx++)
        {
            table.insert(new acl_list_db(
                ACL_TBL_KEY(table_idx, item_idx), DM_V4V6IPADDRESS(IPV6_IP, "20000000000000000000000000000001"),
                DM_V4V6IPADDRESS(IPV6_IP, "3fff000000000000000000000000ffff"), 1239, 1239, 0));

            table.insert(new acl_list_db(
                ACL_TBL_KEY(table_idx, item_idx+50), DM_V4V6IPADDRESS(IPV6_IP, "30000000000000000000000000000001"),
                DM_V4V6IPADDRESS(IPV6_IP, "3fff000000000000000000000000ffff"), 1000, 1000, 0));
        }
    }
}

int
main(int argc, _TCHAR* argv[])
{
    ACLContainer my_test_container;

    little_timer.Start();
    my_test_container.create_stat();
    little_timer.End();

    my_test_container.print_out_tbl();

    little_timer.Start();
    while 
    my_test_container.find(ACL_TBL_KEY(20,0), DM_V4V6IPADDRESS(IPV6_IP, "2511000000020cc30000000000010004"), 1239);
    little_timer.End();

    my_test_container.print_out_curr_internal_tbl();
    //暂停，输入任意键继续
    system("pause");
    return 0;
}