# 可变参数及可变参数宏的使用

我们在C语言编程中会遇到一些参数个数可变的函数,例如printf()这个函数,这里将介绍可变函数的写法以及原理.

## 1. 可变参数的宏


一般在调试打印Debug 信息的时候, 需要可变参数的宏. 从C99开始可以使编译器标准支持可变参数宏(variadic macros), 另外GCC 也支持可变参数宏, 但是两种在细节上可能存在区别.

### 1. __VA_ARGS__

__VA_ARGS__ 将"..." 传递给宏.如
#define debug(format, ...) fprintf(stderr, fmt, __VA_ARGS__)

在GCC中也支持这类表示, 但是在G++ 中不支持这个表示.

2. GCC 的复杂宏

GCC使用一种不同的语法从而可以使你可以给可变参数一个名字，如同其它参数一样。
#define debug(format, args...) fprintf (stderr, format, args)

这和上面举的那个定义的宏例子是完全一样的，但是这么写可读性更强并且更容易进行描述。

3. ##__VA_ARGS__

上面两个定义的宏, 如果出现debug("A Message") 的时候, 由于宏展开后有个多余的逗号, 所以将导致编译错误. 为了解决这个问题，CPP使用一个特殊的‘##’操作。

#define debug(format, ...) fprintf (stderr, format, ## __VA_ARGS__)
这里，如果可变参数被忽略或为空，‘##’操作将使预处理器（preprocessor）去除掉它前面的那个逗号。如果你在宏调用时，确实提供了一些可变参数，GNU CPP也会工作正常，它会把这些可变参数放到逗号的后面。

4. 其他方法

一种流行的技巧是用一个单独的用括弧括起来的的 "参数" 定义和调用宏, 参数在宏扩展的时候成为类似 printf() 那样的函数的整个参数列表。
#define DEBUG(args) (printf("DEBUG: "), printf(args))


* 2. 可变参数的函数


写可变参数的C函数要在程序中用到以下这些宏:
void va_start( va_list arg_ptr, prev_param )
type va_arg( va_list arg_ptr, type )
void va_end( va_list arg_ptr )

va在这里是variable-argument(可变参数)的意思,这些宏定义在stdarg.h中.下面我们写一个简单的可变参数的函数,该函数至少有一个整数参数,第二个参数也是整数,是可选的.函数只是打印这两个参数的值.
[cpp] view plain copy

    void simple_va_fun(int i, ...)  
    {  
       va_list arg_ptr;  
       int j=0;  
        
       va_start(arg_ptr, i);  
       j=va_arg(arg_ptr, int);  
       va_end(arg_ptr);  
       printf("%d %d\n", i, j);  
       return;  
    }  


在程序中可以这样调用:
[cpp] view plain copy

    simple_va_fun(100);  
    simple_va_fun(100,200);  


从这个函数的实现可以看到,使用可变参数应该有以下步骤:
1)首先在函数里定义一个va_list型的变量,这里是arg_ptr,这个变量是指向参数的指针.
2)然后用va_start宏初始化变量arg_ptr,这个宏的第二个参数是第一个可变参数的前一个参数,是一个固定的参数.
3)然后用va_arg返回可变的参数,并赋值给整数j. va_arg的第二个参数是你要返回的参数的类型,这里是int型.
4)最后用va_end宏结束可变参数的获取.然后你就可以在函数里使用第二个参数了.如果函数有多个可变参数的,依次调用va_arg获取各个参数.

如果我们用下面三种方法调用的话,都是合法的,但结果却不一样:
1)simple_va_fun(100);
结果是:100 -123456789(会变的值)
2)simple_va_fun(100,200);
结果是:100 200
3)simple_va_fun(100,200,300);
结果是:100 200

我们看到第一种调用有错误,第二种调用正确,第三种调用尽管结果正确,但和我们函数最初的设计有冲突.下面一节我们探讨出现这些结果的原因和可变参数在编译器中是如何处理的.

* 3. 可变参数函数原理


va_start,va_arg,va_end是在stdarg.h中被定义成宏的,由于硬件平台的不同,编译器的不同,所以定义的宏也有所不同,下面以VC++中stdarg.h里x86平台的宏定义摘录如下:
[cpp] view plain copy

    typedef char * va_list;  
    #define _INTSIZEOF(n) ((sizeof(n)+sizeof(int)-1)&~(sizeof(int) - 1) )  
    #define va_start(ap,v) ( ap = (va_list)&v + _INTSIZEOF(v) )  
    #define va_arg(ap,t) ( *(t *)((ap += _INTSIZEOF(t)) - _INTSIZEOF(t)) )  
    #define va_end(ap) ( ap = (va_list)0 )   

定义_INTSIZEOF(n)主要是为了内存对齐,C语言的函数是从右向左压入堆栈的(设数据进栈方向为从高地址向低地址发展,即首先压入的数据在高地址). 下图是函数的参数在堆栈中的分布位置:

低地址   |-----------------------------|<-- &v
       |第n-1个参数(最后一个固定参数)|
       |-----------------------------|<--va_start后ap指向
       |第n个参数(第一个可变参数) |
       |-----------------------------|
       |....... |
       |-----------------------------|
       |函数返回地址 |
高地址 |-----------------------------|

1. va_list 被定义为char *
2. va_start 将地址ap定义为 &v+_INTSIZEOF(v),而&v是固定参数在堆栈的地址,所以va_start(ap, v)以后,ap指向第一个可变参数在堆栈的地址
3. va_arg 取得类型t的可变参数值,以int型为例,va_arg取int型的返回值:
   j= ( *(int*)((ap += _INTSIZEOF(int))-_INTSIZEOF(int)) );
4. va_end 使ap不再指向堆栈,而是跟NULL一样.这样编译器不会为va_end产生代码.

在不同的操作系统和硬件平台的定义有些不同,但原理却是相似的.


* 4. 小结


对于可变参数的函数,因为va_start, va_arg, va_end等定义成宏,所以它显得很愚蠢,可变参数的类型和个数需要在该函数中由程序代码控制;另外,编译器对可变参数的函数的原型检查不够严格,对编程查错不利.
所以我们写一个可变函数的C函数时,有利也有弊,所以在不必要的场合,无需用到可变参数.如果在C++里,我们应该利用C++的多态性来实现可变参数的功能,尽量避免用c语言的方式来实现.


* 5. 附一些代码


[cpp] view plain copy

    #define debug(format, ...) fprintf(stderr, fmt, __VA_ARGS__)  
    #define debug(format, args...) fprintf (stderr, format, args)  
    #define debug(format, ...) fprintf (stderr, format, ## __VA_ARGS__)  


// 使用va... 实现
[cpp] view plain copy

    void debug(const char *fmt, ...)  
    {  
        int nBuf;  
        char szBuffer[1024];  
        va_list args;  
      
        va_start(args, fmt);  
        nBuf = vsprintf(szBuffer, fmt, args) ;  
        assert(nBuf >= 0);  
      
        printf("QDOGC ERROR:%s\n",szBuffer);  
        va_end(args);  
    }   