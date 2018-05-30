#include <iostream>
#include <vector>
#include <algorithm>
#include <functional>
#include <stdarg.h>
#include "boost/algorithm/string.hpp"
#include "boost/lambda/lambda.hpp"
#include "boost/lambda/bind.hpp"

using namespace boost::algorithm;

using namespace std;

std::string
string_find_value(std::string source_str, std::string key_str, int separator_cnt, ...)
{
    if (source_str.length() == 0 || key_str.length() == 0)
    {
        return ("");
    }

    va_list arg_ptr;

    va_start(arg_ptr, separator_cnt);

    trim_if(source_str, is_any_of(" "));

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

typedef struct _reqUriVal
{
    int len;
    char val[100];
} _reqUriVal;

typedef enum PRESENT_ENUM
{
    PRSNT_NODEF,
    NOTPREST,
} PRESENT_ENUM;

//p_SipConEvntPtr->rn.rnVal.val
typedef struct _rnVal
{
    int  len;
    char val[100];
    PRESENT_ENUM pres;
} _rnVal;

typedef struct _eh
{
    PRESENT_ENUM pres;
} _eh;

typedef struct _RN{
    _rnVal rnVal;
    _eh    eh;
}RN;

typedef struct _reqUri
{
    _reqUriVal reqUriVal;
}_reqUri;

typedef struct SipConEvnt
{
    _reqUri reqUri;
    _RN     rn;
    SipConEvnt(const char *value, int length, const char* rn_value, int rn_length)
    {
        strncpy(reqUri.reqUriVal.val, value, length);
        reqUri.reqUriVal.len = length;
        strncpy(rn.rnVal.val, rn_value, rn_length);
        rn.rnVal.len = rn_length;
    }

} SipConEvnt;

typedef enum sip_type {
    SIP_URI,
    SIPS_URI,
}sip_type;

sip_type URI_getType(std::string req_uri_ptr)
{
    if (req_uri_ptr.find("sips:") != std::string::npos )
    {
        return SIPS_URI;
    }

    if (req_uri_ptr.find("sip:") != std::string::npos )
    {
        return SIP_URI;
    }

    return SIP_URI;
}

void replace_uri(SipConEvnt *p_SipConEvntPtr, std::string &usr_name_string, std::string req_uri_ptr)
{
   std::cout << "before reqUriVal update "<< endl  << usr_name_string << endl;
    
    while (1)
    {
        if (p_SipConEvntPtr->reqUri.reqUriVal.len == 0)
        {
            break;
        }

         std::string new_usr_string;
        new_usr_string.assign(p_SipConEvntPtr->reqUri.reqUriVal.val,
                              p_SipConEvntPtr->reqUri.reqUriVal.len);

        std::string sip_uri_type_str;
        if (URI_getType(req_uri_ptr) == SIP_URI)
        {
            sip_uri_type_str.append("sip:");
        }
        else
        {
            sip_uri_type_str.append("sips:");
        }

        std::string usr_string =
            string_find_value(usr_name_string, sip_uri_type_str, 5, '>', ' ', '@', ';', '\0');

        usr_name_string = usr_name_string.replace(
            usr_name_string.find(sip_uri_type_str) + sip_uri_type_str.length(), usr_string.length(),
            new_usr_string);

        break;
   }

   std::cout << "after reqUriVal update "<< endl << usr_name_string << endl;

   while (1)
   {
       std::string            new_rn_string;
       std::string::size_type rn_pos = usr_name_string.find(";rn=");

       // need to remove rn if existing in R-URI
       if (rn_pos != std::string::npos)
       {
           std::string rn_string =
               string_find_value(usr_name_string, ";rn=", 5, '>', ' ', '@', ';', '\0');

           usr_name_string = usr_name_string.replace(usr_name_string.find(";rn="),
                                                     strlen(";rn=") + rn_string.length(), "");
       }

       if (!((p_SipConEvntPtr->rn.eh.pres == PRSNT_NODEF) &&
             (p_SipConEvntPtr->rn.rnVal.pres == PRSNT_NODEF)))
       {
           break;
       }

       new_rn_string.assign(p_SipConEvntPtr->rn.rnVal.val, p_SipConEvntPtr->rn.rnVal.len);
       if (new_rn_string.length() == 0)
       {
           break;
       }

       // need to add add rn
       new_rn_string = ";rn=" + new_rn_string;
       usr_name_string =
           usr_name_string.replace(usr_name_string.find("@"), 0, new_rn_string.c_str());

       break;
      }

      std::cout << "after rn update "<< endl << usr_name_string << endl;

      while (1)
      {
          // remove npdi
          if (usr_name_string.find(";npdi=") != std::string::npos)
          {
              std::string npdi_string =
                  string_find_value(usr_name_string, ";npdi=", 5, '>', ' ', '@', ';', '\0');

              usr_name_string = usr_name_string.replace(
                  usr_name_string.find(";npdi="), strlen(";npdi=") + npdi_string.length(), "");
          }
          else if (usr_name_string.find(";npdi") != std::string::npos)
          {
              usr_name_string =
                  usr_name_string.replace(usr_name_string.find(";npdi"), strlen(";npdi"), "");
          }

        // need to remove npdi
        //   if ((npdi.pres != PRSNT_NODEF) || (npdi.val == 0))
        //   {
        //       break;
        //   }

          if (usr_name_string.find("@") == std::string::npos)
          {
              break;
          }

          // need to add npdi=yes
          usr_name_string = usr_name_string.replace(usr_name_string.find("@"), strlen("@"), ";npdi=yes@");
          break;
      }
     std::cout << "after npdi update "<< endl << usr_name_string << endl;
}

int
main()
{
    std::string usr_name_string1("1401718123;npdi;rn=14053200000");
    std::string usr_name_string2("14017181123123122323;npdi;rn=14053200000");
    usr_name_string2 = usr_name_string1;
    cout << usr_name_string2 << endl;
    while (1)
    {
        std::string usr_name_string("1401718123;npdi;rn=14053200000");

        std::string            new_rn_string;
        std::string::size_type rn_pos = usr_name_string.find(";rn=");

        // need to remove rn if existing in R-URI
        if (rn_pos != std::string::npos)
        {
            std::string rn_string =
                string_find_value(usr_name_string, ";rn=", 5, '>', ' ', '@', ';', '\0');

            usr_name_string = usr_name_string.replace(usr_name_string.find(";rn="),
                                                      strlen(";rn=") + rn_string.length(), "");
        }

        // if (!((p_SipConEvntPtr->rn.eh.pres == PRSNT_NODEF) &&
        //       (p_SipConEvntPtr->rn.rnVal.pres == PRSNT_NODEF)))
        // {
        //     break;
        // }

        new_rn_string.assign(reinterpret_cast<char *>(0),
                             0);
        if (new_rn_string.length() == 0)
        {
            break;
        }

        // need to add add rn
        new_rn_string = ";rn=" + new_rn_string;
        usr_name_string =
            usr_name_string.replace(usr_name_string.find("@"), 0, new_rn_string.c_str());

        break;
    }
    {
        std::string usr_name_string(
            "sip:+1401718123;npdi;rn=1234567890");

        SipConEvnt my_sipConEvent("+86192168999988887777", strlen("+86192168999988887777"), "+1999888777", strlen("+1999888777"));
        replace_uri(&my_sipConEvent, usr_name_string, "sip:");
    }

        {
        std::string usr_name_string(
            "sip:+1401718123;npdi;rn=1234567890@10.84.2.10:50004;transport=udp;user=phone");

        SipConEvnt my_sipConEvent("+86192168999988887777", strlen("+86192168999988887777"), "", strlen(""));
        replace_uri(&my_sipConEvent, usr_name_string, "sip:");
    }

    {
        std::string usr_name_string(
            "sip:+1401718123;npdi;rn=1234567890@10.84.2.10:50004;transport=udp;user=phone");

        SipConEvnt my_sipConEvent("+86192168999988887777", strlen("+86192168999988887777"), "+1999888777", strlen("+1999888777"));
        replace_uri(&my_sipConEvent, usr_name_string, "sip:");
    }


    {
        std::string usr_name_string(
            "sip:+1401718123;npdi=;rn=1234567890@10.84.2.10:50004;transport=udp;user=phone");

        SipConEvnt my_sipConEvent("+86192168999988887777", strlen("+86192168999988887777"), "+1999888777", strlen("+1999888777"));
        replace_uri(&my_sipConEvent, usr_name_string, "sip:");
    }

    {
        std::string usr_name_string(
            "sip:+1401718123;npdi=yes;rn=1234567890@10.84.2.10:50004;transport=udp;user=phone");

        SipConEvnt my_sipConEvent("+86192168999988887777", strlen("+86192168999988887777"), "+1999888777", strlen("+1999888777"));
        replace_uri(&my_sipConEvent, usr_name_string, "sip:");
    }

    {
        std::string usr_name_string(
            "sip:+1401718123;npdi=yes;rn=@10.84.2.10:50004;transport=udp;user=phone");

        SipConEvnt my_sipConEvent("+86192168999988887777", strlen("+86192168999988887777"), "+1999888777", strlen("+1999888777"));
        replace_uri(&my_sipConEvent, usr_name_string, "sip:");
    }


    // {
    //     std::string usr_name_string(
    //         "sip:+1401718123@10.84.2.10:50004;transport=udp;user=phone");

    //     SipConEvnt my_sipConEvent("+86192168999988887777", strlen("+86192168999988887777"), "+1999888777", strlen("+1999888777"));
    //     replace_uri(&my_sipConEvent, usr_name_string, "sip:");
    // }

    // {
    //     std::string usr_name_string(
    //         "sip:+1401718123@10.84.2.10:50004;transport=udp;user=phone");

    //     SipConEvnt my_sipConEvent("+861921689999888877776666", strlen("+86192168999988887777"), "+1999888777", strlen("+1999888777"));
    //     replace_uri(&my_sipConEvent, usr_name_string, "sip:");
    // }

    return 0;
}