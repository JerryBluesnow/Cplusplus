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

class range_index{
public:
    unsigned int index_start;
    unsigned int index_end;

    range_index() : index_start(0), index_end(0)
    {
        //
    }
    range_index(unsigned int start, unsigned int end) : index_start(start), index_end(end)
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

/*
bool
operator==(const range_index &lhs, const range_index &obj)  //重载 ==
{
    // when start==end, it indicates the struct is a value, not a range!
    if (lhs.index_start == lhs.index_end)
    {
        return ((lhs.index_start >= obj.index_start) && (lhs.index_end <= obj.index_end));
    }

    // when start==end, it indicates the struct is a value, not a range!
    if (obj.index_start == obj.index_end)
    {
        return ((lhs.index_start <= obj.index_start) && (lhs.index_end >= obj.index_end));
    }

    // if above conditions aren't satisfied, it indicates two range are compared!
    return (lhs.index_start == obj.index_start && lhs.index_end == obj.index_end);
}
*/

bool
operator<(const range_index &lhs, const range_index &obj)  //重载 <
{
    if ((lhs.index_start == obj.index_start) && (lhs.index_end == obj.index_end))
    {
        return (false);
    }

    // when left is a pointer, and the right is also a pointer
    if ((lhs.index_start == lhs.index_end) && (obj.index_start == obj.index_end))
    {
        if (lhs.index_start < obj.index_start)
        {
            return (true);
        }

        return (false);
    }

    // when left is a pointer, and the right is line
    if ((lhs.index_start == lhs.index_end) && (obj.index_start != obj.index_end))
    {
        // when the left pointer is in left of the right line
        if (lhs.index_end < obj.index_start)
        {
            return (true);
        }
        
        return (false);
    }

    // when left is a line, and the right is pointer
    if ((lhs.index_start != lhs.index_end) && (obj.index_start == obj.index_end))
    {
        // when the end index of left line is less than the right pointer start_index
        if (lhs.index_end < obj.index_start)
        {
            return (true);
        }
        
        return (false);
    }

    // when left is a line, and the right is also a line
    if ((lhs.index_start != lhs.index_end) && (obj.index_start == obj.index_end))
    {
        // when the start index of left line is less than the right pointer start_index
        if (lhs.index_start < obj.index_start)
        {
            return (true);
        }

        // when the start of the both are equal, compare the end index
        if (lhs.index_start == obj.index_start)
        {
            if (lhs.index_end < obj.index_end)
            {
                return (true);
            }
        }
 
        return (false);
    }

    return (false);
}

bool
operator>(const range_index &lhs, const range_index &obj)  //重载 >
{
    if ((lhs.index_start == obj.index_start) && (lhs.index_end == obj.index_end))
    {
        return (false);
    }

    // when left is a pointer, and the right is also a pointer
    if ((lhs.index_start == lhs.index_end) && (obj.index_start == obj.index_end))
    {
        if (lhs.index_start > obj.index_start)
        {
            return (true);
        }

        return (false);
    }

    // when left is a pointer, and the right is line
    if ((lhs.index_start == lhs.index_end) && (obj.index_start != obj.index_end))
    {
        // when the left pointer is in left of the right line
        if (lhs.index_start > obj.index_end)
        {
            return (true);
        }
        
        return (false);
    }

    // when left is a line, and the right is pointer
    if ((lhs.index_start != lhs.index_end) && (obj.index_start == obj.index_end))
    {
        // when the end index of left line is less than the right pointer start_index
        if (lhs.index_start > obj.index_end)
        {
            return (true);
        }
        
        return (false);
    }

    // when left is a line, and the right is also a line
    if ((lhs.index_start != lhs.index_end) && (obj.index_start == obj.index_end))
    {
        // when the start index of left line is less than the right pointer start_index
        if (lhs.index_start > obj.index_start)
        {
            return (true);
        }

        // when the start of the both are equal, compare the end index
        if (lhs.index_start == obj.index_start)
        {
            if (lhs.index_end > obj.index_end)
            {
                return (true);
            }
        }
 
        return (false);
    }

    return (false);
}

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
    int a;
    int b;
    acl_list_attributes(){}
    acl_list_attributes
    operator=(const acl_list_attributes &obj)
    {
        acl_list_attributes target;
        target.a = obj.a;
        target.b = obj.b;
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
    acl_list_db(unsigned int        ip_pre,
                range_index         ip_suf_rang,
                range_index         port_rang,
                acl_list_attributes acl_list_attr,
                unsigned int        _id)
    {
        myIndex.ip_prefix       = ip_pre;
        myIndex.ip_suffix_range = ip_suf_rang;
        myIndex.port_range      = port_rang;
        myData                  = acl_list_attr;
        id                      = _id;
    }

    ~acl_list_db() { print(", destructed"); }

    void
    print(const char *prompt = "") const
    {
        cout << "(" << myIndex.ip_prefix << ", " << myIndex.ip_suffix_range.index_start << ", "
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

struct MyIndex_X_Tag{};  
struct MyIndex_XY_Tag{};  
typedef multi_index_container<  
    acl_list_db*,  
    indexed_by<  
        ordered_non_unique<tag<MyIndex_X_Tag>, x_key, composite_key_result_less<x_key::result_type> >,  
        ordered_non_unique<tag<MyIndex_XY_Tag>, xy_key, composite_key_result_less<xy_key::result_type> >
    >  
>MyContainer_T;  

typedef nth_index<MyContainer_T, 0>::type MyContainer_X_T;  
typedef nth_index<MyContainer_T, 1>::type MyContainer_XY_T;  
typedef MyContainer_T::index<MyIndex_X_Tag>::type MyContainerIndex_X_T;  
typedef MyContainer_T::index<MyIndex_X_Tag>::type::iterator MyContainerIterator_X_T;  
typedef std::pair<MyContainerIterator_X_T, bool> MyContainerPair_X_T;  
typedef MyContainer_T::index<MyIndex_XY_Tag>::type MyContainerIndex_XY_T;  
typedef MyContainer_T::index<MyIndex_XY_Tag>::type::iterator MyContainerIterator_XY_T;  
typedef std::pair<MyContainerIterator_XY_T, bool> MyContainerPair_XY_T;  

//a template class  
template <class MultiIndexContainer_T, class Tag_T, class Data_T, class Index_T, class Index_EX_T>  
class MyContainer  
{  
private:
    MultiIndexContainer_T theContainer;  
public:  
    void insert(Data_T* data);  
    void find(const Index_T& index); 
    void find(const Index_T& index, const Index_EX_T& index_ex);  
    void print();  
    void free();
};  

//MyContainer_T mycontainer;  
//MyContainer_X_T& mycontainerx = mycontainer;  
//MyContainer_XY_T& mycontainerxy = get<1>(mycontainer);  

MyContainer<MyContainer_T, MyIndex_X_Tag, acl_list_db, acl_list_address_index, unsigned int> mycontainer;  
MyContainer<MyContainer_X_T, MyIndex_X_Tag, acl_list_db, acl_list_address_index, unsigned int> &mycontainerx = mycontainer;  
//MyContainer<MyContainer_XY_T, MyIndex_XY_Tag, acl_list_db, acl_list_address_index, unsigned int> &mycontainerxy = get<1>mycontainer;  

bool
operator<(const acl_list_address_index &lhs, const acl_list_address_index &rhs)
{
    if (lhs.ip_prefix < rhs.ip_prefix)
        return true;
        
    if (lhs.ip_prefix > rhs.ip_prefix)
        return false;
    
    if (lhs.ip_suffix_range < rhs.ip_suffix_range)
        return true;

    if (lhs.ip_suffix_range > rhs.ip_suffix_range)
        return false;

    if (lhs.port_range < rhs.port_range)
        return true;

    if (lhs.port_range > rhs.port_range)
        return false;

    return false;
}

template <class MultiIndexContainer_T, class Tag_T, class Data_T, class Index_T, class Index_EX_T>  
void MyContainer<MultiIndexContainer_T, Tag_T, Data_T, Index_T, Index_EX_T>::insert(Data_T* data)  
{  
    theContainer.insert(data);  
}  

template <class MultiIndexContainer_T, class Tag_T, class Data_T, class Index_T, class Index_EX_T>  
void MyContainer<MultiIndexContainer_T, Tag_T, Data_T, Index_T, Index_EX_T>::find(const Index_T& my_index)  
{
    MyContainerIterator_X_T it0, it1;
    boost::tie(it0, it1) = mycontainerx.equal_range(boost::make_tuple(my_index));

    if (*it0 == *it1)
    {
        // cout << my_index.ip_prefix <<", (" <<my_index.ip_suffix_range.index_start<<", "
        //      << my_index.ip_suffix_range.index_end<<"), (" << my_index.port_range.index_start<<",
        //      "
        //      << my_index.port_range.index_end<<") " << "- not found" << endl;
        return;
    }
    while (it0 != it1)
    {
        (*it0)->print(", found");
        ++it0;
    }
}  

template <class MultiIndexContainer_T, class Tag_T, class Data_T, class Index_T, class Index_EX_T>  
void MyContainer<MultiIndexContainer_T, Tag_T, Data_T, Index_T, Index_EX_T>::find(const Index_T my_index, const Index_EX_T my_id) 
{
//    MyContainerIterator_XY_T it0, it1;
//    boost::tie(it0, it1) = mycontainerxy.equal_range(boost::make_tuple(my_index, my_id));

//    if (*it0 == *it1)
//    {
//        cout << my_id << ", " << my_index.ip_prefix << ", (" << my_index.ip_suffix_range.index_start
//             << ", " << my_index.ip_suffix_range.index_end << "), ("
//             << my_index.port_range.index_start << ", " << my_index.port_range.index_end << ") "
//             << "- not found" << endl;
//        return;
//    }
//    while (it0 != it1)
//    {
//        (*it0)->print(", found");

//        ++it0;
//    }
}  

template <class MultiIndexContainer_T, class Tag_T, class Data_T, class Index_T, class Index_EX_T>  
void MyContainer<MultiIndexContainer_T, Tag_T, Data_T, Index_T, Index_EX_T>::print()  
{  
    // //obtain a reference to the index tagged by Tag  
    // const typename boost::multi_index::index<MultiIndexContainer_T, Tag_T>::type& indexSet = get<Tag_T>(container);  
    // typedef typename MultiIndexContainer_T::value_type value_type;  
    // //dump the elements of the index to cout  
    // std::copy(indexSet.begin(), indexSet.end(), std::ostream_iterator<value_type>(cout));  
}  

template <class MultiIndexContainer_T, class Tag_T, class Data_T, class Index_T, class Index_EX_T>  
void MyContainer<MultiIndexContainer_T, Tag_T, Data_T, Index_T, Index_EX_T>::free()  
{
    // cout << "try to print the container --:" << endl;
    // typedef typename MultiIndexContainer_T::value_type value_type;
    // while (!container.empty())
    // {
    //     typename MultiIndexContainer_T::iterator iter = container.begin();
    //     if (NULL == (*iter))
    //     {
    //         container.erase(iter);
    //         continue;
    //     }
    //     value_type pobj = *iter;
    //     container.erase(iter);
    //     delete pobj;
    // }
}  
void test1()  
{
    acl_list_db *a = NULL;

    // a = new acl_list_db(172018000, range_index(1, 5), range_index(1230, 1240), acl_list_attributes(), 1);
    // mycontainer.insert(a);  
    // a = new acl_list_db(172018000, range_index(0, 3), range_index(1250, 1260), acl_list_attributes(), 2);
    // mycontainer.insert(a);  
    // a = new acl_list_db(172018000, range_index(4, 8), range_index(2000, 4000), acl_list_attributes(), 1721);
    // mycontainer.insert(a);  
    //print_container<MyIndex_X_Tag>(mycontainer);    
}  

int main()  
{
    //  test1();
    //  find(acl_list_address_index(172018000, range_index(1, 5), range_index(1230, 1240)), 1);
    //  find(acl_list_address_index(172018000, range_index(1, 5), range_index(1230, 1240)));
    //  find(acl_list_address_index(172018000, range_index(0, 3), range_index(1250, 1260)), 2);
    //  find(acl_list_address_index(172018000, range_index(0, 3), range_index(1250, 1260)));
    //  find(acl_list_address_index(172018000, range_index(4, 8), range_index(2000, 4000)), 1722);
    //  find(acl_list_address_index(172018000, range_index(4, 8), range_index(2000, 4000)));

    //  cout << "try to find [172018000, range_index(1, 1), range_index(1257, 1257)] --:"
    //       << endl;
    
    //  find(acl_list_address_index(172018000, range_index(1, 1), range_index(1257, 1257)));

     //cout << "try to print the container --:" << endl;
     //print_container<MyIndex_X_Tag>(mycontainer);
     // cout<<endl;
     // print_container<MyIndex_XY_Tag>(mycontainer);
     // cout<<endl;
     // print_container<MyIndex_XYZ_Tag>(mycontainer);
     // cout<<endl;r
     // query_test();5
     // cout<<endl;

    //  free_container(mycontainer);
     system("pause");
     return 0;  
}  
