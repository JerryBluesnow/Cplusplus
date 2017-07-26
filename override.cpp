#include <iostream>
#include <stdio.h>
#include <stdlib.h>

using namespace std;

class BaseClass {
public:
  void func_1() { cout << __func__ << " Base Class" << endl; }
  virtual void func_2() { cout << __func__ << " Base Class" << endl; }
};
class DeriveClass : public BaseClass {
public:
  void func_1() { cout << __func__ << " DeriveClass" << endl; }
  void func_2() { cout << __func__ << " DeriveClass" << endl; }
  void func_2(int abc) { cout << __func__ << " DeriveClass" << endl; }
};

int main() {
  BaseClass *base_ptr = new DeriveClass;
  DeriveClass *derive_class = dynamic_cast<DeriveClass *>(base_ptr);

  if (base_ptr != NULL) {
    base_ptr->func_1(); // redefining will call func_1 in Base Class
    base_ptr->func_2(); // override will call func_1 in Base Class
                        // base_ptr->func_2(2); // ERROR
    derive_class->func_2();
    derive_class->func_2(2); // overload will call func_2(int abc)
  }

  delete base_ptr;
  return 0;
}