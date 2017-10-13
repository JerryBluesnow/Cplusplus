#include <stdio.h>
#include <stdlib.h>
#include <iostream>
using namespace std;

#define PRIVATE_ARGS_GLUE(x, y) x y
#define PRIVATE_MACRO_VAR_ARGS_IMPL_COUNT(_1, _2, _3, _4, _5, _6, _7, _8, _9, N, ...) N
#define PRIVATE_MACRO_VAR_ARGS_IMPL(args) PRIVATE_MACRO_VAR_ARGS_IMPL_COUNT args
#define COUNT_MACRO_VAR_ARGS(...) PRIVATE_MACRO_VAR_ARGS_IMPL((__VA_ARGS__, 10, 9, 8, 7, 6, 5 4, 3, 2, 1, 0))

#define PRIVATE_MACRO_CHOOSE_HELPER2(M, count) M##count
#define PRIVATE_MACRO_CHOOSE_HELPER1(M, count) PRIVATE_MACRO_CHOOSE_HELPER2(M, count)
#define PRIVATE_MACRO_CHOOSE_HELPER(M, count) PRIVATE_MACRO_CHOOSE_HELPER1(M, count)

#define INVOKE_VAR_MACRO(M, ...) PRIVATE_ARGS_GLUE(PRIVATE_MACRO_CHOOSE_HELPER(M, COUNT_MACRO_VAR_ARGS(__VA_ARGS__)), (__VA_ARGS__))

#define PRINT_ARGS1(a1) std::cout << a1 << std::endl
#define PRINT_ARGS2(a1, a2) std::cout << a1 << "," << a2 << std::endl
#define PRINT_ARGS3(a1, a2, a3) std::cout << a1 << "," << a2 << "," << a3 << std::endl
#define PRINT_ARGS4(a1, a2, a3, a4) std::cout << a1 << "," << a2 << "," << a3 << "," << a4 << std::endl
#define PRINT_ARGS5(a1, a2, a3, a4, a5) std::cout << a1 << "," << a2 << "," << a3 << "," << a4 << "," << a5 << std::endl

#define PP_ARG_N(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, N, ...) ((#_1 == "") ? 0 : N)
#define PP_NARG_(args) PP_ARG_N args
#define PP_NARG(...) PP_NARG_((__VA_ARGS__, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0))

#define TESTINVOKE_VAR_MACRO(M, ...) std::cout << PP_NARG(__VA_ARGS__) << std::endl

int main()
{
    INVOKE_VAR_MACRO(PRINT_ARGS, 4);
    INVOKE_VAR_MACRO(PRINT_ARGS, 4, 5);
    INVOKE_VAR_MACRO(PRINT_ARGS, 4, 5, 6);
    //INVOKE_VAR_MACRO(PRINT_ARGS, 4, 5, 6, 7);
    //INVOKE_VAR_MACRO(PRINT_ARGS, 4, 5, 6, 7, 8);
    TESTINVOKE_VAR_MACRO(PRINT_ARGS);
    TESTINVOKE_VAR_MACRO(PRINT_ARGS, 4);
    TESTINVOKE_VAR_MACRO(PRINT_ARGS, 4, 5);
    TESTINVOKE_VAR_MACRO(PRINT_ARGS, 4, 5, 6);
    TESTINVOKE_VAR_MACRO(PRINT_ARGS, 4, 5, 6, 7);
    TESTINVOKE_VAR_MACRO(PRINT_ARGS, 4, 5, 6, 7, 8);

    return 0;
}