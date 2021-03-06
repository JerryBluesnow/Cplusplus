#include <iostream>
#include <stdlib.h>
#include <stdio.h>
using std::cout;	
using std::endl;

/* this way will cause the desctructor function called 4 times */
class Singleton
{
public:
    static Singleton &get_instance() {
   	 	static Singleton pinstance;
		return pinstance;
	}

	void show() const{
		cout << "Singleton::show() called." << endl;
	}
	
    ~Singleton() {				
        cout<<"~Singleton is called"<<endl;
    }
private:
	
	Singleton()	{
		cout<<"Singleton is called"<<endl;}
};

int main( void )
{
	Singleton p = Singleton::get_instance();
	Singleton q = Singleton::get_instance();
	Singleton r = Singleton::get_instance();

	p.show();

    system("pause");
	return 0;
}