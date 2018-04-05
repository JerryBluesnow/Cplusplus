#include <list>
#include <string>

#include <boost/multi_index_container.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/identity.hpp>
#include <boost/multi_index/member.hpp>

#include <boost/lambda/lambda.hpp>
#include <iostream>

using namespace std;

#define UINT unsigned int

typedef struct composite_key
{
    UINT tbl_id;
    UINT id;
    composite_key(UINT tbl_id_, UINT id_)
    {
        tbl_id = tbl_id_;
        id     = id_;
    }
} composite_key;

bool
operator<(const composite_key& key_1, const composite_key& key_2)
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
operator>(const composite_key& key_1, const composite_key& key_2)
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
operator==(const composite_key& key_1, const composite_key& key_2)
{
    return ((!(key_1 < key_2)) && (!(key_1 > key_2)));
}

bool
operator<=(const composite_key& key_1, const composite_key& key_2)
{
    return (!(key_1 > key_2));
}
bool
operator>=(const composite_key& key_1, const composite_key& key_2)
{
    return (!(key_1 < key_2));
}


		struct tbl_key{};
		//struct id{};
		struct ip_start{};
		struct ip_end{};
		struct port_start{};
		struct port_end{};
		struct vlan_id{};

    struct acl_lis_db
		{
        composite_key _tbl_key;
        UINT _ip_start;
        UINT _ip_end;
        UINT _port_start;
        UINT _port_end;
        UINT _vlan_id;

        acl_lis_db(composite_key tbl_key,
                   UINT ip_start,
                   UINT ip_end,
                   UINT port_start,
                   UINT port_end,
                   UINT vlan_id)
            : _tbl_key(tbl_key),
              _ip_start(ip_start),
              _ip_end(ip_end),
              _port_start(port_start),
              _port_end(port_end),
              _vlan_id(vlan_id)
        {

        }

        bool
        operator<(const acl_lis_db& e) const
        {
            return _tbl_key < e._tbl_key;
        }
        bool
        operator<=(const acl_lis_db& e) const
        {
            return _tbl_key <= e._tbl_key;
        }

        friend std::ostream&
        operator<<(std::ostream& os, const acl_lis_db& dt);
    };

    std::ostream&
    operator<<(std::ostream& os, const acl_lis_db& dt)
    {
        os << "[" 
		   << dt._tbl_key.tbl_id << ',' 
		   << dt._tbl_key.id << ','
		   << dt._ip_start << ','
           << dt._ip_end << ',' 
		   << dt._port_start << ',' 
		   << dt._ip_end << ',' 
		   << dt._vlan_id << "]";
        return os;
    }

    typedef boost::multi_index::multi_index_container<
			acl_lis_db,
			boost::multi_index::indexed_by<
			boost::multi_index::ordered_unique<boost::multi_index::tag<tbl_key>, boost::multi_index::identity<acl_lis_db> >,
			boost::multi_index::ordered_non_unique<boost::multi_index::tag<ip_start>, boost::multi_index::member<acl_lis_db, UINT, &acl_lis_db::_ip_start> >,
			boost::multi_index::ordered_non_unique<boost::multi_index::tag<ip_end>, boost::multi_index::member<acl_lis_db, UINT, &acl_lis_db::_ip_end> >,
			boost::multi_index::ordered_non_unique<boost::multi_index::tag<port_start>, boost::multi_index::member<acl_lis_db, UINT, &acl_lis_db::_port_start> >,
			boost::multi_index::ordered_non_unique<boost::multi_index::tag<port_end>, boost::multi_index::member<acl_lis_db, UINT, &acl_lis_db::_port_end> >,
			boost::multi_index::ordered_non_unique<boost::multi_index::tag<vlan_id>, boost::multi_index::member<acl_lis_db, UINT, &acl_lis_db::_vlan_id> >
			>

		> acl_list_container;

void print_out_by_tbl_key(const acl_list_container& acl_tbl)
{
	const acl_list_container::index<tbl_key>::type& tbl_key_index = acl_tbl.get<tbl_key>();
	
	std::copy(
		tbl_key_index.begin(), tbl_key_index.end(),
		std::ostream_iterator<acl_lis_db, char>(std::cout));
	std::cout << std::endl << std::endl;
}

typedef acl_list_container::index<ip_start>::type acl_list_by_ip_start;
typedef acl_list_container::index<ip_end>::type acl_list_by_ip_end;

void select_stat(acl_list_container& table)
{
	typedef acl_list_container::index<tbl_key>::type acl_list_by_tbl_key;
	acl_list_by_tbl_key& tbl_key_index = table.get<tbl_key>();

	std::pair<acl_list_by_tbl_key::iterator, acl_list_by_tbl_key::iterator> pp;

    composite_key key1(20, 0);
    composite_key key2(20, 0);

   // pp = tbl_key_index.range(key1 <= boost::lambda::_1, boost::lambda::_1 <= key2);

    acl_list_by_ip_start& ip_start_index = table.get<ip_start>();

	std::pair<acl_list_by_ip_start::iterator, acl_list_by_ip_start::iterator> ppp;
	ppp = ip_start_index.range(1 <= boost::lambda::_1, boost::lambda::_1 <= 2);

    acl_list_by_ip_end& ip_end_index = table.get<ip_end>();

	std::pair<acl_list_by_ip_end::iterator, acl_list_by_ip_end::iterator> pppp;
	pppp = ip_end_index.range(1 <= boost::lambda::_1, boost::lambda::_1 <= 2);

	//pp = name_index.range("AAA" <= boost::lambda::_1, boost::lambda::_1 <= "BBB");
	//for_each(pp.first, pp.second, std::cout << boost::lambda::_1 << ' ');

	// 	std::pair<acl_list_container::iterator, acl_list_container::iterator> p;
	// 	p = table.range(acl_lis_db(100, "", "") <= boost::lambda::_1, boost::lambda::_1 <= acl_lis_db(200, "", "")); // 100<= x <=200
	// 	for (acl_list_container::iterator it = p.first; it != p.second; it++)
	// 	{
	// 		std::cout << *it << " , ";
	// 	}
	// 	std::cout << std::endl << std::endl;



	// typedef acl_list_container::index<tbl_id>::type acl_list_by_tbl_key;
	// acl_list_by_tbl_key& name_index = table.get<tbl_id>();

	// std::pair<acl_list_by_tbl_key::iterator, acl_list_by_tbl_key::iterator> pp;
	// pp = name_index.range("AAA" <= boost::lambda::_1, boost::lambda::_1 <= "BBB");
	// for_each(pp.first, pp.second, std::cout << boost::lambda::_1 << ' ');

	//acl_list_by_tbl_key::iterator it = name_index.find("Anna Jones");
	//name_index.modify_key(it, boost::lambda::_1 = "Anna Smith");
}



int main(int argc, _TCHAR* argv[])
{
	acl_list_container table;

	//暂停，输入任意键继续
	system("pause");
	return 0;
}