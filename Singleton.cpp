
/*************************************************************************
	> File Name: Singleton.h
	> Created Time: Sat 20 Jun 2015 03:04:28 PM CST
 ************************************************************************/
#include <iostream>
#include <stdlib.h>
#include <stdio.h>
using std::cout;	
using std::endl;
class Singleton
{

public:
	
	static Singleton* get_instance();
	void show() const;

private:
	
	Singleton(){}
	static Singleton* pinstance;

	class Garbo
	{
	public:
		~Garbo()
		{
			if( Singleton::pinstance )
			{
				std::cout << "~Garbo() called." << std::endl;
				delete Singleton::pinstance;
				Singleton::pinstance = NULL;
			}
		}
	};
	static Garbo garbo;
    
};

/*************************************************************************
	> File Name: Singleton.cpp
	> Created Time: Sat 20 Jun 2015 03:08:16 PM CST
 ************************************************************************/

Singleton::Garbo Singleton::garbo; 
Singleton* Singleton::pinstance = NULL;

Singleton* Singleton::get_instance()
{
	if( NULL == pinstance )
	{
		pinstance = new Singleton;
		cout << "Singleton::get_instance() called." << endl;
	}
	return pinstance;
}

void Singleton::show() const
{
	cout << "Singleton::show() called." << endl;
}
/*************************************************************************
	> File Name: main.cpp
	> Author: mz
	> Mail:1282662879@qq.com 
	> Created Time: Sat 20 Jun 2015 03:12:36 PM CST
 ************************************************************************/

int main( void )
{
	Singleton* p = Singleton::get_instance();
	Singleton* q = Singleton::get_instance();
	Singleton* r = Singleton::get_instance();

	p->show();

    system("pause");
	return 0;
}