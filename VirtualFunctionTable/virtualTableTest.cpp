/* The file is created to test virtual function table in Class Base/Derive */

#include <iostream>

using namespace std;

class Base1 {
public:
  virtual void f() { cout << "Base1::f" << endl; }

  virtual void g() { cout << "Base1::g" << endl; }

  virtual void h() { cout << "Base1::h" << endl; }
};

class Base2 {
public:
  virtual void f() { cout << "Base2::f" << endl; }

  virtual void g() { cout << "Base2::g" << endl; }

  virtual void h() { cout << "Base2::h" << endl; }
};

class Base3 {
public:
  virtual void f() { cout << "Base3::f" << endl; }

  virtual void g() { cout << "Base3::g" << endl; }

  virtual void h() { cout << "Base3::h" << endl; }
};

class Derive : public Base1, public Base2, public Base3 {
public:
  virtual void f() { cout << "Derive::f" << endl; }
  virtual void f(int i_input) {
    cout << "Derive::i_input = " << i_input << endl;
    cout << "Derive::f with input variable" << endl;
  }

  virtual void g1() { cout << "Derive::g1" << endl; }
};

typedef void (*Fun)(void);
typedef void (*Fun_ex)(int);
void function_parameter(int) { cout << "hello: parameters" << endl; }

int main() {

  Fun pFun = NULL;
  Fun_ex pFun_ex = NULL;
  Derive d;

  pFun_ex = function_parameter;
  pFun_ex(1);
  cout << pFun_ex << endl;

  int **pVtab = (int **)&d;

  // Base1's vtable

  // pFun = (Fun)*((int*)*(int*)((int*)&d+0)+0);

  pFun = (Fun)pVtab[0][0];

  pFun();

  // pFun = (Fun)*((int*)*(int*)((int*)&d+0)+1);

  pFun = (Fun)pVtab[0][1];

  pFun();

  // pFun = (Fun)*((int*)*(int*)((int*)&d+0)+2);

  pFun = (Fun)pVtab[0][2];

  pFun();

  // Derive's vtable

  // pFun = (Fun)*((int*)*(int*)((int*)&d+0)+3);

  pFun = (Fun)pVtab[0][3];

  pFun();

  pFun_ex = (Fun_ex)pVtab[0][3];

  pFun_ex(1);

  // The tail of the vtable

  pFun = (Fun)pVtab[0][4];

  pFun();

  cout << pFun << endl;

  // Base2's vtable

  // pFun = (Fun)*((int*)*(int*)((int*)&d+1)+0);

  pFun = (Fun)pVtab[1][0];

  pFun();

  // pFun = (Fun)*((int*)*(int*)((int*)&d+1)+1);

  pFun = (Fun)pVtab[1][1];

  pFun();

  pFun = (Fun)pVtab[1][2];

  pFun();

  // The tail of the vtable

  pFun = (Fun)pVtab[1][3];

  cout << pFun << endl;

  // Base3's vtable

  // pFun = (Fun)*((int*)*(int*)((int*)&d+1)+0);

  pFun = (Fun)pVtab[2][0];

  pFun();

  // pFun = (Fun)*((int*)*(int*)((int*)&d+1)+1);

  pFun = (Fun)pVtab[2][1];

  pFun();

  pFun = (Fun)pVtab[2][2];

  pFun();

  // The tail of the vtable

  pFun = (Fun)pVtab[2][3];

  cout << pFun << endl;

  return 0;
}