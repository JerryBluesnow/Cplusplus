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

int
main()
{
    std::string usr_name_string(
        "sip:+1401718123;npdi;rn=1234567890@10.84.2.10:50004;transport=udp;user=phone");

    std::string new_usr_string("+185000099998888");

    // plase use trim to process the string

    // replace usr number
    usr_name_string=usr_name_string.replace(usr_name_string.find("sip:")+strlen("sip:"),usr_string.length(),new_usr_string);   

    // remove npdi
    if (usr_name_string.find(";npdi") != std::string::npos)
    {
        usr_name_string = usr_name_string.replace(usr_name_string.find(";npdi"), strlen(";npdi"), "");
    }

    // replace rn
    if (usr_name_string.find(";rn=") != std::string::npos)
    {
        std::string rn_string = string_find_value(usr_name_string, ";rn=", 5, '>', ' ', '@', ';', '\0');
        usr_name_string            = usr_name_string.replace(usr_name_string.find(";rn="),
                                        strlen(";rn=") + rn_string.length(), ";rn=helloworld");
    }
   
    cout << usr_name_string << endl;

    // remove rn
    if (usr_name_string.find(";rn=") != std::string::npos)
    {
        std::string rn_string = string_find_value(usr_name_string, ";rn=", 5, '>', ' ', '@', ';', '\0');
        usr_name_string =
            usr_name_string.replace(usr_name_string.find(";rn="), strlen(";rn=") + rn_string.length(), "");
    }

    // if rn doest not exist
   if (usr_name_string.find(";rn=") == std::string::npos)
   {
       //
   }

   sipuri_ptr = URI_getSipUri(req_uri_ptr);

   if (IS_VALID_PTR(sipuri_ptr) == FALSE)
   {
       IMS_DLOG(IMS_IBCF_LOGHIGH, "%s, sipuri_ptr invalid pointer.", __func__);
       return;
   }

   DString *ds_user_name = Sip_URI_getUserName(sipuri_ptr);

   if (IS_VALID_PTR(ds_user_name) == FALSE)
   {
       IMS_DLOG(IMS_IBCF_LOGHIGH, "%s, user name in sip uri is invalid.", __func__);
       return;
   }

   std::string usr_name_string;
   usr_name_string.assign(DString_getStr(ds_user_name), DString_getLen(ds_user_name));

   // If egress TG is SIPI TG, then IAM is encapsulated into INVITE MIME
   // if either egress or ingress TG is SIPI TG,
   // R-URI's called party DN should be gotten from GCC, for GCC already
   // got the idea of what called DN in R-URI should be
   sipi_profile_id = egress_tg_data_ptr->sipi_profile_id;

   while (1)
   {
       if (p_SipConEvntPtr->reqUri.reqUriVal.len == 0)
       {
           break;
       }

       // there maybe noise information in the end of the reqUriVal.val, as there is no
       // mandatory '\0'. most of the time, there is no noise.
       IMS_DLOG(IMS_IBCF_LOGLOWEST, "%s, p_SipConEvntPtr->reqUri.reqUriVal.val(i.e userPart) is %s",
                __func__, p_SipConEvntPtr->reqUri.reqUriVal.val);

       std::string new_usr_string;
       new_usr_string.assgin(p_SipConEvntPtr->reqUri.reqUriVal.val,
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

       if (usr_string.length() != 0)
       {
           usr_name_string = usr_name_string.replace(
               usr_name_string.find(sip_uri_type_str) + strlen(sip_uri_type_str),
               usr_string.length(), new_usr_string);
       }

       IMS_DLOG(IMS_IBCF_LOGLOWEST, "%s, after replace Number, user_name[%s].", __func__,
                usr_name_string.c_str());
       break;
   }

   while (1)
   {
       std::string            new_rn_string;
       std::string::size_type rn_pos = usr_name_string.find(";rn=");

       if (!((p_SipConEvntPtr->rn.eh.pres == PRSNT_NODEF) &&
             (p_SipConEvntPtr->rn.rnVal.pres == PRSNT_NODEF)))
       {
           // need to remove rn if existing in R-URI
           if (rn_pos == std::string::npos)
           {
               // no rn
               break;
           }

           std::string rn_string =
               string_find_value(usr_name_string, ";rn=", 5, '>', ' ', '@', ';', '\0');

           usr_name_string = usr_name_string.replace(usr_name_string.find(";rn="),
                                                     strlen(";rn=") + rn_string.length(), "");

           break;
       }

       new_rn_string.assign(p_SipConEvntPtr->rn.rnVal.val, p_SipConEvntPtr->rn.rnVal.len);
       if (new_rn_string.length() == 0)
       {
           break;
       }

       // need to add add rn
       if (rn_pos == std::string::npos)
       {
           new_rn_string = ";rn=" + new_rn_string;
           usr_name_string =
               usr_name_string.replace(usr_name_string.find("@") - 1, 0, new_rn_string.c_str());
           break;
       }

       // need to replace rn
       std::string rn_string =
           string_find_value(usr_name_string, ";rn=", 5, '>', ' ', '@', ';', '\0');

       usr_name_string =
           usr_name_string.replace(usr_name_string.find(";rn="),
                                   strlen(";rn=") + rn_string.length(), new_rn_string.c_str());
       break;
      }

      while (1)
      {
          // remove npdi
          if (usr_name_string.find(";npdi=") == std::string::npos)
          {
              std::string npdi_string =
                  string_find_value(usr_name_string, ";npdi=", 5, '>', ' ', '@', ';', '\0');

              usr_name_string = usr_name_string.replace(
                  usr_name_string.find(";npdi="), strlen(";npdi=") + npdi_string.length(), "");
          }
          else if (usr_name_string.find(";npdi") == std::string::npos)
          {
              usr_name_string =
                  usr_name_string.replace(usr_name_string.find(";npdi"), strlen(";npdi"), "");
          }

          // need to remove npdi
          if ((npdi.pres != PRSNT_NODEF) || (npdi.val == 0))
          {
              break;
          }

          if (usr_name_string.find("@") == std::string::npos)
          {
              break;
          }

          // need to add npdi=yes
          usr_name_string = usr_name_string.replace(usr_name_string.find("@"), strlen("@"), ";npdi=yes@");
      }

      return 0;
}