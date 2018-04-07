#include <list>
#include <string>

#include <boost/multi_index_container.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/identity.hpp>
#include <boost/multi_index/member.hpp>

#include <boost/lambda/lambda.hpp>
#include <boost/lambda/bind.hpp>
#include <iostream>

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
    ACL_TBL_KEY _tbl_key;
    UINT        _ip_start;
    UINT        _ip_end;
    UINT        _port_start;
    UINT        _port_end;
    UINT        _vlan_id;

    acl_list_db(ACL_TBL_KEY tbl_key,
               UINT        ip_start,
               UINT        ip_end,
               UINT        port_start,
               UINT        port_end,
               UINT        vlan_id)
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
    os << "[" << dt._tbl_key.tbl_id << ',' << dt._tbl_key.id << ',' << dt._ip_start << ','
       << dt._ip_end << ',' << dt._port_start << ',' << dt._port_end << ',' << dt._vlan_id << "]";
    os << endl;
    return os;
}

typedef boost::multi_index::multi_index_container<
    acl_list_db,
    boost::multi_index::indexed_by<
        boost::multi_index::ordered_unique<boost::multi_index::identity<acl_list_db> >,
        boost::multi_index::ordered_non_unique<
            boost::multi_index::tag<tbl_key>,
            boost::multi_index::member<acl_list_db, ACL_TBL_KEY, &acl_list_db::_tbl_key> >,
        boost::multi_index::ordered_non_unique<
            boost::multi_index::tag<ip_start>,
            boost::multi_index::member<acl_list_db, UINT, &acl_list_db::_ip_start> >,
        boost::multi_index::ordered_non_unique<
            boost::multi_index::tag<ip_end>,
            boost::multi_index::member<acl_list_db, UINT, &acl_list_db::_ip_end> >,
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
    acl_list_container     mycontainer;
    static acl_list_container     mycontainer_1;
    static acl_list_container     mycontainer_2;
    static acl_list_container     mycontainer_3;
    static acl_list_container     mycontainer_4;

   public:
    ACLContainer()
        // : mycontainerx(mycontainer)
    {

    }

    void
    insert(acl_list_db acl_db_ptr);
};

void
ACLContainer::insert(acl_list_db acl_db)
{
    // if (acl_db == NULL)
    // {
    //     return;
    // }

    mycontainer.insert(acl_db);
}

void print_out_tbl_key(const acl_list_container& table)
{
    std::cout << "All Data in the table has been dumped below:" << std::endl;
    std::cout << "--------------S-----T-----A-----R-----T--------------" << std::endl;
	const acl_list_by_tbl_key& tbl_key_index = table.get<tbl_key>();

    typedef typename acl_list_container::value_type value_type;  
    
    std::copy(tbl_key_index.begin(), tbl_key_index.end(), std::ostream_iterator<value_type>(cout));
    
    std::cout << "--------------------E-----N-----D--------------------" << std::endl;

}

void create_stat(acl_list_container& table)
{
    cout << "hello world" << endl;
    for (UINT y = 0; y < 100; y++)
    {
        for (UINT i = 0; i < 40960;)
        {
            table.insert(acl_list_db(ACL_TBL_KEY(y, i + 0), 110, 255, 1000, 1200, 0));
            table.insert(acl_list_db(ACL_TBL_KEY(y, i + 1), 001, 255, 1000, 1200, 0));
            table.insert(acl_list_db(ACL_TBL_KEY(y, i + 2), 120, 140, 1200, 1400, 0));
            table.insert(acl_list_db(ACL_TBL_KEY(y, i + 3), 130, 160, 1230, 1240, 0));
            table.insert(acl_list_db(ACL_TBL_KEY(y, i + 4), 55, 110, 1230, 1240, 0));
            i += 5;
        }
    }
}

acl_list_container table_tbl_key;
acl_list_container* table_tbl_key_ptr = &table_tbl_key;

acl_list_container table_ip_start;
acl_list_container* table_ip_start_ptr = &table_ip_start;

acl_list_container table_ip_end;
acl_list_container* table_ip_end_ptr = &table_ip_end;

acl_list_container table_port_start;
acl_list_container* table_port_start_ptr = &table_port_start;

acl_list_container table_port_end;
acl_list_container* table_port_end_ptr = &table_port_end;

static acl_list_container* table_ptr = NULL;

void create_aaaa(acl_list_db db){
    cout << "hello binding" << endl;
}


void create_bbbb(acl_list_db db){

    table_ptr->insert(db);
    cout << "hello binding" << endl;
}

void
update_pointer(acl_list_container* tbl_ptr)
{
    table_ptr = tbl_ptr;
}

void select_stat(acl_list_container& table)
{
    table_ptr = &table;

    {
        acl_list_by_tbl_key& itbl_key_index = table_ptr->get<tbl_key>();
        
        std::pair<acl_list_by_tbl_key::iterator, acl_list_by_tbl_key::iterator> ppp;

        ppp = itbl_key_index.range(ACL_TBL_KEY(20,0) <= boost::lambda::_1, boost::lambda::_1 <= ACL_TBL_KEY(20,40960));

        update_pointer(table_tbl_key_ptr);

        std::for_each(ppp.first, ppp.second, bind(&create_bbbb, _1));
        //print_out_tbl_key(*table_ptr);
    }

    {
        acl_list_by_ip_start& ip_start_index = table_ptr->get<ip_start>();

        std::pair<acl_list_by_ip_start::iterator, acl_list_by_ip_start::iterator> ppp;
        // acl_list_by_ip_start::iterator p0=ip_start_index.upper_bound(115);
       
        update_pointer(table_ip_start_ptr);

        // for (; p0 != ip_start_index.end(); p0++)
        // {
        //     create_bbbb(*p0);
        // }

        ppp = ip_start_index.range(120 >= boost::lambda::_1, boost::lambda::_1 <= 120);

        std::for_each(ppp.first, ppp.second, bind(&create_bbbb, _1));

        //print_out_tbl_key(*table_ptr);
    }

    {
        acl_list_by_ip_end& ip_end_index = table_ptr->get<ip_end>();
        
        std::pair<acl_list_by_ip_end::iterator, acl_list_by_ip_end::iterator> ppp;

        ppp = ip_end_index.range(120 <= boost::lambda::_1, boost::lambda::_1 >= 120);

        update_pointer(table_ip_end_ptr);

        std::for_each(ppp.first, ppp.second, bind(&create_bbbb, _1));
        //print_out_tbl_key(*table_ptr);
    }

    {
        acl_list_by_port_start& port_start_index = table_ptr->get<port_start>();

        std::pair<acl_list_by_port_start::iterator, acl_list_by_port_start::iterator> ppp;

        ppp = port_start_index.range(1239 >= boost::lambda::_1, boost::lambda::_1 <= 1239);

        update_pointer(table_port_start_ptr);

        std::for_each(ppp.first, ppp.second, bind(&create_bbbb, _1));
        //print_out_tbl_key(*table_ptr);
    }

    {
        acl_list_by_port_end& port_end_index = table_ptr->get<port_end>();
        
        std::pair<acl_list_by_port_end::iterator, acl_list_by_port_end::iterator> ppp;

        ppp = port_end_index.range(1239 <= boost::lambda::_1, boost::lambda::_1 >= 1239);

        update_pointer(table_port_end_ptr);
    
        std::for_each(ppp.first, ppp.second, bind(&create_bbbb, _1));
        print_out_tbl_key(*table_ptr);
    }
    return;
    // {
    //     acl_list_by_port_end& ip_end_index = table_ptr->get<ip_end>();
        
    //     std::pair<acl_list_by_ip_end::iterator, acl_list_by_ip_end::iterator> ppp;

    //     ppp = ip_end_index.range(100 <= boost::lambda::_1, boost::lambda::_1 >= 100);

    //     update_pointer(table_port_end_ptr);

    //     std::for_each(ppp.first, ppp.second, bind(&create_bbbb, _1));
    //     print_out_tbl_key(*table_ptr);
    // }

    //for_each(ppp.first, ppp.second, std::cout << boost::lambda::_1);
    // acl_list_by_ip_start::iterator aa = ppp.first;

    // cout << *aa<<endl;
    
    // std::for_each(ppp.first, ppp.second, bind(&create_aaaa, _1));
    //ACLContainer::insert
    //std::for_each(vec.begin(), vec.end(), bind(&plain_function, _1));
    // 
   // for_each(ppp.first, ppp.second, bind(&ACLContainer::insert,&ACLContainer::mycontainer_1,boost::lambda::_1));

    //for_each(ppp.first, ppp.second, bind(&acl_list_container::insert,table_ip_start,boost::lambda::_1));


    // for_each(ppp.first, ppp.second, std::cout << endl);

    // cout << "try to use lambda::_2......" << endl;
    // std::pair<acl_list_by_ip_start::iterator, acl_list_by_ip_end::iterator> pppp;
    // pppp = ip_start_index.range(120 == boost::lambda::_1, boost::lambda::_2 == 120);

    // for_each(pppp.first, pppp.second, std::cout << endl);

    // pppp = ip_start_index.range(120 == boost::lambda::_1, boost::lambda::_1 == 140);

    // acl_list_by_ip_end& ip_end_index = table.get<ip_start>();

    // std::pair<acl_list_by_ip_end::iterator, acl_list_by_ip_end::iterator> pppp;
    // pppp = ip_end_index.range(110 <= boost::lambda::_1, boost::lambda::_1 <= 110);

    // pp = name_index.range("AAA" <= boost::lambda::_1, boost::lambda::_1 <= "BBB");
    // for_each(pp.first, pp.second, std::cout << boost::lambda::_1 << ' ');

    // 	std::pair<acl_list_container::iterator, acl_list_container::iterator> p;
    // 	p = table.range(acl_list_db(100, "", "") <= boost::lambda::_1, boost::lambda::_1 <=
    // acl_list_db(200, "", "")); // 100<= x <=200 	for (acl_list_container::iterator it = p.first;
    // it != p.second; it++)
    // 	{
    // 		std::cout << *it << " , ";
    // 	}
    // 	std::cout << std::endl << std::endl;

    // typedef acl_list_container::index<tbl_id>::type acl_list_by_tbl_key;
    // acl_list_by_tbl_key& name_index = table.get<tbl_id>();

    // std::pair<acl_list_by_tbl_key::iterator, acl_list_by_tbl_key::iterator> pp;
    // pp = name_index.range("AAA" <= boost::lambda::_1, boost::lambda::_1 <= "BBB");
    // for_each(pp.first, pp.second, std::cout << boost::lambda::_1 << ' ');

    // acl_list_by_tbl_key::iterator it = name_index.find("Anna Jones");
    // name_index.modify_key(it, boost::lambda::_1 = "Anna Smith");
}

int
main(int argc, _TCHAR* argv[])
{
    acl_list_container table;
    create_stat(table);

    //print_out_tbl_key(table);
    
    select_stat(table);
    //暂停，输入任意键继续
    system("pause");
    return 0;
}