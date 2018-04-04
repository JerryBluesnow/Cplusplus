<<<<<<< HEAD
struct IMS_ACL_RANGE_KEY : public IMSDC_KEY_BASE
{
    USHORT            acl_table_id;
    DM_V4V6IPADDRESS *ip_addr_low;
    DM_V4V6IPADDRESS *ip_addr_high;
    USHORT            port_low;
    USHORT            port_high;

    inline void
    dump(ostream &os = cout)
    {
        os << "IMS_ACL_RANGE_KEY:: acl_table_id [" << dec << acl_table_id << "] " << endl
           << "ip_addr_low [" << hex << *ip_addr_low << "] "
           << "ip_addr_high [" << hex << *ip_addr_high << "]" << endl
           << "port_low [" << dec << port_low << "] "
           << "port_high " << dec << port_high << endl;
    }

    IMS_ACL_RANGE_KEY()
    {
        acl_table_id = 0;
        ip_addr_low  = NULL;
        ip_addr_high = NULL;
        port_low     = 0;
        port_high    = 0;
    }

    IMS_ACL_RANGE_KEY(const IMS_ACL_RANGE_KEY &k)
    {
        acl_table_id = k.acl_table_id;
        ip_addr_low  = k.ip_addr_low;
        ip_addr_high = k.ip_addr_high;
        port_low     = k.port_low;
        port_high    = k.port_high;
    }

    inline static bool
    build_key(IMS_ACL_RANGE_KEY &key,
              USHORT             acl_table_id,
              DM_V4V6IPADDRESS & ip_addr_low,
              DM_V4V6IPADDRESS & ip_addr_high,
              USHORT             port_low,
              USHORT             port_high)
    {
        key.acl_table_id = acl_table_id;
        key.ip_addr_low  = &ip_addr_low;
        key.ip_addr_high = &ip_addr_high;
        key.port_low     = port_low;
        key.port_high    = port_high;
        return true;
    }

    struct key_compare
    {
        bool
        operator()(IMS_ACL_RANGE_KEY s1, IMS_ACL_RANGE_KEY s2) const
        {
            if (s1.acl_table_id < s2.acl_table_id)
            {
                return true;
            }

            else if (s1.acl_table_id > s2.acl_table_id)
            {
                return false;
            }

            else
            {
                // 从数据库中查找s2 
                if (isZero(*s2.ip_addr_high) && s2.port_high == 0)
                {
                    // Find single transport
                    if (*s1.ip_addr_high < *s2.ip_addr_low)
                    {
                        return true;
                    }
                    else
                    {
                        // Check the port range
                        if (s1.port_high < s2.port_low)
                        {
                            return true;
                        }
                        else
                        {
                            return false;
                        }
                    }
                }

                if ((isZero(*s1.ip_addr_high)) && (s1.port_high == 0))
                {
                    // Find single transport
                    if (*s1.ip_addr_low < *s2.ip_addr_low)
                    {
                        return true;
                    }
                    else
                    {
                        if (s1.port_low < s2.port_low)
                        {
                            return true;
                        }
                        else
                        {
                            return false;
                        }
                    }
                }

                if (*s1.ip_addr_low < *s2.ip_addr_low)
                {
                    if (!(*s1.ip_addr_high > *s2.ip_addr_high))
                    {
                        return true;
                    }
                    else
                    {
                        // Report error
                        IMS_DLOG(IMS_LOGHIGH, "ip range conflict!");
                        s1.dump();
                        s2.dump();
                        return false;
                    }
                }
                else if (*s1.ip_addr_low > *s2.ip_addr_low)
                {
                    if (!(*s1.ip_addr_high < *s2.ip_addr_high))
                    {
                        return false;
                    }
                    else
                    {
                        // Report error
                        IMS_DLOG(IMS_LOGHIGH, "ip range conflict!");
                        s1.dump();
                        s2.dump();
                        return false;
                    }
                }
                else
                {
                    if (*s1.ip_addr_high < *s2.ip_addr_high)
                    {
                        return true;
                    }
                    else if (*s1.ip_addr_high > *s2.ip_addr_high)
                    {
                        return false;
                    }
                    else
                    {
                        if (s1.port_low < s2.port_low)
                        {
                            if (!(s1.port_high > s2.port_high))
                            {
                                return true;
                            }
                            else
                            {
                                // Report error
                                IMS_DLOG(IMS_LOGHIGH, "ip range conflict!");
                                s1.dump();
                                s2.dump();
                                return false;
                            }
                        }
                        else if (s1.port_low > s2.port_low)
                        {
                            if (!(s1.port_high < s2.port_high))
                            {
                                return false;
                            }
                            else
                            {
                                // Report error
                                IMS_DLOG(IMS_LOGHIGH, "ip range conflict!");
                                s1.dump();
                                s2.dump();
                                return false;
                            }
                        }
                        else
                        {
                            // Report error
                            if (s1.port_high < s2.port_high)
                            {
                                return true;
                            }
                            else if (s1.port_high > s2.port_high)
                            {
                                return false;
                            }
                            else
                            {
                                return false;
                            }
                        }
                    }
                }
            }
        }
    };
=======
#include <iostream>
using namespace std;  

inline ostream & operator << (ostream &output, IP_ADDRESS_TYPE &data){
                output << (int)data;
                return output;
}

inline ostream & operator << (ostream &output, DM_V4V6IPADDRESS &data){
                output << "ip_type is "<<(int)data.ip_type << endl;
                output << "ip_addr is [";
                for(int i =0; i< MAX_IPv6_ADDR_LEN; i++){
                        output<< hex << (int)data.addr[i]<<dec <<":";
                }
		output<<"]"<<endl;
                return output;
}

inline bool isZero( DM_V4V6IPADDRESS &c1)
{
	for(int i = MAX_IPv6_ADDR_LEN - sizeof(unsigned int); i >=0; i-=sizeof(unsigned int)){
		if(*(unsigned int*)&c1.addr[i] != 0) return false;
	}
	return true;
}


inline int IPcompare( DM_V4V6IPADDRESS &c1, DM_V4V6IPADDRESS &c2)
{
	// IPv4 < IPv6
	if(c1.ip_type < c2.ip_type) return -1;
	if(c1.ip_type > c2.ip_type) return 1;

	return memcmp(c1.addr, c2.addr, MAX_IPv6_ADDR_LEN);;
}

inline bool operator ==( DM_V4V6IPADDRESS &c1,DM_V4V6IPADDRESS &c2)
{
        return IPcompare(c1, c2) == 0;
}

inline bool operator >( DM_V4V6IPADDRESS &c1,DM_V4V6IPADDRESS &c2)
{
        return IPcompare(c1, c2) > 0;
}

inline bool operator <( DM_V4V6IPADDRESS &c1,DM_V4V6IPADDRESS &c2)
{
	return IPcompare(c1, c2) < 0;
}

struct IMS_ACL_RANGE_KEY: public IMSDC_KEY_BASE
{
	USHORT acl_table_id;
	DM_V4V6IPADDRESS *ip_addr_low;
	DM_V4V6IPADDRESS *ip_addr_high;
	USHORT port_low;
	USHORT port_high;

	inline void dump(ostream &os = cout)
	{
		os << "IMS_ACL_RANGE_KEY:: acl_table_id [" << dec << acl_table_id << "] " << endl
		   << "ip_addr_low [" << hex << *ip_addr_low << "] "
		   << "ip_addr_high [" << hex << *ip_addr_high << "]" << endl
		   << "port_low [" << dec << port_low << "] "
		   << "port_high " << dec << port_high << endl;
	}

	IMS_ACL_RANGE_KEY()
	{
		acl_table_id = 0;
		ip_addr_low = NULL;
		ip_addr_high = NULL;
		port_low = 0;
		port_high = 0;
	}

	IMS_ACL_RANGE_KEY(const IMS_ACL_RANGE_KEY &k)
	{
		acl_table_id = k.acl_table_id;
		ip_addr_low = k.ip_addr_low;
		ip_addr_high = k.ip_addr_high;
		port_low = k.port_low;
		port_high = k.port_high;
	}

	inline static bool build_key(IMS_ACL_RANGE_KEY &key,
			USHORT acl_table_id, 
			DM_V4V6IPADDRESS& ip_addr_low, DM_V4V6IPADDRESS& ip_addr_high,
			USHORT port_low, USHORT port_high)
	{
		key.acl_table_id = acl_table_id;
		key.ip_addr_low = &ip_addr_low;
		key.ip_addr_high = &ip_addr_high;
		key.port_low = port_low;
		key.port_high = port_high;
		return true;
	}

  struct key_compare
  {
      bool
      operator()(IMS_ACL_RANGE_KEY s1, IMS_ACL_RANGE_KEY s2) const
      {
          if (s1.acl_table_id < s2.acl_table_id)
          {
              return true;
          }

          if (s1.acl_table_id > s2.acl_table_id)
          {
              return false;
          }

		  // when 
          if (isZero(*s2.ip_addr_high) && s2.port_high == 0)
          {
              // Find single transport
              if (*s1.ip_addr_high < *s2.ip_addr_low)
              {
                  return true;
              }
              else
              {
                  // Check the port range
                  if (s1.port_high < s2.port_low)
                  {
                      return true;
                  }
                  else
                  {
                      return false;
                  }
              }
          }

          if ((isZero(*s1.ip_addr_high)) && (s1.port_high == 0))
          {
              // Find single transport
              if (*s1.ip_addr_low < *s2.ip_addr_low)
              {
                  return true;
              }
              else
              {
                  if (s1.port_low < s2.port_low)
                  {
                      return true;
                  }
                  else
                  {
                      return false;
                  }
              }
          }

          if (*s1.ip_addr_low < *s2.ip_addr_low)
          {
              if (!(*s1.ip_addr_high > *s2.ip_addr_high))
              {
                  return true;
              }
              else
              {
                  // Report error
                  IMS_DLOG(IMS_LOGHIGH, "ip range conflict!");
                  s1.dump();
                  s2.dump();
                  return false;
              }
          }
          else if (*s1.ip_addr_low > *s2.ip_addr_low)
          {
              if (!(*s1.ip_addr_high < *s2.ip_addr_high))
              {
                  return false;
              }
              else
              {
                  // Report error
                  IMS_DLOG(IMS_LOGHIGH, "ip range conflict!");
                  s1.dump();
                  s2.dump();
                  return false;
              }
          }
          else
          {
              if (*s1.ip_addr_high < *s2.ip_addr_high)
              {
                  return true;
              }
              else if (*s1.ip_addr_high > *s2.ip_addr_high)
              {
                  return false;
              }
              else
              {
                  if (s1.port_low < s2.port_low)
                  {
                      if (!(s1.port_high > s2.port_high))
                      {
                          return true;
                      }
                      else
                      {
                          // Report error
                          IMS_DLOG(IMS_LOGHIGH, "ip range conflict!");
                          s1.dump();
                          s2.dump();
                          return false;
                      }
                  }
                  else if (s1.port_low > s2.port_low)
                  {
                      if (!(s1.port_high < s2.port_high))
                      {
                          return false;
                      }
                      else
                      {
                          // Report error
                          IMS_DLOG(IMS_LOGHIGH, "ip range conflict!");
                          s1.dump();
                          s2.dump();
                          return false;
                      }
                  }
                  else
                  {
                      // Report error
                      if (s1.port_high < s2.port_high)
                      {
                          return true;
                      }
                      else if (s1.port_high > s2.port_high)
                      {
                          return false;
                      }
                      else
                      {
                          return false;
                      }
                  }
              }
          }
      }
  };
>>>>>>> 312c9226ca4a54424252e12840a3e71e9667f469
