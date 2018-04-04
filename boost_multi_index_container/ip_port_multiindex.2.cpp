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


template<class T>
bool
operator<(const range_index<T> &lhs, const range_index<T> &obj)  //重载 <
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
    if ((lhs.index_start != lhs.index_end) && (obj.index_start != obj.index_end))
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

template<class T>
bool
operator==(const range_index<T> &lhs, const range_index<T> &obj)  //重载 >
{
    if ((lhs.index_start == obj.index_start) && (lhs.index_end == obj.index_end))
    {
        return (true);
    }

    if ((lhs.index_start >= obj.index_start) && (lhs.index_end <= obj.index_end))
    {
        return (true);
    }

    if ((lhs.index_start <= obj.index_start) && (lhs.index_end >= obj.index_end))
    {
        return (true);
    }

    return (false);
}

template<class T>
bool
operator>(const range_index<T> &lhs, const range_index<T> &obj)  //重载 >
{
    if ((!(lhs < obj)) && (!(lhs == obj)))
    {
        return (true);
    }
    return (false);
    #if 0
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
    if ((lhs.index_start != lhs.index_end) && (obj.index_start != obj.index_end))
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
    #endif // #if 0

}

class acl_list_address_index
{
   public:
    unsigned int ip_prefix;
    range_index<unsigned int>  ip_suffix_range;
    range_index<unsigned int>  port_range;
    acl_list_address_index()
    {
        ip_prefix                   = 0;
        ip_suffix_range.index_start = 0;
        ip_suffix_range.index_end   = 0;
        port_range.index_start      = 0;
        port_range.index_end        = 0;
    }
    acl_list_address_index(unsigned int ip_pre, range_index<unsigned int> ip_rang, range_index<unsigned int> port_rang)
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
    acl_list_db(unsigned int        ip_pre,
                range_index<unsigned int>        ip_suf_rang,
                range_index<unsigned int>        port_rang,
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
    if (lhs.ip_prefix < rhs.ip_prefix)
        return true;
        
    if (lhs.ip_prefix > rhs.ip_prefix)
        return false;

    cout << "ip_prefix equal, then compare ip_suffix"
         << "," << lhs.ip_prefix << "," << rhs.ip_prefix << "," << lhs.ip_suffix_range.index_start
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
    
    cout << "port equal, then compare nothing"<< endl;

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
    cout << "ADD: " << acl_db->id << ", " << acl_db->myIndex.ip_prefix << ", "
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
        cout << my_index.ip_prefix <<", (" <<my_index.ip_suffix_range.index_start<<", "
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
       cout << my_id << ", " << my_index.ip_prefix << ", (" << my_index.ip_suffix_range.index_start
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
       cout << my_id << ", " << my_index.ip_prefix << ", (" << my_index.ip_suffix_range.index_start
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
    os << acl_db_ptr->id << ", " << acl_db_ptr->myIndex.ip_prefix << ", ("
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

void test1()  
{
    //MyContainer_T mycontainer;
    cout << "Allocate a new container for ACL list......" << endl;
    acl = new ACLContainer();
    acl->insert(new acl_list_db(174015000, range_index<unsigned int>(252, 253), range_index<unsigned int>(1000, 1200), acl_list_attributes(), 1));
    acl->insert(new acl_list_db(174015000, range_index<unsigned int>(250, 255), range_index<unsigned int>(1230, 1240), acl_list_attributes(), 2));
    acl->print(); 
}  

int main()  
{
    test1();

   cout << "try to find......" << endl;
   acl->find(acl_list_address_index(174015000, range_index<unsigned int>(254, 254), range_index<unsigned int>(1239, 1239)), 1);

    system("pause");
    return 0;  
}  
