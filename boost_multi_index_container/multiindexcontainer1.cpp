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

    ~MyTest(){};

    void
    print(const char *prompt) const
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
// define operator< of index for this container
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
}

// define a global container
MyContainer_T mycontainer;

// print the container
void
print_container1()
{
    MyContainerIndex_T &                   indexSet = mycontainer.get<MyIndexTag>();
    typedef MyContainerIndex_T::value_type value_type;
    std::copy(indexSet.begin(), indexSet.end(), std::ostream_iterator<value_type>(cout));
}

// print the container
void
print_container2()
{
    MyContainerIndex_T &  indexSet = mycontainer.get<MyIndexTag>();
    MyContainerIterator_T iter     = indexSet.begin();
    while (iter != indexSet.end())
    {
        (*iter)->print("");
        ++iter;
    }
}

std::ostream &
operator<<(std::ostream &os, const MyTest *mytest)
{
    // mytest->print("");  //this clause can work
    os << "(" << mytest->myIndex.x << ", " << mytest->myIndex.y << ", " << mytest->myIndex.z
       << ") - ";
    os << "(" << mytest->myData.a << ", " << mytest->myData.b << ")" << endl;
    return os;
}

// find an element in the container
void
find(int x, int y, int z)
{
    MyContainerIndex_T &indexSet = mycontainer.get<MyIndexTag>();
    // or
    // const boost::multi_index::index<MyContainer_T, MyIndexTag>::type& indexSet =
    // get<MyIndexTag>(mycontainer);
    MyContainerIterator_T iter = indexSet.find(MyIndex(x, y, z));
    if (indexSet.end() == iter)
    {
        cout << "(" << x << ", " << y << ", " << z << ") - not found" << endl;
        return;
    }
    (*iter)->print(", found");
}

// find an element in the container, the efficiency is very low
void
find2(int x, int y, int z)
{
    MyContainerIndex_T &  indexSet = mycontainer.get<MyIndexTag>();
    MyContainerIterator_T iter     = indexSet.begin();
    while (iter != indexSet.end())
    {
        MyIndex index = (*iter)->myIndex;
        if (index.x == x && index.y == y && index.z == z)
        {
            (*iter)->print(", found");
            return;
        }
        ++iter;
    }
    cout << "(" << x << ", " << y << ", " << z << ") - not found" << endl;
}

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
    find(1, 1, 1);
    find(1, 1, 2);
    find(1, 1, 3);
    find(1, 2, 1);
    find(1, 2, 2);
    find(1, 2, 3);
    find(1, 3, 1);
    find(1, 3, 2);
    find(1, 3, 3);
    find(2, 1, 1);
    find(2, 1, 2);
    find(2, 1, 3);
    find(2, 2, 1);
    find(2, 2, 2);
    find(2, 2, 3);
    find(2, 3, 1);
    find(2, 3, 2);
    find(2, 3, 3);
}

void
test_find2()
{
    find2(1, 1, 1);
    find2(1, 1, 2);
    find2(1, 1, 3);
    find2(1, 2, 1);
    find2(1, 2, 2);
    find2(1, 2, 3);
    find2(1, 3, 1);
    find2(1, 3, 2);
    find2(1, 3, 3);
    find2(2, 1, 1);
    find2(2, 1, 2);
    find2(2, 1, 3);
    find2(2, 2, 1);
    find2(2, 2, 2);
    find2(2, 2, 3);
    find2(2, 3, 1);
    find2(2, 3, 2);
    find2(2, 3, 3);
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
    print_container1();
    cout << endl;
    print_container2();
    cout << endl;
    test_find();
    cout << endl;
    test_find2();
    return 0;
}