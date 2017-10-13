/***test the offset of a member of a struct*/
#include <iostream>
#include <stdlib.h>
#include <stdio.h>
    
typedef struct
{
    char x;
    char y;
    long long z;
} Point;
//#define offsetof(TYPE, MEMBER) ((size_t) &((TYPE*)0)- >MEMBER)
#define offsetof(TYPE, MEMBER) ((size_t) & (((TYPE*)0) -> MEMBER))
void
test1()
{
    int x = (size_t) &((Point*)0) -> x;
    int y = (size_t) &((Point*)0) -> y;
    int z = (size_t) &(((Point*)0) -> z);
    printf("offset: x = %d, y = %d, z = %d\n", x, y, z);
}
void
test2()
{
    printf("Point.x offset = %d\n", offsetof(Point, x));
    printf("Point.y offset = %d\n", offsetof(Point, y));
    printf("Point.z offset = %d\n", offsetof(Point, z));
}
int
main(int argc, char** argv)
{
    test1();
    printf("\n");
    test2();
    system("pause");
    return 0;
}
