/**
 * boost multi index container test
 * platform: win32, visual studio 2005/2010; Linux, gcc4.1.2
 */
#include <iostream>
#include "boost/multi_index_container.hpp"
#include "boost/multi_index/member.hpp"
#include "boost/multi_index/ordered_index.hpp"
#include "billing_option.h"
#include "monitor_timer.h"
#include <windows.h>

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
    Data_Index         test;
    MultiContainerData m_data;

public:
    DB_Indexed(int index1, int index2, int index3, int index4, BILLING_OPTIONS bill_op, const char*bill_addr, const char *bill_addr_bk)
    {
        test.index1 = index1;
        test.index2 = index2;
        test.index3 = index3;
        test.index4 = index4;

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
        cout << "(" << test.index1 << ", " << test.index2 << ", " << test.index3 << ", "
             << test.index4 << ") - ";
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
        ordered_unique<tag<DB_Index_Tag>, member<DB_Indexed, Data_Index, &DB_Indexed::test> > > >
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

void
test_insert_multiindexcontainer(int count_max)
{
    START_MONITOR_TIMER();
    unsigned int count = 0;
    cout << "---INSERT DATA START---" << endl;
    for (int vn_id = 20; vn_id <= 1024; vn_id++)
    {

        for (int tg_id = 15; tg_id <= 32; tg_id++)
        {
            if (count >= count_max)
            {
                break;
            }
            for (int vn_id2 = 500; vn_id2 <= 1024; vn_id2++)
            {
                if (count >= count_max)
                {
                    break;
                }
                for (int tg_id2 = 0; tg_id2 <= 20; tg_id2++)
                {
                    if (count >= count_max)
                    {
                        break;
                    }
                    DB_Indexed *a =
                        new DB_Indexed(vn_id, tg_id, vn_id2, tg_id2, BILLING_OPTION_ICCF,
                                       "qa25a.none.billing.com", "qa25a.iccf.billingbackup.com");
                    count++;
                    if (a != NULL)
                    {
                        mycontainer.insert(a);
                        cout << "insert data: " << count << endl;
                    }
                }
            }
        }
    }
    
    cout << "---INSERT DATA COMPLETE---: " << count << endl;
    STOP_MONITOR_TIMER();
    system("pause");
    return;
}


void
test_time_find(int a, int b, int c, int d)
{
    START_MONITOR_TIMER();
    DB_Indexed *found = mycontainer.find(Data_Index(a, b, c, d));
    if (found != NULL)
    {
        cout << found->test.index1 << ", "<< found->test.index2 << ", " <<found->test.index3
             << ", "<< found->test.index4 << ", " <<found->m_data.billing_address << endl;
    }
    STOP_MONITOR_TIMER();
    //system("pause");
}

int arrary[500][32][500][32];

void
test_time_array(int a, int b, int c, int d)
{
    cout << "---TEST find ARRAY time START----" << endl;
    START_MONITOR_TIMER();

    cout << arrary[a][b][c][d] << endl;

    STOP_MONITOR_TIMER();
    cout << "---TEST find ARRAY time START----" << endl;    
    system("pause");
}

void test_find_multiindexcontainer(int count_max)
{
    unsigned int count = 0;
    cout << "---FOUND DATA START---" << endl;
    for (int vn_id = 100; vn_id <= 1024; vn_id++)
    {

        for (int tg_id = 4; tg_id <= 14; tg_id++)
        {
            if (count >= count_max)
            {
                break;
            }
            for (int vn_id2 = 20; vn_id2 <= 1024; vn_id2++)
            {
                if (count >= count_max)
                {
                    break;
                }
                for (int tg_id2 = 10; tg_id2 <= 32; tg_id2++)
                {
                    if (count >= count_max)
                    {
                        break;
                    }
                    count++;
                    test_time_find(vn_id, tg_id, vn_id2, tg_id2);
                    Sleep(1000);
                }
            }
        }
    }
    
    cout << "---FOUND DATA COMPLETE---: " << count << endl;
    system("pause");
    
    return;

}

void free_container()
{
    cout << "GO ON to FREE CONTAINER" << endl;
    mycontainer.free();
    cout << "GO ON to FREE CONTAINER" << endl;   
    system("pause");
    return;
}

int
main()
{
    /*
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
        cout << found->test.index1 << ", "<< found->test.index2 << ", " <<found->test.index3
             << ", "<< found->test.index4 << ", " <<found->m_data.billing_address << endl;
    }
    */

    test_time_array(200,20,100,10);
    test_time_array(200,20,100,10);
    test_time_array(200,20,100,10);
    test_time_array(199,20,100,10);
    test_time_array(190,20,100,10);
    
    test_insert_multiindexcontainer(10000);

    test_find_multiindexcontainer(10000);

    test_time_find(1, 2, 3, 4);
    test_time_find(0, 0, 3, 4);
    test_time_find(0, 0, 300, 4);
    test_time_find(0, 0, 300, 4);
    test_time_find(0, 0, 199, 32);
    test_time_find(4, 0, 500, 0);
    test_time_find(0, 0, 198, 4);
    test_time_find(0, 4, 1998, 4);

    free_container();

    return 0;
}