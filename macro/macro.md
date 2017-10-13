# C语言宏中"#"和"##"的用法
 

在查看linux内核源码的过程中，遇到了许多宏，这里面有许多都涉及到"#"和"##"，因此，在网上搜索了一些资料，整理如下：

## 一、一般用法 
我们使用#把宏参数变为一个字符串,用##把两个宏参数贴合在一起. 
用法: 

    ＃include<cstdio> 
    ＃include<climits> 
    using namespace std; 

    #define STR(s)     #s 
    #define CONS(a,b)  int(a##e##b) 

    int main() 
    { 
        printf(STR(vck));           // 输出字符串"vck" 
        printf("%d\n", CONS(2,3));  // 2e3 输出:2000 
        return 0; 
    } 

## 二、当宏参数是另一个宏的时候 
需要注意的是凡宏定义里有用'#'或'##'的地方宏参数是不会再展开. 

### 1, 非'#'和'##'的情况 

    #define TOW      (2) 
    #define MUL(a,b) (a*b) 

    printf("%d*%d=%d\n", TOW, TOW, MUL(TOW,TOW)); 
这行的宏会被展开为： 

    printf("%d*%d=%d\n", (2), (2), ((2)*(2))); 
MUL里的参数TOW会被展开为(2). 

### 2, 当有'#'或'##'的时候 

    #define A          (2) 
    #define STR(s)     #s 
    #define CONS(a,b)  int(a##e##b) 

    printf("int max: %s\n",  STR(INT_MAX));    // INT_MAX ＃include<climits> 
这行会被展开为： 
    printf("int max: %s\n", "INT_MAX"); 

    printf("%s\n", CONS(A, A));               // compile error  
这一行则是： 
    printf("%s\n", int(AeA)); 

INT_MAX和A都不会再被展开, 然而解决这个问题的方法很简单. 加多一层中间转换宏. 
加这层宏的用意是把所有宏的参数在这层里全部展开, 那么在转换宏里的那一个宏(_STR)就能得到正确的宏参数. 

    #define A           (2) 
    #define _STR(s)     #s 
    #define STR(s)      _STR(s)          // 转换宏 
    #define _CONS(a,b)  int(a##e##b) 
    #define CONS(a,b)   _CONS(a,b)       // 转换宏 

    printf("int max: %s\n", STR(INT_MAX));  // INT_MAX,int型的最大值，为一个变量    ＃include<climits>
     
输出为: int max: 0x7fffffff 

    STR(INT_MAX) -->  _STR(0x7fffffff) 然后再转换成字符串； 

    printf("%d\n", CONS(A, A)); 
输出为：200 

    CONS(A, A)  -->  _CONS((2), (2))  --> int((2)e(2)) 

## 三、'#'和'##'的一些应用特例 
### 1、合并匿名变量名 

    #define  ___ANONYMOUS1(type, var, line)  type  var##line 
    #define  __ANONYMOUS0(type, line)  ___ANONYMOUS1(type, _anonymous, line) 
    #define  ANONYMOUS(type)  __ANONYMOUS0(type, __LINE__) 

例：ANONYMOUS(static int);  即: static int _anonymous70;  70表示该行行号； 
第一层：ANONYMOUS(static int);  -->  __ANONYMOUS0(static int, __LINE__); 
第二层：                        -->  ___ANONYMOUS1(static int, _anonymous, 70); 
第三层：                        -->  static int  _anonymous70; 
即每次只能解开当前层的宏，所以__LINE__在第二层才能被解开； 

### 2、填充结构 

    #define  FILL(a)   {a, #a} 

    enum IDD{OPEN, CLOSE}; 
    typedef struct MSG{ 
        IDD id; 
        const char * msg; 
    }MSG; 

    MSG _msg[] = {FILL(OPEN), FILL(CLOSE)}; 
相当于： 

    MSG _msg[] = {{OPEN, "OPEN"}, 
               {CLOSE, "CLOSE"}}; 

### 3、记录文件名 

    #define  _GET_FILE_NAME(f)   #f 
    #define  GET_FILE_NAME(f)    _GET_FILE_NAME(f) 
    static char  FILE_NAME[] = GET_FILE_NAME(__FILE__); 

### 4、得到一个数值类型所对应的字符串缓冲大小 

    #define  _TYPE_BUF_SIZE(type)  sizeof #type 
    #define  TYPE_BUF_SIZE(type)   _TYPE_BUF_SIZE(type) 
    char  buf[TYPE_BUF_SIZE(INT_MAX)]; 
     -->  char  buf[_TYPE_BUF_SIZE(0x7fffffff)]; 
     -->  char  buf[sizeof "0x7fffffff"]; 
这里相当于： 
    char  buf[11];

#### 【alps_008】:
基本看了一遍，楼主的情况属于一般用法：

“#把宏参数变为一个字符串,用##把两个宏参数贴合在一起”

    #include<stdio.h>
    #include<string.h>
    #define STRCPY(a,b) strcpy(a##_p,#b)   //把第一个参数后边加上字符_p，把第二个参数变成字符串
    
    int main()
    {
        char var1_p[20];
        char var2_p[30];
        strcpy(var1_p,"aaaa");
        strcpy(var2_p,"bbbb");
        STRCPY(var1,var2);            //等于strcpy(var1_p,"var2");
        STRCPY(var2,var1);            //等于strcpy(var2_p,"var1");
        printf("%s\n",var1_p);
        printf("%s\n",var2_p);
        return 0;
    }
    
    
    
#### 【jeffer007】:
    Token-Pasting Operator (##) 

    // preprocessor_token_pasting.cpp
    #include <stdio.h>
    #define paster( n ) printf_s( "token" #n " = %d", token##n )
    int token9 = 9;
    
    int main()
    {
       paster(9);
    }
     
    Output token9 = 9
    
    Stringizing Operator (#) 
	
	
    // stringizer.cpp
    #include <stdio.h>
    #define stringer( x ) printf( #x "\n" )
    int main() {
       stringer( In quotes in the printf function call ); 
       stringer( "In quotes when printed to the screen" );   
       stringer( "This: \"  prints an escaped double quote" );
    }
    
    Output
    In quotes in the printf function call
    "In quotes when printed to the screen"
    "This: \"  prints an escaped double quote" 
    

# 可变参数宏的展开，宏重载

因某些方面需要，涉及到可变参数宏的展开及可变参数数量的确定，在网上查找了大部分资料，基本如下所述

http://www.cnblogs.com/goooon/p/5642514.html

涉及到的问题点：不支持0个参数。

经过VS2010及gcc version 4.9.1 (GCC)两个编译器编译，可以至少支持此两种编译器下的0参数问题。

以上文中的代码为例：

定义部分

    #define PRIVATE_ARGS_GLUE(x, y) x y

    #define PRIVATE_MACRO_VAR_ARGS_IMPL_COUNT(_1,_2,_3,_4,_5,_6,_7,_8,_9, _10, N, ...) N
    #define PRIVATE_MACRO_VAR_ARGS_IMPL(args) PRIVATE_MACRO_VAR_ARGS_IMPL_COUNT args
    #define COUNT_MACRO_VAR_ARGS(...) PRIVATE_MACRO_VAR_ARGS_IMPL((__VA_ARGS__,10, 9,8,7,6,5,4,3,2,1,0))

    #define PRIVATE_MACRO_CHOOSE_HELPER2(M,count)  M##count
    #define PRIVATE_MACRO_CHOOSE_HELPER1(M,count) PRIVATE_MACRO_CHOOSE_HELPER2(M,count)
    #define PRIVATE_MACRO_CHOOSE_HELPER(M,count)   PRIVATE_MACRO_CHOOSE_HELPER1(M,count)

    #define INVOKE_VAR_MACRO(M,...) PRIVATE_ARGS_GLUE(PRIVATE_MACRO_CHOOSE_HELPER(M,COUNT_MACRO_VAR_ARGS(__VA_ARGS__)), (__VA_ARGS__))

实现部分

    #define PRINT_ARGS1(a1) std::cout<<a1<<std::endl
    #define PRINT_ARGS2(a1,a2) std::cout<<a1<<","<<a2<<std::endl
    #define PRINT_ARGS3(a1,a2,a3) std::cout<<a1<<","<<a2<<","<<a3<<std::endl
    #define PRINT_ARGS4(a1,a2,a3,a4) std::cout<<a1<<","<<a2<<","<<a3<<","<<a4<<std::endl

使用部分

    INVOKE_VAR_MACRO(PRINT_ARGS, 4);
    INVOKE_VAR_MACRO(PRINT_ARGS, 4, 5);
    INVOKE_VAR_MACRO(PRINT_ARGS, 4, 5, 6);

 

如何实现?

    #define PRINT_ARGS0() std::cout<<"ARGS0"<<std::endl 

如何使用?

    INVOKE_VAR_MACRO(PRINT_ARGS);

参考  http://en.cppreference.com/w/cpp/preprocessor/replace

    some compilers offer an extension that allows ## to appear after a comma and before __VA_ARGS__,

    in which case the ## does nothing when __VA_ARGS__ is non-empty,

    but removes the comma when __VA_ARGS__ is empty: this makes it possible to define macros such as fprintf (stderr, format, ##__VA_ARGS__)

看不懂？那看看http://blog.csdn.net/sakaue/article/details/29591631

 

那是不是可以改成这样呢？

    #define PRIVATE_MACRO_VAR_ARGS_IMPL_COUNT(_0,_1,_2,_3,_4,_5,_6,_7,_8,_9, _10, N, ...) N
    #define PRIVATE_MACRO_VAR_ARGS_IMPL(args) PRIVATE_MACRO_VAR_ARGS_IMPL_COUNT args
    #define COUNT_MACRO_VAR_ARGS(...) PRIVATE_MACRO_VAR_ARGS_IMPL((0,##__VA_ARGS__,10, 9,8,7,6,5,4,3,2,1,0))

在gcc version 4.9.1 (GCC)下是没有问题的，

可以输出

    ARGS0
    4
    4,5
    4,5,6

但是在vs2010中就出现了编译错误

    warning C4003: “PRINT_ARGS1”宏的实参不足
    error C2059: 语法错误:“<<”

那是不是意味着vs2010不支持 ”如果可变参数被忽略或为空，‘##’操作将使预处理器（preprocessor）去除掉它前面的那个逗号。”

在vs2010中定义

    #define debug(format, ...) fprintf (stderr, format, ##__VA_ARGS__)

vs2010中实现

    debug("abc");

结果编译没有问题，输出也正常。

 

因此，应该是vs2010和gcc在预编译中，对于宏的处理存在差异。最明显的一个例子就是参考google chromium中logging，

    typedef int LogSeverity;
    const LogSeverity LOG_VERBOSE = -1;  // This is level 1 verbosity
    // Note: the log severities are used to index into the array of names,
    // see log_severity_names.
    const LogSeverity LOG_INFO = 0;
    const LogSeverity LOG_WARNING = 1;
    const LogSeverity LOG_ERROR = 2;
    const LogSeverity LOG_FATAL = 3;

     

    LOG(INFO) << "Found " << num_cookies << " cookies";

    其中涉及使用 ::logging::LOG_ ## severity  组成一个 ::logging::LOG_INFO

个人修改时，添加了一个LOG_DEBUG，结果vs2010上编译使用都没有问题，gcc version 4.9.1 (GCC)下编译不过，说找不到LOG_1，修改为LOG_DEBUGGING，编译及输出均OK。

 

那么如何在此两种编译环境下解决不支持0个参数问题呢？

 

测试代码

    #define PP_ARG_N(_1,_2,_3,_4,_5,_6,_7,_8,_9, _10, N, ...) ((#_1 == "") ? 0 : N)
    #define PP_NARG_(args)  PP_ARG_N args
    #define PP_NARG(...) PP_NARG_((__VA_ARGS__,10,9,8,7,6,5,4,3,2,1,0))

    #define TESTINVOKE_VAR_MACRO(M,...) std::cout<<PP_NARG(__VA_ARGS__)<<std::endl

    TESTINVOKE_VAR_MACRO(PRINT_ARGS);
    TESTINVOKE_VAR_MACRO(PRINT_ARGS, 4);
    TESTINVOKE_VAR_MACRO(PRINT_ARGS, 4, 5);
    TESTINVOKE_VAR_MACRO(PRINT_ARGS, 4, 5, 6);

输出

    0
    1
    2
    3

 

但要针对#define PRIVATE_MACRO_VAR_ARGS_IMPL_COUNT(_1,_2,_3,_4,_5,_6,_7,_8,_9, _10, N, ...) N 的修改，那就要需要慢慢修改，为什么呢？

参考宏替换规则，如果将N改为一个表达式，那还能找到对应的 PRINT_ARGSX 吗？显然是不能的。但如果作为函数传参，这个方法还是能够胜任的，比如上述的测试代码。