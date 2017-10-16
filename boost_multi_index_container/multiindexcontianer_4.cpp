/**
 * boost multi index container test
 * platform: win32, visual studio 2005/2010; Linux, gcc4.1.2
 */
#include <iostream>
#include "boost/multi_index_container.hpp"
#include "boost/multi_index/member.hpp"
#include "boost/multi_index/ordered_index.hpp"
#include "billing_option.h"

using namespace std;
using namespace boost::multi_index;
using boost::multi_index_container;

// define multiple index
typedef struct Data_Index
{
    int index1;
    int index2;
    int index3;
    int index4;

    Data_Index(int ax = 0, int ay = 0, int az = 0, int at = 0)
        : index1(ax), index2(ay), index3(az), index4(at)
    {
        
    }

    void
    print(const char *prompt) const
    {
        cout << "(" << index1 << ", " << index2 << ", " << index3 << ", " << index4 << ") - "
             << prompt << endl;
    }

} Data_Index;

#define BILLING_ADDRESS_LENGTH_MAX (128+1)

// define data to be indexed
typedef struct MultiContainerData
{
    BILLING_OPTIONS billing_option;
    char            billing_address[BILLING_ADDRESS_LENGTH_MAX];
    char            billing_address_backup[BILLING_ADDRESS_LENGTH_MAX];
    MultiContainerData_init()
    {
        billing_option = BILLING_OPTION_NONE;
        memset(&billing_address, 0, BILLING_ADDRESS_LENGTH_MAX);
        memset(&billing_address_backup, 0, BILLING_ADDRESS_LENGTH_MAX);
    };
} MultiContainerData;

// define object to be indexed
class DB_Indexed
{
public:
    Data_Index         m_Index;
    MultiContainerData m_data;

public:
    DB_Indexed(int index1, int index2, int index3, int index4, BILLING_OPTIONS bill_op, const char*bill_addr, const char *bill_addr_bk)
    {
        m_Index.index1 = index1;
        m_Index.index2 = index2;
        m_Index.index3 = index3;
        m_Index.index4 = index4;

        m_data.billing_option = bill_op;
        if (bill_addr != NULL)
        {
            strncpy(m_data.billing_address, bill_addr, BILLING_ADDRESS_LENGTH_MAX - 1);
        }

        if (bill_addr_bk != NULL)
        {
            strncpy(m_data.billing_address_backup, bill_addr_bk, BILLING_ADDRESS_LENGTH_MAX - 1);
        }
    }

    ~DB_Indexed() { print(", destructed"); }

    void
    print(const char *prompt = "") const
    {
        cout << "(" << m_Index.index1 << ", " << m_Index.index2 << ", " << m_Index.index3 << ", "
             << m_Index.index4 << ") - ";
        cout << "(" << billing_option_string[m_data.billing_option] << ", " << m_data.billing_address << ", " << m_data.billing_address_backup << ")" << prompt << endl;
    }

private:
    DB_Indexed(const DB_Indexed &);
    DB_Indexed &
    operator=(const DB_Indexed &);

};

// define index tag, multi_index_container, and its type
struct DB_Index_Tag
{

};

typedef multi_index_container<
    DB_Indexed *,
    indexed_by<
        ordered_unique<tag<DB_Index_Tag>, member<DB_Indexed, Data_Index, &DB_Indexed::m_Index> > > >
                                                           MyContainer_T;
typedef MyContainer_T::index<DB_Index_Tag>::type           MyContainerIndex_T;
typedef MyContainer_T::index<DB_Index_Tag>::type::iterator MyContainerIterator_T;
typedef std::pair<MyContainerIterator_T, bool> MyContainerPair_T;

// a template class
template <class MultiIndexContainer_T, class Tag_T, class Data_T, class Index_T>
class DBContainer
{
    MultiIndexContainer_T theContainer;

   public:
    void
    insert(Data_T *data);
    Data_T *
    find(const Index_T &index);
    void
    print();
    void
    free();
    void
    erase(const Index_T &index);
};

template <class MultiIndexContainer_T, class Tag_T, class Data_T, class Index_T>
void
DBContainer<MultiIndexContainer_T, Tag_T, Data_T, Index_T>::insert(Data_T *data)
{
    theContainer.insert(data);
}

template <class MultiIndexContainer_T, class Tag_T, class Data_T, class Index_T>
Data_T *
DBContainer<MultiIndexContainer_T, Tag_T, Data_T, Index_T>::find(const Index_T &index)
{
    const typename boost::multi_index::index<MultiIndexContainer_T, Tag_T>::type &indexSet =
        get<Tag_T>(theContainer);
    const typename boost::multi_index::index<MultiIndexContainer_T, Tag_T>::type::iterator iter =
        indexSet.find(index);
    if (indexSet.end() == iter)
    {
        index.print(const_cast<char *>(", not found"));
        return NULL;
    }
    (*iter)->print(", found");
    return (*iter);
}

template <class MultiIndexContainer_T, class Tag_T, class Data_T, class Index_T>
void
DBContainer<MultiIndexContainer_T, Tag_T, Data_T, Index_T>::print()
{
    const typename boost::multi_index::index<MultiIndexContainer_T, Tag_T>::type &indexSet =
        get<Tag_T>(theContainer);
    typedef typename MultiIndexContainer_T::value_type value_type;
    std::copy(indexSet.begin(), indexSet.end(), std::ostream_iterator<value_type>(cout));
}

template <class MultiIndexContainer_T, class Tag_T, class Data_T, class Index_T>
void
DBContainer<MultiIndexContainer_T, Tag_T, Data_T, Index_T>::free()
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

template <class MultiIndexContainer_T, class Tag_T, class Data_T, class Index_T>
void
DBContainer<MultiIndexContainer_T, Tag_T, Data_T, Index_T>::erase(const Index_T &index)
{
    const typename boost::multi_index::index<MultiIndexContainer_T, Tag_T>::type &indexSet =
        get<Tag_T>(theContainer);
    const typename boost::multi_index::index<MultiIndexContainer_T, Tag_T>::type::iterator iter =
        indexSet.find(index);

    if (indexSet.end() == iter)
    {
        index.print(const_cast<char *>(", not found"));
        return;
    }

    (*iter)->print(", found and erase");

    typedef typename MultiIndexContainer_T::value_type value_type;
    value_type pobj = *iter;
    theContainer.erase(iter);
    delete pobj;

    return;
}

bool
operator<(const Data_Index &lhs, const Data_Index &rhs)
{
    if (lhs.index1 < rhs.index1)
    {
        return true;
    }
    else if (lhs.index1 > rhs.index1)
    {
        return false;
    }
    else if (lhs.index2 < rhs.index2)
    {
        return true;
    }
    else if (lhs.index2 > rhs.index2)
    {
        return false;
    }
    else if (lhs.index3 < rhs.index3)
    {
        return true;
    }
    else if (lhs.index3 > rhs.index3)
    {
        return false;
    }
    else if (lhs.index4 < rhs.index4)
    {
        return true;
    }
    else if (lhs.index4 > rhs.index4)
    {
        return false;
    }

    return false;
}

std::ostream &
operator<<(std::ostream &os, const DB_Indexed *mytest)
{
    mytest->print();
    return os;
}

// instantiate a instance for this template class
DBContainer<MyContainer_T, DB_Index_Tag, DB_Indexed, Data_Index> mycontainer;
void
test1()
{
    DB_Indexed *a =
        new DB_Indexed(1, 1, 1, 1, BILLING_OPTION_NONE, "qa25a.none.billing.com", "qa25a.none.billingbackup.com");
    mycontainer.insert(a);
    DB_Indexed *b =
        new DB_Indexed(1, 1, 2, 1, BILLING_OPTION_RF, "qa25a.rf.billing.com", "qa25a.rf.billingbackup.com");
    mycontainer.insert(b);
    DB_Indexed *c =
        new DB_Indexed(1, 1, 3, 1, BILLING_OPTION_ICCF, "qa25a.iccf.billing.com", "qa25a.iccf.billingbackup.com");
    mycontainer.insert(c);
}

void
test2()
{
    DB_Indexed *a =
        new DB_Indexed(1, 2, 1, 1, BILLING_OPTION_NONE, "qa25a.none.billing.com", "qa25a.none.billingbackup.com");
    mycontainer.insert(a);
    DB_Indexed *b =
        new DB_Indexed(1, 2, 2, 1, BILLING_OPTION_RF, "qa25a.rf.billing.com", "qa25a.rf.billingbackup.com");
    mycontainer.insert(b);
    DB_Indexed *c =
        new DB_Indexed(1, 2, 3, 1, BILLING_OPTION_ICCF, "qa25a.iccf.billing.com", "qa25a.iccf.billingbackup.com");
    mycontainer.insert(c);
}

void
test3()
{
    DB_Indexed *a =
        new DB_Indexed(1, 3, 1, 1, BILLING_OPTION_NONE, "qa25a.none.billing.com", "qa25a.none.billingbackup.com");
    mycontainer.insert(a);
    DB_Indexed *b =
        new DB_Indexed(1, 3, 2, 1, BILLING_OPTION_RF, "qa25a.rf.billing.com", "qa25a.rf.billingbackup.com");
    mycontainer.insert(b);
    DB_Indexed *c =
        new DB_Indexed(1, 3, 3, 1, BILLING_OPTION_ICCF, "qa25a.iccf.billing.com", "qa25a.iccf.billingbackup.com");
    mycontainer.insert(c);
}

void
test4()
{
    DB_Indexed *a =
        new DB_Indexed(2, 1, 1, 1, BILLING_OPTION_NONE, "qa25a.none.billing.com", "qa25a.none.billingbackup.com");
    mycontainer.insert(a);
    DB_Indexed *b =
        new DB_Indexed(2, 1, 2, 1, BILLING_OPTION_RF, "qa25a.rf.billing.com", "qa25a.rf.billingbackup.com");
    mycontainer.insert(b);
    DB_Indexed *c =
        new DB_Indexed(2, 1, 3, 1, BILLING_OPTION_ICCF, "qa25a.iccf.billing.com", "qa25a.iccf.billingbackup.com");
    mycontainer.insert(c);
}

void
test5()
{
    DB_Indexed *a =
        new DB_Indexed(2, 2, 1, 1, BILLING_OPTION_NONE, "qa25a.none.billing.com", "qa25a.none.billingbackup.com");
    mycontainer.insert(a);
    DB_Indexed *b =
        new DB_Indexed(2, 2, 2, 1, BILLING_OPTION_RF, "qa25a.rf.billing.com", "qa25a.rf.billingbackup.com");
    mycontainer.insert(b);
    DB_Indexed *c =
        new DB_Indexed(2, 2, 3, 1, BILLING_OPTION_ICCF, "qa25a.iccf.billing.com", "qa25a.iccf.billingbackup.com");
    mycontainer.insert(c);
}

void
test6()
{
    DB_Indexed *a =
        new DB_Indexed(2, 3, 1, 1, BILLING_OPTION_NONE, "qa25a.none.billing.com", "qa25a.none.billingbackup.com");
    mycontainer.insert(a);
    DB_Indexed *b =
        new DB_Indexed(2, 3, 2, 1, BILLING_OPTION_RF, "qa25a.rf.billing.com", "qa25a.rf.billingbackup.com");
    mycontainer.insert(b);
    DB_Indexed *c =
        new DB_Indexed(2, 3, 3, 1, BILLING_OPTION_ICCF, "qa25a.iccf.billing.com", "qa25a.iccf.billingbackup.com");
    mycontainer.insert(c);
}

void
test_find()
{
    mycontainer.find(Data_Index(1, 1, 1, 1));
    mycontainer.find(Data_Index(1, 1, 2, 1));
    mycontainer.find(Data_Index(1, 1, 3, 1));
    mycontainer.find(Data_Index(1, 2, 1, 1));
    mycontainer.find(Data_Index(1, 2, 2, 1));
    mycontainer.find(Data_Index(1, 2, 3, 1));
    mycontainer.find(Data_Index(1, 3, 1, 1));
    mycontainer.find(Data_Index(1, 3, 2, 1));
    mycontainer.find(Data_Index(1, 3, 3, 1));
    mycontainer.find(Data_Index(2, 1, 1, 1));
    mycontainer.find(Data_Index(2, 1, 2, 1));
    mycontainer.find(Data_Index(2, 1, 3, 1));
    mycontainer.find(Data_Index(2, 2, 1, 1));
    mycontainer.find(Data_Index(2, 2, 2, 1));
    mycontainer.find(Data_Index(2, 2, 3, 1));
    mycontainer.find(Data_Index(2, 3, 1, 1));
    mycontainer.find(Data_Index(2, 3, 2, 1));
    mycontainer.find(Data_Index(2, 3, 3, 1));
    mycontainer.find(Data_Index(2, 3, 3, 0));
    mycontainer.find(Data_Index(2, 3, 3, 0));
    mycontainer.find(Data_Index(2, 3, 0, 1));

    mycontainer.erase(Data_Index(1, 3, 3, 1));
    mycontainer.find(Data_Index(1, 3, 3, 1));
}

int
main()
{
    test2();
    test4();
    test6();
    test1();
    test3();
    test5();
    mycontainer.print();
    cout << endl;
    test_find();
    cout << endl;

    DB_Indexed *found = mycontainer.find(Data_Index(2, 3, 3, 1));
    if (found != NULL)
    {
        cout << found->m_Index.index1 << ", "<< found->m_Index.index2 << ", " <<found->m_Index.index3
             << ", "<< found->m_Index.index4 << ", " <<found->m_data.billing_address << endl;
    }
    mycontainer.free();
    system("pause");
    return 0;
}