#include <iostream>
#include <stdio.h>
#include <stdlib.h>

using namespace std;

typedef struct _struct_a {
  int a;
  int b;
  int c;
  char d;
} STRUCT_A;

typedef struct _struct_b {
  int a;
  int b;
  int c;
  char d;
} STRUCT_B;

int main() {

  cout << "a test result: " << endl;
  STRUCT_A instance_a;
  instance_a.a = 100;
  instance_a.b = 200;
  instance_a.c = 499;
  instance_a.d = 19;

  cout << reinterpret_cast<STRUCT_B *>(&instance_a)->a << endl;
  cout << (*reinterpret_cast<STRUCT_B *>(&instance_a)).c << endl;
  return 0;
}