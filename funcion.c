#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <iostream>
using namespace std;


#define MAX_ARRAY_LEN 128
#define STRING_CLASS_NAME(_CLASS_NAME) #_CLASS_NAME"::"

class Mother {
    public:
    char _array[MAX_ARRAY_LEN];
    void function_1(){
        cout << STRING_CLASS_NAME(Mother)<<__func__<<endl;    }
    
    virtual void function_2(){
        cout << STRING_CLASS_NAME(Mother)<<__func__<<endl;    }
    Mother(){ 
        cout << STRING_CLASS_NAME(Mother)<<endl;
        memset(_array, 0, MAX_ARRAY_LEN);}
    ~ Mother(){}
};

class Son :public Mother{
    public:
    void function_1(){
        cout << STRING_CLASS_NAME(Son)<<__func__<<endl;    }

    // virtual void function_2(){
    //     cout <<STRING_CLASS_NAME(Son)<< __func__<<endl;    }
    Son(){ 
        cout << STRING_CLASS_NAME(Son)<<__func__<<endl;
        memset(_array, 1, MAX_ARRAY_LEN);}
    ~ Son(){}
};



struct Mother_A
{
    int a;
    int b;
    char c;
    unsigned long d;
    char e;
};

struct Mother_B
{
    int a;
    int b;
    char c;
    unsigned long d;
    char e;
};

int main()
{
        std::string s = "This is an example";
    std::cout << s << '\n';
 
    s.erase(0, 5); // Erase "This "
    std::cout << s << '\n';
 
    s.erase(std::find(s.begin(), s.end(), ' ')); // Erase ' '
    std::cout << s << '\n';
 
    s.erase(s.find(' ')); // Trim from ' ' to the end of the string
    std::cout << s << '\n';
    return 0;
    Mother *object_opr = new Son();

    object_opr->function_1();
    object_opr->function_2();
    
    void *a_ptr = reinterpret_cast<void *>(new Mother_A());

    Mother_B *b_ptr = reinterpret_cast<Mother_B *>(a_ptr);
    b_ptr->a = 100;
    b_ptr->b = 99;
    b_ptr->c = 98;
    b_ptr->d = 97;
    b_ptr->e = 96;

    Mother_A *aa_ptr = reinterpret_cast<Mother_A *>(a_ptr);

    return 0;
}