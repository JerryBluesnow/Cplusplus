#include <iostream>
#include <stdlib.h>
#include <stdio.h>
using std::cout;	
using std::endl;

class Singleton
{
public:
    static Singleton* get_instance() {
   	 	static Singleton* pinstance = NULL;
		if( NULL == pinstance )
		{
			pinstance = new Singleton;
			cout << "Singleton::get_instance() called." << endl;
		}
		return pinstance;
	}

	void show() const{
		cout << "Singleton::show() called." << endl;
	}
	
    ~Singleton() {				// will not be called in when the process quite
        cout<<"~Singleton is called"<<endl;
    }
private:
	
	Singleton()	{
		cout<<"Singleton is called"<<endl;}
};

int main( void )
{
	Singleton* p = Singleton::get_instance();
	Singleton* q = Singleton::get_instance();
	Singleton* r = Singleton::get_instance();

	p->show();

    system("pause");
	return 0;
}