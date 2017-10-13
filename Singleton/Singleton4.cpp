#include <iostream>
#include <stdio.h>
#include <stdlib.h>

using std::cout;
using std::endl;

class Singleton {
public:
  static Singleton *get_instance() {
    static Singleton pinstance;
    return &pinstance;
  }

  void show() const { cout << "Singleton::show() called." << endl; }

  ~Singleton() { // will be called in when the process quite
    cout << "~Singleton is called" << endl;
  }

private:
  Singleton() { cout << "Singleton is called" << endl; }
};

int main(void) {
  Singleton *p = Singleton::get_instance();
  Singleton *q = Singleton::get_instance();
  Singleton *r = Singleton::get_instance();

  p->show();

  system("pause");
  return 0;
}