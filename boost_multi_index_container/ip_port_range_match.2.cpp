/**
 * boost multi index container test
 * platform: win32, visual studio 2005/2010; Linux, gcc4.1.2
 */
#include <iostream>
#include "boost/multi_index_container.hpp"
#include "boost/multi_index/member.hpp"
#include "boost/multi_index/ordered_index.hpp"
#include "range_index.h"

using namespace std;
using namespace boost::multi_index;
using boost::multi_index_container;

class acl_list_address_index
{
   public:
    unsigned int ip_prefix;
    range_index  ip_suffix_range;
    range_index  port_range;
    acl_list_address_index()
    {
        ip_prefix                   = 0;
        ip_suffix_range.index_start = 0;
        ip_suffix_range.index_end   = 0;
        port_range.index_start      = 0;
        port_range.index_end        = 0;
    }
    acl_list_address_index(unsigned int ip_pre, range_index ip_rang, range_index port_rang)
        : ip_prefix(ip_pre), ip_suffix_range(ip_rang), port_range(port_rang)
    {
    }
    void
    print(char *prompt) const
    {
        cout << ip_prefix << ", "
             << "(" << ip_suffix_range.index_start << ", " << ip_suffix_range.index_end << ")"
             << "-(" << port_range.index_start << ", " << port_range.index_end << ")"
             << " - " << prompt << endl;
    }
};

// define data to be indexed
class acl_list_attributes
{
   public:
    char reserved;
    acl_list_attributes(){}
    acl_list_attributes
    operator=(const acl_list_attributes &obj)
    {
        acl_list_attributes target;
        target.reserved = obj.reserved;
        return (target);
    }
};

// define object to be indexed
class acl_list_db
{
   public:
    acl_list_address_index myIndex;
    acl_list_attributes    myData;

   public:
    acl_list_db(unsigned int ip_pre, range_index ip_suf_rang, range_index port_rang, acl_list_attributes acl_list_attr)
    {
        myIndex.ip_prefix = ip_pre;
        myIndex.ip_suffix_range = ip_suf_rang;
        myIndex.port_range      = port_rang;
        myData                  = acl_list_attr;
    }

    ~acl_list_db() { print(", destructed"); }
    
    void
    print( const char *prompt = "") const
    {
        cout << "(" 
            << myIndex.ip_prefix << ", " 
            << myIndex.ip_suffix_range.index_start << ", " 
            << myIndex.ip_suffix_range.index_end << ", " 
            << myIndex.port_range.index_start << ", " 
            << myIndex.port_range.index_end << ") ---";
        cout << prompt << endl;
        // cout << "(" << myData.a << ", " << myData.b << ")" << prompt << endl;
    }

   private:
    acl_list_db(const acl_list_db &);
    acl_list_db &
    operator=(const acl_list_db &);
};

// define index tag, multi_index_container, and its type
struct MyIndexTag
{
};

typedef multi_index_container<
    acl_list_db *,
    indexed_by<ordered_unique<tag<MyIndexTag>, member<acl_list_db, acl_list_address_index, &acl_list_db::myIndex> > > >
                                                         MyContainer_T;
typedef MyContainer_T::index<MyIndexTag>::type           MyContainerIndex_T;
typedef MyContainer_T::index<MyIndexTag>::type::iterator MyContainerIterator_T;
typedef std::pair<MyContainerIterator_T, bool> MyContainerPair_T;

// a template class
template <class MultiIndexContainer_T, class Tag_T, class Data_T, class Index_T>
class MyContainer
{
    MultiIndexContainer_T theContainer;

   public:
    void
    insert(Data_T *data);
    void
    find(const Index_T &index);
    void
    print();
    void
    free();
};

template <class MultiIndexContainer_T, class Tag_T, class Data_T, class Index_T>
void
MyContainer<MultiIndexContainer_T, Tag_T, Data_T, Index_T>::insert(Data_T *data)
{
    theContainer.insert(data);
}

template <class MultiIndexContainer_T, class Tag_T, class Data_T, class Index_T>
void
MyContainer<MultiIndexContainer_T, Tag_T, Data_T, Index_T>::find(const Index_T &index)
{
    const typename boost::multi_index::index<MultiIndexContainer_T, Tag_T>::type &indexSet =
        get<Tag_T>(theContainer);
    const typename boost::multi_index::index<MultiIndexContainer_T, Tag_T>::type::iterator iter =
        indexSet.find(index);
    if (indexSet.end() == iter)
    {
        index.print(const_cast<char*>("not found"));
        return;
    }
    (*iter)->print(", found");
}

template <class MultiIndexContainer_T, class Tag_T, class Data_T, class Index_T>
void
MyContainer<MultiIndexContainer_T, Tag_T, Data_T, Index_T>::print()
{
    const typename boost::multi_index::index<MultiIndexContainer_T, Tag_T>::type &indexSet =
        get<Tag_T>(theContainer);
    typedef typename MultiIndexContainer_T::value_type value_type;
    std::copy(indexSet.begin(), indexSet.end(), std::ostream_iterator<value_type>(cout));
}

template <class MultiIndexContainer_T, class Tag_T, class Data_T, class Index_T>
void
MyContainer<MultiIndexContainer_T, Tag_T, Data_T, Index_T>::free()
{
    typedef typename MultiIndexContainer_T::value_type value_type;
    while (!theContainer.empty())
    {
        typename MultiIndexContainer_T::iterator iter = theContainer.begin();
        if (NULL == (*iter))
        {
            theContainer.erase(iter);
            continue;
        }
        value_type pobj = *iter;
        theContainer.erase(iter);
        delete pobj;
    }
}

bool
operator<(const acl_list_address_index &lhs, const acl_list_address_index &rhs)
{
    if (lhs.ip_prefix < rhs.ip_prefix)
        return true;
    else if (lhs.ip_prefix > rhs.ip_prefix)
        return false;
    else if (lhs.ip_suffix_range.index_start < rhs.ip_suffix_range.index_start)
        return true;
    else if (lhs.ip_suffix_range.index_start > rhs.ip_suffix_range.index_start)
        return false;
    else if (lhs.port_range.index_start < rhs.port_range.index_start)
        return true;
    else if (lhs.port_range.index_start > rhs.port_range.index_start)
        return false;
    else
        return false;
}

std::ostream &
operator<<(std::ostream &os, const acl_list_db *mytest)
{
    mytest->print();
    return os;
}

// instantiate a instance for this template class
MyContainer<MyContainer_T, MyIndexTag, acl_list_db, acl_list_address_index> mycontainer;

void
test_insert_2(unsigned int ip_prefix, unsigned int a_index, unsigned int b_index)
{
    acl_list_db *a         = NULL;

    a = new acl_list_db(ip_prefix, range_index(a_index, b_index), range_index(1240, 1260),
                        acl_list_attributes());
    mycontainer.insert(a);
    a = new acl_list_db(ip_prefix, range_index(a_index, b_index), range_index(1220, 1220),
                        acl_list_attributes());
    mycontainer.insert(a);
    a = new acl_list_db(ip_prefix, range_index(a_index, b_index), range_index(1220, 1240),
                        acl_list_attributes());
    mycontainer.insert(a);
    a = new acl_list_db(ip_prefix, range_index(a_index, b_index), range_index(1220, 1250),
                        acl_list_attributes());
    mycontainer.insert(a);
    a = new acl_list_db(ip_prefix, range_index(a_index, b_index), range_index(1220, 1260),
                        acl_list_attributes());
    mycontainer.insert(a);
    a = new acl_list_db(ip_prefix, range_index(a_index, b_index), range_index(1220, 1270),
                        acl_list_attributes());
    mycontainer.insert(a);
    a = new acl_list_db(ip_prefix, range_index(a_index, b_index), range_index(1240, 1240),
                        acl_list_attributes());
    mycontainer.insert(a);
    a = new acl_list_db(ip_prefix, range_index(a_index, b_index), range_index(1240, 1250),
                        acl_list_attributes());
    mycontainer.insert(a);
    a = new acl_list_db(ip_prefix, range_index(a_index, b_index), range_index(1240, 1270),
                        acl_list_attributes());
    mycontainer.insert(a);
    a = new acl_list_db(ip_prefix, range_index(a_index, b_index), range_index(1260, 1260),
                        acl_list_attributes());
    mycontainer.insert(a);
    a = new acl_list_db(ip_prefix, range_index(a_index, b_index), range_index(1260, 1270),
                        acl_list_attributes());
    mycontainer.insert(a);
    a = new acl_list_db(ip_prefix, range_index(a_index, b_index), range_index(1270, 1270),
                        acl_list_attributes());
    mycontainer.insert(a);
    a = new acl_list_db(ip_prefix, range_index(a_index, b_index), range_index(1270, 1280),
                        acl_list_attributes());
    mycontainer.insert(a);
    cout << "function test 2 insert..... done!" << endl;
}

void
test_insert_1(unsigned int ip_prefix, unsigned int port_start, unsigned port_end)
{
    // acl_list_db(unsigned int ip_pre, range_index ip_suf_rang, range_index port_rang,
    // acl_list_attributes acl_list_attr)
    acl_list_db *a         = NULL;

    a = new acl_list_db(ip_prefix, range_index(4, 10), range_index(port_start, port_end),
                        acl_list_attributes());
    mycontainer.insert(a);
    test_insert_2(ip_prefix, 4, 10);
    //
    a = new acl_list_db(ip_prefix, range_index(1, 1), range_index(port_start, port_end),
                        acl_list_attributes());
    mycontainer.insert(a);
    //
    a = new acl_list_db(ip_prefix, range_index(1, 2), range_index(port_start, port_end),
                        acl_list_attributes());
    mycontainer.insert(a);
    //
    a = new acl_list_db(ip_prefix, range_index(1, 4), range_index(port_start, port_end),
                        acl_list_attributes());
    mycontainer.insert(a);
    //
    a = new acl_list_db(ip_prefix, range_index(1, 6), range_index(port_start, port_end),
                        acl_list_attributes());
    mycontainer.insert(a);
    //
    a = new acl_list_db(ip_prefix, range_index(1, 10), range_index(port_start, port_end),
                        acl_list_attributes());
    mycontainer.insert(a);
    //
    a = new acl_list_db(ip_prefix, range_index(1, 11), range_index(port_start, port_end),
                        acl_list_attributes());
    mycontainer.insert(a);
    //
    a = new acl_list_db(ip_prefix, range_index(4, 4), range_index(port_start, port_end),
                        acl_list_attributes());
    mycontainer.insert(a);
    //
    a = new acl_list_db(ip_prefix, range_index(4, 6), range_index(port_start, port_end),
                        acl_list_attributes());
    mycontainer.insert(a);
    //
    a = new acl_list_db(ip_prefix, range_index(4, 11), range_index(port_start, port_end),
                        acl_list_attributes());
    mycontainer.insert(a);
    //
    a = new acl_list_db(ip_prefix, range_index(5, 5), range_index(port_start, port_end),
                        acl_list_attributes());
    mycontainer.insert(a);
    //
    a = new acl_list_db(ip_prefix, range_index(5, 6), range_index(port_start, port_end),
                        acl_list_attributes());
    mycontainer.insert(a);
    //
    a = new acl_list_db(ip_prefix, range_index(5, 10), range_index(port_start, port_end),
                        acl_list_attributes());
    mycontainer.insert(a);
    //
    a = new acl_list_db(ip_prefix, range_index(5, 11), range_index(port_start, port_end),
                        acl_list_attributes());
    mycontainer.insert(a);
    //
    a = new acl_list_db(ip_prefix, range_index(10, 10), range_index(port_start, port_end),
                        acl_list_attributes());
    mycontainer.insert(a);
    //
    a = new acl_list_db(ip_prefix, range_index(10, 11), range_index(port_start, port_end),
                        acl_list_attributes());
    mycontainer.insert(a);
    //
    a = new acl_list_db(ip_prefix, range_index(11, 11), range_index(port_start, port_end),
                        acl_list_attributes());
    mycontainer.insert(a);
    //
    a = new acl_list_db(ip_prefix, range_index(11, 12), range_index(port_start, port_end),
                        acl_list_attributes());
    mycontainer.insert(a);
    //
    a = new acl_list_db(ip_prefix, range_index(11, 11), range_index(port_start, port_end),
                        acl_list_attributes());
    mycontainer.insert(a);
    //
    a = new acl_list_db(ip_prefix, range_index(16, 16), range_index(port_start, port_end),
                        acl_list_attributes());
    mycontainer.insert(a);
    //
    a = new acl_list_db(ip_prefix, range_index(1, 254), range_index(port_start, port_end),
                        acl_list_attributes());
    mycontainer.insert(a);
    //
    a = new acl_list_db(ip_prefix, range_index(254, 254), range_index(port_start, port_end),
                        acl_list_attributes());
    mycontainer.insert(a);
}

void
test_find()
{
    mycontainer.find(acl_list_address_index(172111001,  range_index(1, 1), range_index(10, 100)));
}

int
main()
{

    mycontainer.print();
    acl_list_db *a         = NULL;

    a = new acl_list_db(172018000, range_index(1, 5), range_index(1230, 1240),
                        acl_list_attributes());
    mycontainer.insert(a);
    a = new acl_list_db(172018000, range_index(0, 3), range_index(1250, 1260),
                        acl_list_attributes());
    mycontainer.insert(a);
    a = new acl_list_db(172018000, range_index(4, 8), range_index(2000, 3000),
                        acl_list_attributes());
    mycontainer.insert(a);

    mycontainer.find(acl_list_address_index(172018000,  range_index(2, 2), range_index(1230, 1240)));

    // cout << endl;
    // mycontainer.free();
    system("pause");
    return 0;
}