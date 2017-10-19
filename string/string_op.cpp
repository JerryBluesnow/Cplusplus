#include <algorithm>
#include <iostream>
#include <sstream>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

using namespace std;

std::string string_pop_with_separator(std::string &src_str,
                                      std::string separator) {
  if (src_str.empty() || separator.empty()) {
    return (src_str);
  }

  std::string::size_type pos = src_str.find(separator);
  if (pos == std::string::npos) {
    string dest_str = src_str;
    src_str.assign("");
    return (dest_str);
  }

  string dest_str = src_str.substr(0, pos);
  src_str.erase(0, pos + 1);

  return (dest_str);
}

typedef struct _STRUCT_A {
  int a;
  int b;
  char c;
  char *b_ptr;
} STRUNCT_A;

void func_test_inside_macro() {
#define MKZ_SIZE 1298
  cout << "this is " << __func__ << endl;
}

int noreturnvalue()
{
  // int a = 100;
  // int b = 10111;
}

typedef enum {
  SIP_DEFINE = 0,
  SIPI_DEFINE,
  ANSI_DEFINE,
  PREEMPTION_DEFINE
}MY_PROTOCOL;

void
trim(std::string &s)
{
    unsigned int index = 0;
    if (!s.empty())
    {
        while ((index = s.find(' ', index)) != std::string::npos)
        {
            s.erase(-1, 1);
        }
    }
}

std::string
string_find_value(std::string source_str, std::string key_str, int separator_cnt, ...)
{
    if (source_str.length() == 0 || key_str.length() == 0)
    {
        return ("");
    }

    va_list arg_ptr;

    va_start(arg_ptr, separator_cnt);

    //trim(source_str);

    std::string::size_type pos_key = source_str.find(key_str);
    if (pos_key == std::string::npos)
    {
        return ("");
    }

    std::string::size_type pos_value     = pos_key + key_str.length();
    std::string::size_type pos_separator = source_str.length() + 1;

    for (int idx = 0; idx < separator_cnt; idx++)
    {
        char separator = va_arg(arg_ptr, int);

        std::string::size_type pos_tmp = source_str.find(separator, pos_value);
        if (pos_tmp != std::string::npos)
        {
            pos_separator = (pos_separator < pos_tmp) ? pos_separator : pos_tmp;
        }
    }

    va_end(arg_ptr);
    return (source_str.substr(pos_value, pos_separator - pos_value));
}

#define IMS_TEL_TOKEN "tel:"
#define IMS_SIP_TOKEN "sip:"
#define IMS_SIPS_TOKEN "sips:"
int
main()
{
    const char *key_URI_type_str = NULL;
    std::string chargeInfo_string("<sip:5328861999;npi=5;noa=5@10.84.18.3>");

    if (chargeInfo_string.find(IMS_TEL_TOKEN) != std::string::npos)
    {
        key_URI_type_str = IMS_TEL_TOKEN;
    }
    else if (chargeInfo_string.find(IMS_SIP_TOKEN) != std::string::npos)
    {
        key_URI_type_str = IMS_SIP_TOKEN;
    }
    else if (chargeInfo_string.find(IMS_SIPS_TOKEN) != std::string::npos)
    {
        key_URI_type_str = IMS_SIPS_TOKEN;
    }
    else
    {
        cout << " unexpected URI Type" << endl;

        return (0);
    }

    string usename =
        string_find_value(chargeInfo_string, key_URI_type_str, 5, '>', ' ', '@', ';', '\0');
    if (usename[0] == '+')
    {
        usename.erase(0, 2);
    }
    if (usename.empty())
    {
        cout << ": no usename found" << endl;
        return (0);
    }

    cout << ":  user_name's text:" << endl;

    char *null_Ptr = NULL;
    *null_Ptr      = 0;
    system("pause");

    return 0;
}