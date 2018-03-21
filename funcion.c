#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <iostream>
using namespace std;


#define MAX_ARRAY_LEN 128
#define STRING_CLASS_NAME(_CLASS_NAME) #_CLASS_NAME

class Mother {
    public:
    char _array[MAX_ARRAY_LEN];
    void function_1(){
        cout << STRING_CLASS_NAME(Mother)<<"::"<<__func__<<" is called"<<endl;    }
    
    virtual void function_2(){
        cout << __func__<<" is called"<<endl;    }
    Mother(){ memset(_array, 0, MAX_ARRAY_LEN);}
    ~ Mother(){}
};

class Son :public Mother{
    public:
    void function_1(){
        cout << STRING_CLASS_NAME(Son)<<"::"<<__func__<<" is called"<<endl;    }

    virtual void function_2(){
        cout << __func__<<" is called"<<endl;    }
    Son(){ memset(_array, 1, MAX_ARRAY_LEN);}
    ~ Son(){}
};


int main()
{
    Mother *object_opr = new Son();

    object_opr->function_1();

    return 0;
}