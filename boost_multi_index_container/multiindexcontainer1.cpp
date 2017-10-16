/**
 * boost multi index container test
 * platform: win32, visual studio 2005/2010; Linux, gcc4.1.2
 */
#include <iostream>
#include "boost/multi_index_container.hpp"
#include "boost/multi_index/member.hpp"
#include "boost/multi_index/ordered_index.hpp"

using namespace std;
using namespace boost::multi_index;
using boost::multi_index_container;

// define multiple index
typedef struct MyIndex
{
    int x;
    int y;
    int z;
    MyIndex(int ax = 0, int ay = 0, int az = 0) : x(ax), y(ay), z(az) {}
    void
    print(char *prompt) const
    {
        cout << "(" << x << ", " << y << ", " << z << ") - " << prompt << endl;
    }
} MyIndex;

// define data to be indexed
typedef struct
{
    int a;
    int b;
} MyData;

// define object to be indexed
class MyTest
{
   public:
    MyIndex myIndex;
    MyData  myData;

   public:
    MyTest(int x, int y, int z, int a, int b)
    {
        myIndex.x = x;
        myIndex.y = y;
        myIndex.z = z;
        myData.a  = a;
        myData.b  = b;
    }

    ~MyTest() { print(", destructed"); }
    
    void
    print( const char *prompt = "") const
    {
        cout << "(" << myIndex.x << ", " << myIndex.y << ", " << myIndex.z << ") - ";
        cout << "(" << myData.a << ", " << myData.b << ")" << prompt << endl;
    }

   private:
    MyTest(const MyTest &);
    MyTest &
    operator=(const MyTest &);
};

// define index tag, multi_index_container, and its type
struct MyIndexTag
{
};

typedef multi_index_container<
    MyTest *,
    indexed_by<ordered_unique<tag<MyIndexTag>, member<MyTest, MyIndex, &MyTest::myIndex> > > >
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
operator<(const MyIndex &lhs, const MyIndex &rhs)
{
    if (lhs.x < rhs.x)
        return true;
    else if (lhs.x > rhs.x)
        return false;
    else if (lhs.y < rhs.y)
        return true;
    else if (lhs.y > rhs.y)
        return false;
    else if (lhs.z < rhs.z)
        return true;
    else if (lhs.z > rhs.z)
        return false;
    else
        return false;
}

std::ostream &
operator<<(std::ostream &os, const MyTest *mytest)
{
    mytest->print();
    return os;
}

// instantiate a instance for this template class
MyContainer<MyContainer_T, MyIndexTag, MyTest, MyIndex> mycontainer;
void
test1()
{
    MyTest *a = new MyTest(1, 1, 1, 10, 100);
    mycontainer.insert(a);
    MyTest *b = new MyTest(1, 1, 2, 20, 200);
    mycontainer.insert(b);
    MyTest *c = new MyTest(1, 1, 3, 30, 300);
    mycontainer.insert(c);
}

void
test2()
{
    MyTest *a = new MyTest(1, 2, 1, 40, 400);
    mycontainer.insert(a);
    MyTest *b = new MyTest(1, 2, 2, 50, 500);
    mycontainer.insert(b);
    MyTest *c = new MyTest(1, 2, 3, 60, 600);
    mycontainer.insert(c);
}

void
test3()
{
    MyTest *a = new MyTest(1, 3, 1, 70, 700);
    mycontainer.insert(a);
    MyTest *b = new MyTest(1, 3, 2, 80, 800);
    mycontainer.insert(b);
    MyTest *c = new MyTest(1, 3, 3, 90, 900);
    mycontainer.insert(c);
}

void
test4()
{
    MyTest *a = new MyTest(2, 1, 1, 110, 1000);
    mycontainer.insert(a);
    MyTest *b = new MyTest(2, 1, 2, 220, 2000);
    mycontainer.insert(b);
    MyTest *c = new MyTest(2, 1, 3, 330, 3000);
    mycontainer.insert(c);
}

void
test5()
{
    MyTest *a = new MyTest(2, 2, 1, 440, 4000);
    mycontainer.insert(a);
    MyTest *b = new MyTest(2, 2, 2, 550, 5000);
    mycontainer.insert(b);
    MyTest *c = new MyTest(2, 2, 3, 660, 6000);
    mycontainer.insert(c);
}

void
test6()
{
    MyTest *a = new MyTest(2, 3, 1, 770, 7000);
    mycontainer.insert(a);
    MyTest *b = new MyTest(2, 3, 2, 880, 8000);
    mycontainer.insert(b);
    MyTest *c = new MyTest(2, 3, 3, 990, 9000);
    mycontainer.insert(c);
}

void
test_find()
{
    mycontainer.find(MyIndex(1, 1, 1));
    mycontainer.find(MyIndex(1, 1, 2));
    mycontainer.find(MyIndex(1, 1, 3));
    mycontainer.find(MyIndex(1, 2, 1));
    mycontainer.find(MyIndex(1, 2, 2));
    mycontainer.find(MyIndex(1, 2, 3));
    mycontainer.find(MyIndex(1, 3, 1));
    mycontainer.find(MyIndex(1, 3, 2));
    mycontainer.find(MyIndex(1, 3, 3));
    mycontainer.find(MyIndex(2, 1, 1));
    mycontainer.find(MyIndex(2, 1, 2));
    mycontainer.find(MyIndex(2, 1, 3));
    mycontainer.find(MyIndex(2, 2, 1));
    mycontainer.find(MyIndex(2, 2, 2));
    mycontainer.find(MyIndex(2, 2, 3));
    mycontainer.find(MyIndex(2, 3, 1));
    mycontainer.find(MyIndex(2, 3, 2));
    mycontainer.find(MyIndex(2, 3, 3));
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
    mycontainer.free();
    system("pause");
    return 0;
}