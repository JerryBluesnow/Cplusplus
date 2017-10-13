#include <iostream>
#include <stdio.h>
#include <stdlib.h>

using std::cout;
using std::endl;

class Singleton {
public:
  static Singleton &get_instance() {
    static Singleton pinstance;
    return pinstance;
  }

  void show() const { cout << "Singleton::show() called." << endl; }

  ~Singleton() { cout << "~Singleton is called" << endl; }

private:
  Singleton() { cout << "Singleton is called" << endl; }
  Singleton &
  operator=(const Singleton &); // is not allowed to be called like line 39~40
  Singleton(const Singleton &); // is not allowed to be called like line 39~40
};

int main(void) {
  /* this way meant to cause the desctructor function called only 1 times,
   * BUT, Singleton & operator = (const Singleton &); will be undefined;
   * Singleton(const Singleton&); will be undefined
   */
  // Singleton p = Singleton::get_instance();    // is not allowed
  // Singleton q = Singleton::get_instance();    // is not allowed
  // Singleton r = Singleton::get_instance();    // is not allowed

  Singleton::get_instance().show();

  unsigned char *ABC = 0;
  const char *cde = reinterpret_cast<const char *>(ABC);
  char *DDD = reinterpret_cast<char *>(ABC);
  system("pause");
  return 0;
}