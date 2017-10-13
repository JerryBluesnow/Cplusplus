#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include <iostream>

using namespace std;
void
ShowBeforeAndAfterMalloc(unsigned char *pBuffer, int nBufferLength)
{
    int i      = 0;
    int nRange = 20;
    if (NULL != pBuffer)
    {
        for (i = -nRange; i < nBufferLength + nRange; i++)
        {
            if (0 == i || nBufferLength == i)
            {
                printf("\n\n");
            }

            printf("% 02x", *(pBuffer + i));
            if (i % 2 != 0)
            {
                printf(" ");
            }
        }
        printf("\n\n");
    }
}

typedef struct _TknStrVar
{
    int   pres;
    int   len;
    char *val;
} TknStrVar;

void
reset_TknStrVar(TknStrVar &_tkn_str_var)
{
    _tkn_str_var.pres = 0;
    _tkn_str_var.len  = 0;
    _tkn_str_var.val  = NULL;
}

void
Release_TknStrVar(TknStrVar &_tkn_str_var)
{
    //IMS_DLOG(IMS_IBCF_LOGLOW, "%s: try to free _tkn_str_var", __func__);
    if (_tkn_str_var.pres != 0 || _tkn_str_var.val != NULL)
    {
       // IMS_DLOG(IMS_IBCF_LOGLOW, "%s: do to free _tkn_str_var", __func__);
        free(_tkn_str_var.val);
        reset_TknStrVar(_tkn_str_var);
    }

    return;
}

void test_release_TknStrVar()
{
    TknStrVar _val;
    reset_TknStrVar(_val);
    _val.pres = 1;
    _val.len  = strlen("hello");

    int n = 5;  
    _val.val = (char *)malloc(n+1);//最后一个+1就是'\0'  
    strcpy(_val.val,"hello");//string copy 注意这里hello是5个字节

    Release_TknStrVar(_val);
}

int
main(void)
{
#if 0
    const int      nNeedSize = 20;
    unsigned char *p         = (unsigned char *)malloc(nNeedSize);
    unsigned char *q         = NULL;
    memset(p, 0xfd, nNeedSize);
    q = (unsigned char *)malloc(nNeedSize << 1);
    memset(q, 0xfd, nNeedSize << 1);
    ShowBeforeAndAfterMalloc(p, nNeedSize);
    ShowBeforeAndAfterMalloc(q, nNeedSize << 1);
    free(p);
    printf("=================== After free(p)==================\n");
    ShowBeforeAndAfterMalloc(p, nNeedSize);
    free(q);
    printf("=================== After free(q)==================\n");
    ShowBeforeAndAfterMalloc(q, nNeedSize << 1);
    printf("Hello World!\n");
#endif
    // char *p = (char *)malloc(10);
    // ShowBeforeAndAfterMalloc(reinterpret_cast<unsigned char*>(p), 10);
    // memset(p, 0, 10);
    // ShowBeforeAndAfterMalloc(reinterpret_cast<unsigned char*>(p), 10);
    // strcpy(p, "Hello d!");
    // printf("%s\n", p);
    // ShowBeforeAndAfterMalloc(reinterpret_cast<unsigned char*>(p), 10);
    // free(p);
   // test_release_TknStrVar();

    std::string std_string_test("A");
    cout << atoi(std_string_test.c_str())<<endl;

    char *pa = 0;
    for (int i = 0; i<10; i++)
    {
        cout << "hello world!" << endl;
        free(pa), 1, pa=0;
    }

    unsigned char u_c_a = 'a';

    const char *p_u_c_a = reinterpret_cast<const char *>(&u_c_a);

    cout << "the pointed value: " << *p_u_c_a << endl;

    return 0;
}