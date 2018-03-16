#ifndef IMSDC_MAPS_H_
#define IMSDC_MAPS_H_

/*
 * Copyright (c) 2007 - 2008 Alcatel-Lucent
 * All Rights Reserved
 * This is unpublished proprietary source code of Alcatel-Lucent.
 * The copyright notice above does not evidence any actual
 * or intended publication of such source code.
 *
 * Not to be disclosed or used except in accordance with applicable
 * agreements.
 */

/*
 * File: ssp/ds/ims/include/IMSdc_maps.h
 *
 * Description:  definition for IMSdc_maps
 *
 */

#include "IMSmacros.h"
#include "assert_hdr.h"
#include "cnfg_tlv_util.h"
#include "STIgdefs.h"
#include "STIdefs.h"
#include "OSipaddr.h"
#include "IMSdc_common.h"
#include <map>
#include "CPpcscf_vlan_util.h"

class IMSDC_KEY_BASE{
	void dump(ostream &os = cout);
};

template<typename T_VAL>
class IMSDC_MAP_BASE{
public:
	virtual void insert(IMSDC_KEY_BASE &t, T_VAL index) = 0;
	virtual bool empty() = 0;
	virtual void clear() = 0;
	virtual void erase(IMSDC_KEY_BASE &t, BOOL needReport = TRUE) = 0;
	virtual bool find(IMSDC_KEY_BASE &key, T_VAL&val) = 0;
	virtual IMSDC_KEY_BASE *allocateKey() = 0;
	virtual void freeKey(IMSDC_KEY_BASE *key)=0;
	virtual void dump_all(ostream &os = cout)=0;
	virtual void dump_with_key(IMSDC_KEY_BASE &key, ostream &os = cout) =0;

};

inline UINT hash(const char* name, USHORT len) {
	UINT hashsum = 0;
	USHORT i;

	for (i = 0; i < len; i++) {
		hashsum = name[i] + (hashsum << 6) + (hashsum << 16) - hashsum;
	}

	return (hashsum);
}

////////////////////////////////
/////IMSDC_HASH_STRING_MAP_KEY//
////////////////////////////////

struct IMSDC_HASH_STRING_MAP_KEY:public IMSDC_KEY_BASE{
	UINT hash_val;
	const char* str;
	USHORT str_len;

	inline void dump(ostream &os = cout) {
		os << "IMS_HASH_STRING_MAP_KEY:: hash_val [" << hash_val << "]"
		   << " str [" << str << "]" << " str_len ["
	 	   << str_len << "]" << endl;
	}

	static bool build_key(IMSDC_HASH_STRING_MAP_KEY &key,
			const char * str, int len) {
		key.hash_val = 0;
                key.str = NULL;
                key.str_len = 0;
		if (IS_VALID_PTR(str)== FALSE) {
			ASRT_RPT(ASRTBADPARAM, 0, " Invalid vg_member[ %p] with len %d.",
					str, len);
			return false;
		}
		if(len <= 0){
			len = strlen(str);
		}
		key.hash_val = hash(str, len);
		key.str = str;
		key.str_len = len;
		return true;
	}

	struct key_compare {
		bool operator()(const IMSDC_HASH_STRING_MAP_KEY &s1, 
			const IMSDC_HASH_STRING_MAP_KEY &s2) const {

			if (s1.hash_val < s2.hash_val) {
				return true; 
			} else if (s1.hash_val > s2.hash_val) {
				return false; 
			} else {
				if (s1.str_len < s2.str_len) {
					return true; 
				} else if (s1.str_len > s2.str_len) {
					return false; 
				} else {
					return strncmp(s1.str, s2.str, s1.str_len) < 0;
				}
			}
		}
	};
} ;

////////////////////////////////
/////IMSDC_DOMAIN_MAP_KEY///////
////////////////////////////////

struct IMSDC_DOMAIN_MAP_KEY:public IMSDC_KEY_BASE{
	UINT hash_val;
	const char* domain_name;
	USHORT domain_name_len;
	UCHAR domain_type;

	inline void dump(ostream &os = cout) {
		os << "IMSDC_DOMAIN_MAP_KEY:: hash_val [" << hash_val << "]"
		   << " domain_name [" << domain_name << "]" 
		   << " domain_name_len [" << domain_name_len << "]" 
		   << " domain_type [" << (USHORT) domain_type << "]" << endl;
	}

	static bool build_key(IMSDC_DOMAIN_MAP_KEY &key, const char * domain_name,
			int len, UCHAR domain_type) {
		key.hash_val = 0;
		key.domain_name = NULL;
		key.domain_name_len = 0;
		key.domain_type = 0; 
		if (IS_VALID_PTR(domain_name)== FALSE) {
			ASRT_RPT(ASRTBADPARAM, 0, 
				" Invalid vg_member[ %p] with len %d, domain_type %d.",
				domain_name, len, domain_type);
			return false;
		}

		if(len <= 0){
			len = strlen(domain_name);
		}

		key.hash_val = hash(domain_name, len);
		key.domain_name = domain_name;
		key.domain_name_len = len;
		key.domain_type = domain_type;
		return true;
	}
	struct key_compare {
		bool operator()(const IMSDC_DOMAIN_MAP_KEY &s1, 
			const IMSDC_DOMAIN_MAP_KEY &s2) const {

			if (s1.domain_type < s2.domain_type) {
				return true; 
			} else if (s1.domain_type > s2.domain_type) {
				return false; 
			} else {
				if (s1.hash_val < s2.hash_val) {
					return true; 
				} else if (s1.hash_val > s2.hash_val) {
					return false; 
				} else {
					if (s1.domain_name_len < s2.domain_name_len) {
						return true; 
					} else if (s2.domain_name_len > s1.domain_name_len) {
						return false; 
					} else {
						return strncmp(s1.domain_name, s2.domain_name, 
							s1.domain_name_len) < 0;
					}
				}
			}
		}
	};
} ;

////////////////////////////////
/////IMS_VG_IP_KEY//////////////
////////////////////////////////

template<typename T>
struct IMS_VG_IP_KEY:public IMSDC_KEY_BASE{
	T ip_addr;
	T netmask;
	UCHAR type;
	USHORT vlan_id;

	inline void dump(ostream &os = cout) {
		os << "IMS_VG_IP_KEY:: ip_addr [" << hex<<ip_addr << "]" 
		   << " netmask [" << hex << netmask << "]" << dec
		   << " type [" << (USHORT)type << "]"
		   << " vlan_id [" << (USHORT)vlan_id << "]"
		   << endl;
	}

	IMS_VG_IP_KEY<T>() {
		memset(&ip_addr, 0, sizeof(T));
		memset(&netmask, 0, sizeof(T));
		type = 0;
		vlan_id = VLAN_ANY;

	}

	IMS_VG_IP_KEY(const IMS_VG_IP_KEY<T> &k) {
		ip_addr = k.ip_addr;
		netmask = k.netmask;
		type = k.type;
		vlan_id = k.vlan_id;
	}

	inline static bool build_key(IMS_VG_IP_KEY<T> &key, T ip_addr, T netmask, UCHAR type, USHORT vid){
		key.ip_addr = ip_addr;
		key.netmask = netmask;
		key.type = type;
		key.vlan_id = vid;
		return true;
	}

	struct key_compare {
		bool operator()(IMS_VG_IP_KEY<T> s1, IMS_VG_IP_KEY<T> s2) const {

			if (s1.type < s2.type) {
                                return true;
                        } 

			if (s1.type > s2.type) {
                                return false;
      			}

			if (s1.vlan_id < s2.vlan_id) {
				return true;
			}

			if (s1.vlan_id > s2.vlan_id) {
				return false;
			}
 
			if (s1.netmask == 0)
				s1.netmask = s2.netmask;
			if (s2.netmask == 0)
				s2.netmask = s1.netmask;

			if ((s1.ip_addr & s1.netmask) < (s2.ip_addr & s2.netmask)) {
				return true; 
			}
			return false;
		}
	};
} ;

////////////////////////////////
/////IMS_VG_IP_RANGE_KEY////////
////////////////////////////////

struct IMS_VG_IP_RANGE_KEY :public IMSDC_KEY_BASE{
        DM_V4V6IPADDRESS *ip_addr_low;
        DM_V4V6IPADDRESS *ip_addr_high;

        inline void dump(ostream &os = cout) {
                os << "IMS_VG_IP_RANGE_KEY:: ip_addr_low [" << hex << *ip_addr_low << "]"
                   << " ip_addr_high [" << hex << *ip_addr_high << "]" << dec
                   << endl;
        }

        IMS_VG_IP_RANGE_KEY() {
		ip_addr_low = NULL;
		ip_addr_high = NULL;

        }

        IMS_VG_IP_RANGE_KEY(const IMS_VG_IP_RANGE_KEY &k) {
                ip_addr_low = k.ip_addr_low;
                ip_addr_high = k.ip_addr_high;
        }

        inline static bool build_key(IMS_VG_IP_RANGE_KEY &key, DM_V4V6IPADDRESS& ip_addr_low,
                        DM_V4V6IPADDRESS& ip_addr_high) {
                key.ip_addr_low = &ip_addr_low;
                key.ip_addr_high = &ip_addr_high;
                return true;
        }

        struct key_compare {
		bool operator()(IMS_VG_IP_RANGE_KEY s1, IMS_VG_IP_RANGE_KEY s2) const {
			if(isZero(*s2.ip_addr_high)){
				//find single ip
				if (*s1.ip_addr_high < *s2.ip_addr_low){
					return true;
				}
				else {
					return false;
				}
			}

			if(isZero(*s1.ip_addr_high)){
				//find single ip
				if (*s1.ip_addr_low < *s2.ip_addr_low){
					return true;
				}
				else{
					return false;
				}
			}

			if ((*s1.ip_addr_low < *s2.ip_addr_low)
				||(*s1.ip_addr_low == *s2.ip_addr_low
					&& *s1.ip_addr_high < *s2.ip_addr_high)){
				return true;
			}
			else{
				return false;
			}
	    	}
        };
};


////////////////////////////////////////
/////IMS_SIP_ERROR_TREAT_MAP_KEY////////
////////////////////////////////////////

struct IMS_SIP_ERROR_TREAT_MAP_KEY :public IMSDC_KEY_BASE {
        USHORT table_id;
        UINT       internal_code;

	inline void dump(ostream &os = cout)
	{
                os << "IMS_SIP_ERROR_TREAT_MAP_KEY:: table ID:" << table_id
                   << "  internal code:" << internal_code << endl;
        }

        inline static bool build_key(IMS_SIP_ERROR_TREAT_MAP_KEY &key,
				 USHORT table_id, UINT internal_code )
	{
                key.table_id = table_id;
                key.internal_code = internal_code;
                return true;
        }

        struct key_compare {
                bool operator()(IMS_SIP_ERROR_TREAT_MAP_KEY k1, IMS_SIP_ERROR_TREAT_MAP_KEY k2) const
        	{
			if ( k1.table_id < k2.table_id )
			{
                        	return true;
                	}
			else if( k1.table_id > k2.table_id )
			{
                        	return false;
                	}
			else
			{
                        	if ( k1.internal_code < k2.internal_code )
				{
                                	return true;
                        	}
				else
				{
                                	return false;
                        	}
                	}
		}
        };
};

////////////////////////////////////////
/////IMS_RX_AAR_RETRY_MAP_KEY////////
////////////////////////////////////////

struct IMS_RX_AAR_RETRY_MAP_KEY :public IMSDC_KEY_BASE {
        USHORT table_id;
        ULONG       error_code;
        ULONG       vendor_id;

	inline void dump(ostream &os = cout)
	{
                os << "IMS_RX_AAR_RETRY_MAP_KEY:: table ID:" << table_id
                   << "  internal code:" << error_code 
                   << "  vendor_id:" << vendor_id <<endl;
        }

        inline static bool build_key(IMS_RX_AAR_RETRY_MAP_KEY &key,
				 USHORT table_id, ULONG error_code, ULONG vendor_id)
	{
                key.table_id = table_id;
                key.error_code = error_code;
                key.vendor_id = vendor_id;
                return true;
        }

        struct key_compare {
                bool operator()(IMS_RX_AAR_RETRY_MAP_KEY k1, IMS_RX_AAR_RETRY_MAP_KEY k2) const
        	{
			if ( k1.table_id < k2.table_id )
			{
                        	return true;
                	}
			else if( k1.table_id > k2.table_id )
			{
                        	return false;
                	}
			else
			{
                        	if ( k1.error_code < k2.error_code )
				{
                                	return true;
                        	}
				else if ( k1.error_code > k2.error_code )
				{
                                	return false;
                        	}
                                else
                                {
                                       if(k1.vendor_id < k2.vendor_id )
                                       {
                                                  return true;
                                       }
                                       else
                                       {
                                                  return false;
                                       }
                                }
                	}
		}
        };
};
////////////////////////////////
/////IMS_ICR_KEY////////////////
////////////////////////////////

class IMS_ICR_KEY: public IMSDC_KEY_BASE {

public:
	CPICALLID st_icid;
	CPICALLUNQ uniq;

	inline void dump(ostream &os = cout) {
		os << "IMS_ICR_KEY: icid [" << st_icid << "]" << " uniq [" << uniq
				<< "]" << endl;
	}

	static bool build_key(IMS_ICR_KEY &key, CPICALLID st_icid, CPICALLUNQ uniq) {
		key.uniq = uniq;
		key.st_icid = st_icid;

		return true;
	}

	struct key_compare {
		bool operator()(const IMS_ICR_KEY &key1, const IMS_ICR_KEY &key2) const {

			if (key1.st_icid < key2.st_icid) {
				return true;
			}
			else if (key1.st_icid > key2.st_icid) {
				return false;
			}

			if (key1.uniq < key2.uniq) {
				return true;
			}

			return false;
		}
	};
};

////////////////////////////////////////
/////IMSDC_HASH_STRING_OSEP_MAP_KEY//
////////////////////////////////////////

struct IMSDC_HASH_STRING_OSEP_MAP_KEY: public IMSDC_KEY_BASE
{
        UINT		hash_val;
        const char	*str;
        USHORT		str_len;
	OSENDPOINT	*osep_ptr;

        inline void dump(ostream &os = cout)
	{
                os << "IMSDC_HASH_STRING_OSEP_MAP_KEY: hash_val [" << hash_val << "]"
                   << " str [" << str << "]" << " str_len ["
                   << str_len << "]" << endl;
        }

        static bool build_key(IMSDC_HASH_STRING_OSEP_MAP_KEY &key,
                        const char *str, int len, OSENDPOINT *osep_ptr)
	{
		key.hash_val = 0;
                key.str = NULL;
                key.str_len = 0;
		key.osep_ptr = NULL;
                if (IS_VALID_PTR(str)== FALSE)
		{
                        ASRT_RPT(ASRTBADPARAM, 0,
				"Invalid string [%p] with len %d.", str, len);

                        return false;
                }

                if(len <= 0)
		{
                        len = strlen(str);
                }

                key.hash_val = hash(str, len);
                key.str = str;
                key.str_len = len;

		key.osep_ptr = osep_ptr;

                return true;
        }

        struct key_compare
	{
                bool operator()(const IMSDC_HASH_STRING_OSEP_MAP_KEY &s1,
                        const IMSDC_HASH_STRING_OSEP_MAP_KEY &s2) const
		{
                        if (s1.hash_val < s2.hash_val)
			{
                                return true;
                        }
			
			if (s1.hash_val > s2.hash_val)
			{
                                return false;
                        }

                        if (s1.str_len < s2.str_len)
                        {
                                return true;
                        }

                        if (s1.str_len > s2.str_len)
                        {
                                return false;
                        }

			if (s1.osep_ptr != s2.osep_ptr)
			{
				OSIPPORT port1, port2;
				OSIPADDR *ipaddr1_ptr, *ipaddr2_ptr;

				if (IS_VALID_PTR(s1.osep_ptr) == FALSE)
				{
					return true;
				}

				if (IS_VALID_PTR(s2.osep_ptr) == FALSE)
                                {
                                        return false;
                                }

                                port1 = OSEP_PORTIPUDP(s1.osep_ptr);
                                port2 = OSEP_PORTIPUDP(s2.osep_ptr);

				if (port1 < port2)
				{
					return true;
				}

				if (port1 > port2)
				{
					return false;
				}

				ipaddr1_ptr = OSEP_IPADDR(*s1.osep_ptr);
				ipaddr2_ptr = OSEP_IPADDR(*s2.osep_ptr);

				if (IS_VALID_PTR(ipaddr1_ptr) == FALSE)
				{
					return true;
				}

				if (IS_VALID_PTR(ipaddr2_ptr) == FALSE)
                                {
                                        return false;
                                }

				if (OSIPADDRGETTYPE(ipaddr1_ptr)
						< OSIPADDRGETTYPE(ipaddr2_ptr))
				{
					return true;
				}

                                if (OSIPADDRGETTYPE(ipaddr1_ptr)
						> OSIPADDRGETTYPE(ipaddr2_ptr))
                                {
                                        return false;
                                }

				for (UINT i=0; i<AF_IPADDRSIZE; i++)
				{
					if (ipaddr1_ptr->ipaddr[i] < ipaddr2_ptr->ipaddr[i])
					{
						return true;
					}

                                        if (ipaddr1_ptr->ipaddr[i] > ipaddr2_ptr->ipaddr[i])
                                        {
                                                return false;
                                        }
				}
			}

                	return strncmp(s1.str, s2.str, s1.str_len) < 0;
                }
        };
};

////////////////////////////////
/////IMSDC_TRUNK_GROUP_MAP_KEY//
////////////////////////////////

struct IMSDC_TRUNK_GROUP_MAP_KEY:public IMSDC_KEY_BASE {
	UINT tgrp_hash_val;
	UINT tc_hash_val;
	const char* tgrp;
	USHORT tgrp_len;
	const char* tc;
	USHORT tc_len;
	USHORT tg_table_id;
	UCHAR tg_type;

	inline void dump(ostream &os = cout) {
		os << "IMS_HASH_STRING_MAP_KEY::tgrp_hash_val [" << tgrp_hash_val << "]"
		   << " tc_hash_val [" << tc_hash_val << "]" << endl
		   << " tgrp [" << tgrp << "]" << " tgrp_len [" << dec << tgrp_len << "]"
		   << " tc [" << tc << "]" << " tc_len [" << dec << tc_len << "]" << endl
		   << "tg_table_id [" << dec << tg_table_id << "]" << "tg_type [" << dec
		   << tg_type << "]" << endl;
	}

	static bool build_key(IMSDC_TRUNK_GROUP_MAP_KEY &key,
			const char *tgrp_str, int tgrp_len, const char *tc_str, int tc_len,
			USHORT table_id, UCHAR trunk_group_type) {

		key.tgrp_hash_val = 0;
		key.tc_hash_val = 0;
		key.tgrp = NULL;
		key.tgrp_len = 0;
		key.tc = NULL;
		key.tc_len = 0;
		key.tg_table_id = 0;
		key.tg_type = 0;
		if ((IS_VALID_PTR(tgrp_str) == FALSE) ||
		    (IS_VALID_PTR(tc_str) == FALSE)) {
			ASRT_RPT(ASRTBADPARAM, 0,
				 "Invalid tgrp [%p] with len %d or trunk-context [%p] with len %d.",
				 tgrp_str, tgrp_len, tc_str, tc_len);
			return false;
		}
		if (tgrp_len <= 0) {
			tgrp_len = strlen(tgrp_str);
		}
		if (tc_len <= 0) {
			tc_len = strlen(tc_str);
		}
		key.tgrp_hash_val = hash(tgrp_str, tgrp_len);
		key.tgrp = tgrp_str;
		key.tgrp_len = tgrp_len;

		key.tc_hash_val = hash(tc_str, tc_len);
		key.tc = tc_str;
		key.tc_len = tc_len;

		key.tg_table_id = table_id;
		key.tg_type = trunk_group_type;
		return true;
	}

	// map key comparision, to locate the position of
	// pair <key, element> inside of the map.
	struct key_compare {
		bool operator() (const IMSDC_TRUNK_GROUP_MAP_KEY &s1, 
			const IMSDC_TRUNK_GROUP_MAP_KEY &s2) const {

			if (s1.tg_table_id < s2.tg_table_id) {
				return true;
			} else if (s1.tg_table_id > s2.tg_table_id) {
				return false; 
			} else {
				if (s1.tg_type < s2.tg_type) {
					return true;
				} else if (s1.tg_type > s2.tg_type) {
					return false; 
				} else {
					if (s1.tgrp_hash_val < s2.tgrp_hash_val) {
						return true; 
					} else if (s1.tgrp_hash_val > s2.tgrp_hash_val) {
						return false; 
					} else {
						if (s1.tgrp_len < s2.tgrp_len) {
							return true; 
						} else if (s1.tgrp_len > s2.tgrp_len) {
							return false; 
						} else {
							if (s1.tc_hash_val < s2.tc_hash_val) {
								return true; 
							} else if (s1.tc_hash_val > s2.tc_hash_val) {
								return false; 
							} else {
								if (s1.tc_len < s2.tc_len) {
									return true; 
								} else if (s1.tc_len > s2.tc_len) {
									return false; 
								} else {
									if (strncmp(s1.tgrp, s2.tgrp, s1.tgrp_len) < 0) {
										return true;
									} else if (strncmp(s1.tgrp, s2.tgrp, s1.tgrp_len) > 0) {
										return false; 
									} else {
										if (strncmp(s1.tc, s2.tc, s1.tc_len) < 0) {
											return true;
										} else {
											return false;
										}
									}
								}
							}
						}
					}
				}
			}
		}
	};
};

template<class T>
class range_index{
public:
    T index_start;
    T index_end;

    range_index() : index_start(0), index_end(0)
    {
        //
    }
    range_index(T start, T end) : index_start(start), index_end(end)
    {
        //
    }
    range_index(const range_index &obj)
    {
        this->index_start = obj.index_start;
        this->index_end   = obj.index_end;
    }

    range_index operator=(const range_index &obj)
    {
        this->index_start = obj.index_start;
        this->index_end   = obj.index_end;
        return *this;
    }
};



template <class T>
bool
operator<(const range_index<T> &lhs, const range_index<T> &obj)  //重载 <
{
    if ((lhs.index_start == obj.index_start) && (lhs.index_end == obj.index_end))
    {
        return (false);
    }

    // when left is a pointer, and the right is also a pointer
    if ((lhs.index_start == lhs.index_end) && (obj.index_start == obj.index_end))
    {
        if (lhs.index_start < obj.index_start)
        {
            return (true);
        }

        return (false);
    }

    // when left is a pointer, and the right is line
    if ((lhs.index_start == lhs.index_end) && (obj.index_start != obj.index_end))
    {
        // when the left pointer is in left of the right line
        if (lhs.index_end < obj.index_start)
        {
            return (true);
        }
        
        return (false);
    }

    // when left is a line, and the right is pointer
    if ((lhs.index_start != lhs.index_end) && (obj.index_start == obj.index_end))
    {
        // when the end index of left line is less than the right pointer start_index
        if (lhs.index_end < obj.index_start)
        {
            return (true);
        }
        
        return (false);
    }

    // when left is a line, and the right is also a line
    if ((lhs.index_start != lhs.index_end) && (obj.index_start == obj.index_end))
    {
        // when the start index of left line is less than the right pointer start_index
        if (lhs.index_start < obj.index_start)
        {
            return (true);
        }

        // when the start of the both are equal, compare the end index
        if (lhs.index_start == obj.index_start)
        {
            if (lhs.index_end < obj.index_end)
            {
                return (true);
            }
        }
 
        return (false);
    }

    return (false);
}

template <class T>
bool
operator>(const range_index<T> &lhs, const range_index<T> &obj)  //重载 >
{
    if ((lhs.index_start == obj.index_start) && (lhs.index_end == obj.index_end))
    {
        return (false);
    }

    // when left is a pointer, and the right is also a pointer
    if ((lhs.index_start == lhs.index_end) && (obj.index_start == obj.index_end))
    {
        if (lhs.index_start > obj.index_start)
        {
            return (true);
        }

        return (false);
    }

    // when left is a pointer, and the right is line
    if ((lhs.index_start == lhs.index_end) && (obj.index_start != obj.index_end))
    {
        // when the left pointer is in left of the right line
        if (lhs.index_start > obj.index_end)
        {
            return (true);
        }
        
        return (false);
    }

    // when left is a line, and the right is pointer
    if ((lhs.index_start != lhs.index_end) && (obj.index_start == obj.index_end))
    {
        // when the end index of left line is less than the right pointer start_index
        if (lhs.index_start > obj.index_end)
        {
            return (true);
        }
        
        return (false);
    }

    // when left is a line, and the right is also a line
    if ((lhs.index_start != lhs.index_end) && (obj.index_start == obj.index_end))
    {
        // when the start index of left line is less than the right pointer start_index
        if (lhs.index_start > obj.index_start)
        {
            return (true);
        }

        // when the start of the both are equal, compare the end index
        if (lhs.index_start == obj.index_start)
        {
            if (lhs.index_end > obj.index_end)
            {
                return (true);
            }
        }
 
        return (false);
    }

    return (false);
}

struct IMS_ACL_RANGE_KEY: public IMSDC_KEY_BASE
{
	USHORT acl_table_id;
	DM_V4V6IPADDRESS *ip_addr_low;
	DM_V4V6IPADDRESS *ip_addr_high;
	USHORT port_low;
	USHORT port_high;
	USHORT vlan_id;

	inline void dump(ostream &os = cout)
	{
		os << "IMS_ACL_RANGE_KEY:: acl_table_id [" << dec << acl_table_id << "] " << endl
		   << "ip_addr_low [" << hex << *ip_addr_low << "] "
		   << "ip_addr_high [" << hex << *ip_addr_high << "]" << endl
		   << "port_low [" << dec << port_low << "] "
		   << "port_high " << dec << port_high << endl
		   << "vlan_id [" << dec << vlan_id << "] " << endl;
	}

	IMS_ACL_RANGE_KEY()
	{
		acl_table_id = 0;
		ip_addr_low = NULL;
		ip_addr_high = NULL;
		port_low = 0;
		port_high = 0;
		vlan_id = 0;
	}

	IMS_ACL_RANGE_KEY(const IMS_ACL_RANGE_KEY &k)
	{
		acl_table_id = k.acl_table_id;
		ip_addr_low = k.ip_addr_low;
		ip_addr_high = k.ip_addr_high;
		port_low = k.port_low;
		port_high = k.port_high;
		vlan_id = k.vlan_id;
	}

	inline static bool build_key(IMS_ACL_RANGE_KEY &key,
			USHORT acl_table_id, 
			DM_V4V6IPADDRESS& ip_addr_low, DM_V4V6IPADDRESS& ip_addr_high,
			USHORT port_low, USHORT port_high, USHORT vlan_id=0)
	{
		key.acl_table_id = acl_table_id;
		key.ip_addr_low = &ip_addr_low;
		key.ip_addr_high = &ip_addr_high;
		key.port_low = port_low;
		key.port_high = port_high;
		key.vlan_id = vlan_id;
		return true;
  }

  struct key_compare
  {
	  // the compare will not invoke data validation
      bool
      operator()(IMS_ACL_RANGE_KEY s1, IMS_ACL_RANGE_KEY s2) const
      {
          IMS_ACL_RANGE_KEY s11(s1);
          IMS_ACL_RANGE_KEY s22(s2);
          // compare the table id
          {
              // compare the table id <
              if (s1.acl_table_id < s2.acl_table_id)
              {
                  return (true);
              }
              // compare the table id >
              if (s1.acl_table_id > s2.acl_table_id)
              {
                  return (false);
              }
          }

          // compare the ip range 
          {
              // zero high indicates it is a pointer
              if (isZero(*s1.ip_addr_high))
              {
                  s11.ip_addr_high = s11.ip_addr_low;
              }

              // zero high indicates it is a pointer
              if (isZero(*s2.ip_addr_high))
              {
                  s22.ip_addr_high = s22.ip_addr_low;
              }

          	// compare the ip range <
              if (range_index<DM_V4V6IPADDRESS>(*s11.ip_addr_low, *s11.ip_addr_high) <
                  range_index<DM_V4V6IPADDRESS>(*s22.ip_addr_low, *s22.ip_addr_high))
              {
                  return (true);
              }

              // compare the ip range >
              if (range_index<DM_V4V6IPADDRESS>(*s11.ip_addr_low, *s11.ip_addr_high) >
                  range_index<DM_V4V6IPADDRESS>(*s22.ip_addr_low, *s22.ip_addr_high))
              {
                  return (true);
              }
          }

          // compare the port
          {
              // zero high indicates it is a pointer
              if (s11.port_high == 0)
              {
                  s11.port_high = s11.port_low;
              }

              // zero high indicates it is a pointer
              if (s22.port_high == 0)
              {
                  s22.port_high = s22.port_low;
              }

              // compare the port range <
              if (range_index<USHORT>(s11.port_low, s11.port_high) <
                  range_index<USHORT>(s22.port_low, s22.port_high))
              {
                  return (true);
              }

              // compare the port range >
              if (range_index<USHORT>(s11.port_low, s11.port_high) >
                  range_index<USHORT>(s22.port_low, s22.port_high))
              {
                  return (true);
              }
          }

          // compare the vlan_id
          {
              if (s1.vlan_id < s2.vlan_id)
              {
                  return true;
              }

              if (s1.vlan_id > s2.vlan_id)
              {
                  return false;
              }
          }
      }
  };
};

////////////////////////////////
/////IMS_CORE_FED_PORT_MAP_KEY//
////////////////////////////////

struct IMS_CORE_FED_PORT_MAP_KEY:public IMSDC_KEY_BASE {
	UINT appltype_has_val;
	UINT applgrp_has_val;
	const char* appltype;
	USHORT appltype_len;
	const char* applgrp;
	USHORT applgrp_len;
	USHORT core_fed_id;

	inline void dump(ostream &os = cout) {
		os << "IMS_HASH_STRING_MAP_KEY::appltype_has_val [" << appltype_has_val << "]"
		   << " applgrp_has_val [" << applgrp_has_val << "]" << endl
		   << " appltype [" << appltype << "]" << " appltype_len [" << dec << appltype_len << "]"
		   << " applgrp [" << applgrp << "]" << " applgrp_len [" << dec << applgrp_len << "]" << endl
		   << "core_fed_id [" << dec << core_fed_id << "]" << endl;
	}

	static bool build_key(IMS_CORE_FED_PORT_MAP_KEY &key,
			const char *appltype_str, int appltype_len, const char *applgrp_str, int applgrp_len,
			USHORT core_fed_id) {

		key.appltype_has_val = 0;
		key.applgrp_has_val = 0;
		key.appltype = NULL;
		key.appltype_len = 0;
		key.applgrp = NULL;
		key.applgrp_len = 0;
		key.core_fed_id = 0;
		if ((IS_VALID_PTR(appltype_str) == FALSE) ||
		    (IS_VALID_PTR(applgrp_str) == FALSE)) {
			ASRT_RPT(ASRTBADPARAM, 0,
				 "Invalid appltype [%p] with len %d or applgrp [%p] with len %d.",
				 appltype_str, appltype_len, applgrp_str, applgrp_len);
			return false;
		}
		if (appltype_len <= 0) {
			appltype_len = strlen(appltype_str);
		}
		if (applgrp_len <= 0) {
			applgrp_len = strlen(applgrp_str);
		}
		key.appltype_has_val = hash(appltype_str, appltype_len);
		key.appltype = appltype_str;
		key.appltype_len = appltype_len;

		key.applgrp_has_val = hash(applgrp_str, applgrp_len);
		key.applgrp = applgrp_str;
		key.applgrp_len = applgrp_len;

		key.core_fed_id = core_fed_id;
		return true;
	}

	// map key comparision, to locate the position of
	// pair <key, element> inside of the map.
	struct key_compare {
		bool operator() (const IMS_CORE_FED_PORT_MAP_KEY &s1, 
			const IMS_CORE_FED_PORT_MAP_KEY &s2) const {

			if (s1.core_fed_id < s2.core_fed_id) {
				return true;
			} else if (s1.core_fed_id > s2.core_fed_id) {
				return false; 
			} 

			if (s1.appltype_has_val < s2.appltype_has_val) {
				return true; 
			} else if (s1.appltype_has_val > s2.appltype_has_val) {
				return false; 
			} 

			if (s1.appltype_len < s2.appltype_len) {
				return true; 
			} else if (s1.appltype_len > s2.appltype_len) {
				return false; 
			} 

			if (s1.applgrp_has_val < s2.applgrp_has_val) {
				return true; 
			} else if (s1.applgrp_has_val > s2.applgrp_has_val) {
				return false; 
			} 

			if (s1.applgrp_len < s2.applgrp_len) {
				return true; 
			} else if (s1.applgrp_len > s2.applgrp_len) {
				return false; 
			} 

			int retval = strncmp(s1.appltype, s2.appltype, s1.appltype_len);
			if  (retval < 0) {
				return true;
			} else if (retval > 0) {
				return false; 
			} 

			if (strncmp(s1.applgrp, s2.applgrp, s1.applgrp_len) < 0) {
				return true;
			} 

			return false;
		}
	};
};


/////////////////////////////////////////
/////IMS_PCSCF_ROUTING_REGISTER_MAP_KEY//
/////////////////////////////////////////

struct IMS_PCSCF_ROUTING_REGISTER_MAP_KEY:public IMSDC_KEY_BASE
{
        USHORT pcscf_map_key;

        inline void dump(ostream &os = cout)
        {
                os << "IMS_PCSCF_ROUTING_REGISTER_MAP_KEY::pcscf_map_key [" << pcscf_map_key << "]"<< endl;
        }

        static bool build_key(IMS_PCSCF_ROUTING_REGISTER_MAP_KEY &key, USHORT p_input)
        {
                key.pcscf_map_key = p_input;
                return true;
        }

        struct key_compare
        {
                bool operator() (const IMS_PCSCF_ROUTING_REGISTER_MAP_KEY &s1,
                        const IMS_PCSCF_ROUTING_REGISTER_MAP_KEY &s2) const
                {
                        if (s1.pcscf_map_key < s2.pcscf_map_key) {
                                return true;
                        }

                        return false;
                }
        };
};

//////////////////////////////////////////////////
/////IMS_RO_AP_DIAM_RESULT_CODES_MAP_KEY//////////
//////////////////////////////////////////////////

struct IMS_RO_AP_DIAM_RESULT_CODES_MAP_KEY :public IMSDC_KEY_BASE {
	USHORT	result_code;

	inline void dump(ostream &os = cout)
	{
                os << "IMS_RO_AP_DIAM_RESULT_CODES_MAP_KEY:: result_code:"<< result_code << endl;
        }

        inline static bool build_key(IMS_RO_AP_DIAM_RESULT_CODES_MAP_KEY &key,
					USHORT result_code)
	{
                key.result_code = result_code;
                return true;
        }

        struct key_compare {
		bool operator()(IMS_RO_AP_DIAM_RESULT_CODES_MAP_KEY k1, IMS_RO_AP_DIAM_RESULT_CODES_MAP_KEY  k2) const
        	{
			if ( k1.result_code < k2.result_code)
			{
				return true;
			}
			else
			{
                        	return false;
                	}
		}
        };
};

////////////////////////////////////////
/////SMD_PLSG_MAP_KEY////////
////////////////////////////////////////

struct SMD_PLSG_MAP_KEY :public IMSDC_KEY_BASE {
        USHORT group_id;
        USHORT sip_pls_id;

	inline void dump(ostream &os = cout)
	{
                os << "SMD_PLSG_MAP_KEY:: group ID:" << group_id
                   << "  sip pls id code:" << sip_pls_id << endl;
        }

        inline static bool build_key(SMD_PLSG_MAP_KEY &key,
				 USHORT group_id, USHORT sip_pls_id)
	{
                key.group_id = group_id;
                key.sip_pls_id = sip_pls_id;
                return true;
        }

        struct key_compare {
                bool operator()(SMD_PLSG_MAP_KEY k1, SMD_PLSG_MAP_KEY k2) const
        	{
			if ( k1.group_id < k2.group_id )
			{
                        	return true;
                	}
			else if( k1.group_id > k2.group_id )
			{
                        	return false;
                	}
			else
			{
                        	if ( k1.sip_pls_id < k2.sip_pls_id )
				{
                                	return true;
                        	}
				else
				{
                                	return false;
                        	}
                	}
		}
        };
};

template<typename T_KEY, typename T_VAL>
class IMSDC_MAP : public IMSDC_MAP_BASE<T_VAL>
{
public:
	typedef map<T_KEY, T_VAL, typename T_KEY::key_compare> IMS_DC_MAP_TYPE;
	typedef typename IMS_DC_MAP_TYPE::iterator IMS_DC_MAP_ITER;

	IMSDC_MAP<T_KEY, T_VAL>(){
		OSMUTEX_INIT(&map_mutex, NULL);
	}

	inline void insert(IMSDC_KEY_BASE &t, T_VAL index){
		IMSdcMutexGuard guard(map_mutex);
		T_KEY *key = (T_KEY*)&t;
		maps.insert(pair<T_KEY, T_VAL> (*key, index));
	}

	inline bool empty(){
		IMSdcMutexGuard guard(map_mutex);
		return maps.empty();
	}
	inline void clear(){
		IMSdcMutexGuard guard(map_mutex);
		if (!maps.empty()) {
			maps.clear();
		}
	}

	inline void erase(IMSDC_KEY_BASE &t, BOOL needReport = TRUE){
		IMSdcMutexGuard guard(map_mutex);
		T_KEY* key = (T_KEY*)&t;
		IMS_DC_MAP_ITER it = maps.find(*key);
		if (it != maps.end()) {
			maps.erase(it);
		} else {
			if (needReport == TRUE)
			{
				IMS_DLOG( IMS_LOGHIGH, "Try to remove non-exist entries in maps");
			}
		}
	}

        inline void erase(IMS_DC_MAP_ITER it){
                IMSdcMutexGuard guard(map_mutex);
                if (it != maps.end()) {
                        maps.erase(it);
                }
        }

	inline bool find(IMSDC_KEY_BASE &t, T_VAL &val){
		IMSdcMutexGuard guard(map_mutex);
		T_KEY *key = (T_KEY*)&t;
		IMS_DC_MAP_ITER it = maps.find(*key);
			if (it != maps.end()) {
				val = (T_VAL) it->second;
				return true;
			} else {
				return false;
		    }
	}

        inline IMS_DC_MAP_ITER find(IMSDC_KEY_BASE &t)
	{
                IMSdcMutexGuard guard(map_mutex);
                T_KEY *key = (T_KEY*)&t;
                return maps.find(*key);
        }

        inline IMS_DC_MAP_ITER begin()
        {
            IMSdcMutexGuard guard(map_mutex);
            return maps.begin();
        }

        inline IMS_DC_MAP_ITER end()
        {
            IMSdcMutexGuard guard(map_mutex);
            return maps.end();
        }

        inline unsigned int size()
	{
                IMSdcMutexGuard guard(map_mutex);
		return maps.size();
        }

        inline IMS_DC_MAP_ITER upper_bound(const IMSDC_KEY_BASE &t)
        {
                IMSdcMutexGuard guard(map_mutex);
                return maps.upper_bound(t);
        }


	inline IMSDC_KEY_BASE *allocateKey(){
		return new T_KEY();
	}

	virtual void freeKey(IMSDC_KEY_BASE *key){
		if(IS_VALID_PTR(key)){
			delete key;
		}
	}

	inline void dump_all(ostream &os = cout){
		IMSdcMutexGuard guard(map_mutex);

		IMS_DC_MAP_ITER it;
		 for (it = maps.begin(); it != maps.end(); it++) {
			 dump(it, os);
		 }
	}

	inline void dump_with_key(IMSDC_KEY_BASE &t, ostream &os = cout){
		IMSdcMutexGuard guard(map_mutex);

		T_KEY *key = (T_KEY*)&t;

		IMS_DC_MAP_ITER it = maps.find(*key);

		if( it != maps.end()) {
			dump(it, os);
		}
		else{
			os<<"Can not find entry in maps with key: "<<endl;
			key->dump(os);
			os<<endl;
		}
	}

private:
	inline void dump( IMS_DC_MAP_ITER &it, ostream &os = cout){
		if(it != maps.end()){
			T_KEY key = (T_KEY) it->first;
			T_VAL idx = (T_VAL) it->second;
			key.dump(os);
			os << "value is " << idx << endl;
			os << endl;
		}
		else{
			os<<"Reaches the end of the map. Nothing is dumped."<<endl;
		}
	}

	IMS_DC_MAP_TYPE maps;
	OS_MUTEX_T map_mutex;
};

#endif /* IMSDC_MAPS_H_ */

