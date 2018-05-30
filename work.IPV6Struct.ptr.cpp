/*    
 *  A work well solution using object, but will use more memory!
 */

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

#define USHORT unsigned short

typedef struct ACL_TBL_KEY
{
    USHORT tbl_id;
    USHORT id;
    ACL_TBL_KEY(USHORT tbl_id_, USHORT id_)
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
    ACL_TBL_KEY          _tbl_key;
    DM_V4V6IPADDRESS_PTR _ip_start;
    DM_V4V6IPADDRESS_PTR _ip_end;
    USHORT               _port_start;
    USHORT               _port_end;
    USHORT               _vlan_id;

    acl_list_db(ACL_TBL_KEY          tbl_key,
                DM_V4V6IPADDRESS_PTR ip_start,
                DM_V4V6IPADDRESS_PTR ip_end,
                USHORT               port_start,
                USHORT               port_end,
                USHORT               vlan_id)
        : _tbl_key(tbl_key),
          _ip_start(ip_start),
          _ip_end(ip_end),
          _port_start(port_start),
          _port_end(port_end),
          _vlan_id(vlan_id)
    {
    }

    ~acl_list_db()
    {
        if (_ip_start.value != NULL)
        {
            delete _ip_start.value;
            _ip_start.value = NULL;
        }

        if (_ip_end.value != NULL)
        {
            delete _ip_end.value;
            _ip_end.value = NULL;
        }
    }

    bool
    operator<(const acl_list_db& e) const
    {
        return _tbl_key < e._tbl_key;
    }
    bool
    operator<=(const acl_list_db& e) const
    {
        return _tbl_key <= e._tbl_key;
    }

    friend std::ostream&
    operator<<(std::ostream& os, const acl_list_db& dt);
};

std::ostream&
operator<<(std::ostream& os, const acl_list_db& dt)
{
    os << "[" << dt._tbl_key.tbl_id << ',' << dt._tbl_key.id << ',' << *dt._ip_start.value << ','
       << *dt._ip_end.value << ',' << dt._port_start << ',' << dt._port_end << ',' << dt._vlan_id << "]";
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
            boost::multi_index::member<acl_list_db, DM_V4V6IPADDRESS_PTR, &acl_list_db::_ip_start> >,
        boost::multi_index::ordered_non_unique<
            boost::multi_index::tag<ip_end>,
            boost::multi_index::member<acl_list_db, DM_V4V6IPADDRESS_PTR, &acl_list_db::_ip_end> >,
        boost::multi_index::ordered_non_unique<
            boost::multi_index::tag<port_start>,
            boost::multi_index::member<acl_list_db, USHORT, &acl_list_db::_port_start> >,
        boost::multi_index::ordered_non_unique<
            boost::multi_index::tag<port_end>,
            boost::multi_index::member<acl_list_db, USHORT, &acl_list_db::_port_end> >,
        boost::multi_index::ordered_non_unique<
            boost::multi_index::tag<vlan_id>,
            boost::multi_index::member<acl_list_db, USHORT, &acl_list_db::_vlan_id> > > >
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

    public:
     ACLContainer()
     // : mycontainerx(mycontainer)
     {

     }

     void
     insert(acl_list_db* acl_db_ptr);

     void
     erase(ACL_TBL_KEY &tbl_key);

     unsigned short
     find(ACL_TBL_KEY tbl_key, DM_V4V6IPADDRESS_PTR ip, USHORT port, USHORT vlan_id = 0);
     
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

     acl_list_container* table_ptr;
     
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

// will not delete the acl_list_db object
void  ACLContainer::internal_free()
{
    if (table_ptr == NULL)
    {
        std::cout << "table_ptr is invalid" << std::endl;
        return;
    }

    typedef typename acl_list_container::value_type value_type;

    while (!table_ptr->empty())
    {
        typename acl_list_container::iterator iter = table_ptr->begin();
        if (NULL == (*iter))
        {
            table_ptr->erase(iter);
            continue;
        }
        table_ptr->erase(iter);
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

void ACLContainer::erase(ACL_TBL_KEY &tbl_key) 
{
    // int count = table.erase(tbl_key, DM_V4V6IPADDRESS(IPV4_IP, "192.168.1.1"),
    //             DM_V4V6IPADDRESS(IPV4_IP, "192.168.1.1"), 1000, 1000, 0);
    // std::cout << count << "records are deleted" << endl;

    // may need to find a palce to delete/[new] the pointer
    
    return;
}  

void ACLContainer::print_out_tbl()
{
    std::cout << "All Data in the table has been dumped below:" << std::endl;
    std::cout << "--------------S-----T-----A-----R-----T--------------" << std::endl;
	const acl_list_by_tbl_key& tbl_key_index = table.get<tbl_key>();

    typedef typename acl_list_container::value_type value_type;  
    
    //std::copy(tbl_key_index.begin(), tbl_key_index.end(), std::ostream_iterator<value_type>(cout));
    for (acl_list_by_tbl_key::iterator iter = tbl_key_index.begin(); iter != tbl_key_index.end();
         iter++)
    {
        std::cout << *(*iter);
    }
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
ACLContainer::find(ACL_TBL_KEY          input_tbl_key,
                   DM_V4V6IPADDRESS_PTR input_ip,
                   USHORT               input_port,
                   USHORT               input_vlan_id)
{
    acl_list_container table_tbl_key;
    acl_list_container table_ip_start;
    acl_list_container table_ip_end;
    acl_list_container table_port_start;
    acl_list_container table_port_end;

    acl_list_container* table_tbl_key_ptr    = &table_tbl_key;
    acl_list_container* table_ip_end_ptr     = &table_ip_end;
    acl_list_container* table_ip_start_ptr   = &table_ip_start;
    acl_list_container* table_port_start_ptr = &table_port_start;
    acl_list_container* table_port_end_ptr   = &table_port_end;

    bind_table(&table);
    std::cout << "print all data" << endl;
    print_out_curr_internal_tbl();
    {
        acl_list_by_tbl_key& itbl_key_index = table.get<tbl_key>();

        std::pair<acl_list_by_tbl_key::iterator, acl_list_by_tbl_key::iterator> my_pair;

        my_pair =
            itbl_key_index.range(ACL_TBL_KEY(input_tbl_key.tbl_id, 0) <= boost::lambda::_1,
                                 boost::lambda::_1 <= ACL_TBL_KEY(input_tbl_key.tbl_id, 40960));

        bind_table(table_tbl_key_ptr);

        std::for_each(my_pair.first, my_pair.second,
                      bind(&ACLContainer::internal_insert, this, boost::lambda::_1));
        std::cout << "print tbl_key data" << endl;
        print_out_curr_internal_tbl();
    }

    {
        acl_list_by_ip_start& ip_start_index = table_ptr->get<ip_start>();

        std::pair<acl_list_by_ip_start::iterator, acl_list_by_ip_start::iterator> my_pair;

        DM_V4V6IPADDRESS zero_ip_value;

        memset(&zero_ip_value, 0, sizeof(zero_ip_value));

        zero_ip_value.ip_type = input_ip.value->ip_type;
		
		DM_V4V6IPADDRESS_PTR zero_ip(&zero_ip_value);
		
        my_pair = ip_start_index.range(zero_ip <= boost::lambda::_1, boost::lambda::_1 <= input_ip);

        bind_table(table_ip_start_ptr);

        std::for_each(my_pair.first, my_pair.second,
                      bind(&ACLContainer::internal_insert, this, boost::lambda::_1));

        std::cout << "print start ip data" << endl;
        print_out_curr_internal_tbl();
    }

    {
        acl_list_by_ip_end& ip_end_index = table_ptr->get<ip_end>();

        std::pair<acl_list_by_ip_end::iterator, acl_list_by_ip_end::iterator> my_pair;

        DM_V4V6IPADDRESS mask_ip_value;

        memset(mask_ip_value.addr, 0xff, MAX_IPv6_ADDR_LEN);

        mask_ip_value.ip_type = input_ip.value->ip_type;
		
		DM_V4V6IPADDRESS_PTR mask_ip(&mask_ip_value);
		
        my_pair = ip_end_index.range(input_ip <= boost::lambda::_1, boost::lambda::_1 <= mask_ip);

        bind_table(table_ip_end_ptr);

        std::for_each(my_pair.first, my_pair.second,
                      bind(&ACLContainer::internal_insert, this, boost::lambda::_1));

        std::cout << "print end ip data" << endl;
        print_out_curr_internal_tbl();
    }

    {
        acl_list_by_port_start& port_start_index = table_ptr->get<port_start>();

        std::pair<acl_list_by_port_start::iterator, acl_list_by_port_start::iterator> my_pair;

        my_pair = port_start_index.range(0 <= boost::lambda::_1, boost::lambda::_1 <= input_port);

        bind_table(table_port_start_ptr);

        std::for_each(my_pair.first, my_pair.second,
                      bind(&ACLContainer::internal_insert, this, boost::lambda::_1));

        std::cout << "print start port data" << endl;
        print_out_curr_internal_tbl();
    }

    {
        acl_list_by_port_end& port_end_index = table_ptr->get<port_end>();

        std::pair<acl_list_by_port_end::iterator, acl_list_by_port_end::iterator> my_pair;

        my_pair = port_end_index.range(input_port <= boost::lambda::_1, boost::lambda::_1 <= 65535);

        bind_table(table_port_end_ptr);

        std::for_each(my_pair.first, my_pair.second,
                      bind(&ACLContainer::internal_insert, this, boost::lambda::_1));

        std::cout << "print end port data" << endl;
        print_out_curr_internal_tbl();
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

    std::cout << "found index: " << ret_val << endl;

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

    unbind_table();
    return (ret_val);
}

void
ACLContainer::create_stat()
{
    cout << "hello world" << endl;

    table.insert(new acl_list_db(
        ACL_TBL_KEY(20, 200), DM_V4V6IPADDRESS_PTR(new DM_V4V6IPADDRESS(IPV4_IP, "ae0f00ff000000000000000000000000")),
        DM_V4V6IPADDRESS_PTR(new DM_V4V6IPADDRESS(IPV4_IP, "ae0f00ff000000000000000000000000")), 1000, 1000, 0));

    table.insert(new acl_list_db(
        ACL_TBL_KEY(20, 201), DM_V4V6IPADDRESS_PTR(new DM_V4V6IPADDRESS(IPV4_IP, "ae0f00fe000000000000000000000000")),
        DM_V4V6IPADDRESS_PTR(new DM_V4V6IPADDRESS(IPV4_IP, "ae0f00fe000000000000000000000000")), 1000, 1200, 0));
}

void
test_plus(const char* input_str, IP_ADDRESS_TYPE ip_type)
{
    DM_V4V6IPADDRESS data(ip_type, input_str);
    ++data;
    cout << data << endl;
}

void
test_minus(const char* input_str, IP_ADDRESS_TYPE ip_type)
{
    DM_V4V6IPADDRESS data(ip_type, input_str);
    --data;
    cout << data << endl;
}

int
main(int argc, _TCHAR* argv[])
{
    ACLContainer my_test_container;

    little_timer.Start();
    my_test_container.create_stat();
    little_timer.End();

    my_test_container.print_out_tbl();

    for (int i = 0; i < 1; i++)
    {
        DM_V4V6IPADDRESS searched_ip(IPV4_IP, "ae0f00fe000000000000000000000000");
        little_timer.Start();
        my_test_container.find(ACL_TBL_KEY(20, 0),
                              DM_V4V6IPADDRESS_PTR(&searched_ip), 1239);
        little_timer.End();
    }

    my_test_container.free();

    // test_plus("3000000000000000000000000000ffff", IPV6_IP);
    // test_plus("3000000000000000000000000000fffe", IPV6_IP);
    // test_plus("3000000000000000000000000000ef0f", IPV6_IP);
    // test_plus("3ffffffffffffffffffffffffffffeff", IPV6_IP);
    // test_plus("3fffffffffffffffffffffffffffffef", IPV6_IP);
    // test_plus("3ffffffffffffffffffffffffffffffe", IPV6_IP);
    // test_plus("3ffffffffffffffffffffffffffffff0", IPV6_IP);
    // test_plus("fffffffffffffffffffffffffffffffe", IPV6_IP);
    // test_plus("ffffffffffffffffffffffffffffffff", IPV6_IP);

    // std::cout << "test minus----------------------------" << endl;

    // test_minus("3000000000000000000000000000ffff", IPV6_IP);
    // test_minus("3000000000000000000000000000fffe", IPV6_IP);
    // test_minus("3000000000000000000000000000ef0f", IPV6_IP);
    // test_minus("3ffffffffffffffffffffffffffffeff", IPV6_IP);
    // test_minus("3fffffffffffffffffffffffffffffef", IPV6_IP);
    // test_minus("3ffffffffffffffffffffffffffffffe", IPV6_IP);
    // test_minus("3ffffffffffffffffffffffffffffff0", IPV6_IP);
    // test_minus("fffffffffffffffffffffffffffffffe", IPV6_IP);
    // test_minus("ffffffffffffffffffffffffffffffff", IPV6_IP);
    // test_minus("00000000000000000000000000000000", IPV6_IP);

    // test_plus("ae0f00ff000000000000000000000000", IPV4_IP);
    // test_plus("ae0f00fe000000000000000000000000", IPV4_IP);
    // test_plus("ae0f00f0000000000000000000000000", IPV4_IP);
    // test_plus("effffff0000000000000000000000000", IPV4_IP);
    // test_plus("fffffff0000000000000000000000000", IPV4_IP);

    // test_minus("ae0f00ff000000000000000000000000", IPV4_IP);
    // test_minus("ae0f00fe000000000000000000000000", IPV4_IP);
    // test_minus("ae0f00f0000000000000000000000000", IPV4_IP);
    // test_minus("effffff0000000000000000000000000", IPV4_IP);
    // test_minus("fffffff0000000000000000000000000", IPV4_IP);
    // test_minus("00000010000000000000000000000000", IPV4_IP);
    // test_minus("00000000000000000000000000000000", IPV4_IP);

    //暂停，输入任意键继续
    system("pause");
    return 0;
}
