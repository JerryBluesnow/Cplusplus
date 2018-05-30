#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <iostream>
using namespace std;

#define MAX_LENGTH 16
#define MAX_IPv6_ADDR_LEN MAX_LENGTH
#define MAX_IPv4_ADDR_LEN 4

typedef enum {
        IPV4_IP = 0,
        IPV6_IP = 1
} IP_ADDRESS_TYPE;

typedef struct DM_V4V6IPADDRESS{
    IP_ADDRESS_TYPE ip_type;
    unsigned char addr[MAX_IPv6_ADDR_LEN];

    DM_V4V6IPADDRESS(IP_ADDRESS_TYPE input_ip_type, const char* input_addr){
        ip_type = input_ip_type;
        for (int ix=0,iy=0; iy<MAX_IPv6_ADDR_LEN; ix++,iy++)
        {
            char data_tmp[3];
            data_tmp[2] = 0;
            data_tmp[0] = input_addr[ix];
            ix++;
            data_tmp[1] = input_addr[ix];

            for (int i = 0; i < 2; i++)
            {
                if (data_tmp[i] >= '0' && data_tmp[i] <= '9')
                {
                    data_tmp[i] = data_tmp[i] - '0';
                }
                else if (data_tmp[i] >= 'a' && data_tmp[i] <= 'z')
                {
                    data_tmp[i] = data_tmp[i] - 'a' + 10;
                }
            }

            addr[iy] = data_tmp[0] * 16 + data_tmp[1];
        }
    }
    DM_V4V6IPADDRESS() {}
    DM_V4V6IPADDRESS(int) {}
} DM_V4V6IPADDRESS;

void
plus_one(DM_V4V6IPADDRESS &obj, unsigned int length)
{
    bool carry_bit = false;
    for (unsigned int idx = length; idx > 0; idx--)
    {
        if ((idx == length) || (carry_bit == true))
        {
            obj.addr[idx - 1] += 0x01;
        }

        if ((obj.addr[idx - 1] == 0x00) && ((idx == length) || (carry_bit == true)))
        {
            carry_bit = true;
        }
        else
        {
            carry_bit = false;
        }
    }

    return;
}

void
minus_one(DM_V4V6IPADDRESS &obj, unsigned int length)
{
    bool carry_bit = false;
    for (unsigned int idx = length; idx > 0; idx--)
    {
        if ((idx == length) || (carry_bit == true))
        {
            obj.addr[idx - 1] -= 0x01;
        }

        if ((obj.addr[idx - 1] == 0xff) && ((idx == length) || (carry_bit == true)))
        {
            carry_bit = true;
        }
        else
        {
            carry_bit = false;
        }
    }

    return;
}

// prefix
DM_V4V6IPADDRESS &
operator++(DM_V4V6IPADDRESS &obj)
{
    unsigned int length = (obj.ip_type == IPV4_IP) ? MAX_IPv4_ADDR_LEN : MAX_IPv6_ADDR_LEN;

    plus_one(obj, length);

    return obj;
}

// prefix
DM_V4V6IPADDRESS &
operator--(DM_V4V6IPADDRESS &obj)
{
    unsigned int length = (obj.ip_type == IPV4_IP) ? MAX_IPv4_ADDR_LEN : MAX_IPv6_ADDR_LEN;

    minus_one(obj, length);

    return obj;
}

inline int IPcompare(const DM_V4V6IPADDRESS &c1,const DM_V4V6IPADDRESS &c2)
{
        // IPv4 < IPv6
        if(c1.ip_type < c2.ip_type) return -1;
        if(c1.ip_type > c2.ip_type) return 1;

        return memcmp(c1.addr, c2.addr, MAX_IPv6_ADDR_LEN);;
}

inline bool operator ==(const DM_V4V6IPADDRESS &c1, const DM_V4V6IPADDRESS &c2)
{
        return IPcompare(c1, c2) == 0;
}

inline bool operator >(const DM_V4V6IPADDRESS &c1, const DM_V4V6IPADDRESS &c2)
{
        return IPcompare(c1, c2) > 0;
}

inline bool operator <(const DM_V4V6IPADDRESS &c1, const DM_V4V6IPADDRESS &c2)
{
        return IPcompare(c1, c2) < 0;
}

inline bool operator >=(const DM_V4V6IPADDRESS &c1, const DM_V4V6IPADDRESS &c2)
{
        return !(IPcompare(c1, c2) < 0);
}

inline bool operator <=(const DM_V4V6IPADDRESS &c1, const DM_V4V6IPADDRESS &c2)
{
        return !(IPcompare(c1, c2) > 0);
}


typedef struct DM_V4V6IPADDRESS_PTR{
    DM_V4V6IPADDRESS *value;
    DM_V4V6IPADDRESS_PTR(DM_V4V6IPADDRESS *value):value(value)
    {
    }
}DM_V4V6IPADDRESS_PTR;

inline bool operator ==(const DM_V4V6IPADDRESS_PTR &c1, const DM_V4V6IPADDRESS_PTR &c2)
{
        return IPcompare(*c1.value, *c2.value) == 0;
}

inline bool operator >(const DM_V4V6IPADDRESS_PTR &c1, const DM_V4V6IPADDRESS_PTR &c2)
{
        return IPcompare(*c1.value, *c2.value) > 0;
}

inline bool operator <(const DM_V4V6IPADDRESS_PTR &c1, const DM_V4V6IPADDRESS_PTR &c2)
{
        return IPcompare(*c1.value, *c2.value) < 0;
}

inline bool operator >=(const DM_V4V6IPADDRESS_PTR &c1, const DM_V4V6IPADDRESS_PTR &c2)
{
        return !(IPcompare(*c1.value, *c2.value) < 0);
}

inline bool operator <=(const DM_V4V6IPADDRESS_PTR &c1, const DM_V4V6IPADDRESS_PTR &c2)
{
        return !(IPcompare(*c1.value, *c2.value) > 0);
}


inline ostream &
operator<<(ostream &output, const DM_V4V6IPADDRESS &data)
{
    output << "ip_type[" << (int)data.ip_type<<"]";
    output << "ip_addr[";
    for (int i = 0; i < MAX_IPv6_ADDR_LEN; i++)
    {
        output << hex << (int)data.addr[i] << dec;
        if ( i != MAX_IPv6_ADDR_LEN - 1)
        {
            output << ":";
        }     
    }
    output << "]" ;
    return output;
}

/*
int main()
{
    DM_V4V6IPADDRESS input_addr (IPV6_IP, "2511000000020cc30000000000010004");

    DM_V4V6IPADDRESS data_1_low (IPV6_IP, "30000000000000000000000000000001");
    DM_V4V6IPADDRESS data_1_high(IPV6_IP, "3fff000000000000000000000000ffff");

    DM_V4V6IPADDRESS data_2_low (IPV6_IP, "20000000000000000000000000000001");
    DM_V4V6IPADDRESS data_2_high(IPV6_IP, "3000000000000000000000000000ffff");

    cout << "IPcompare input_addr data_1_low return: " << IPcompare(input_addr,data_1_low) << endl;
    cout << "IPcompare input_addr data_1_high return: " << IPcompare(input_addr,data_1_high) << endl;
    cout << "IPcompare input_addr data_2_low return: " << IPcompare(input_addr,data_2_low) << endl;
    cout << "IPcompare input_addr data_2_high return: " << IPcompare(input_addr,data_2_high) << endl;

    return 0;
}
*/