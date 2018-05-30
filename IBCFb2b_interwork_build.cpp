/*                                                                           
  *  Copyright (c) 2000-2016 NOKIA Technologies.                              
  *  All rights reserved.                                                     
  *                                                                           
  *  ALL RIGHTS ARE RESERVED BY NOKIA TECHNOLOGIES.                           
  *  ACCESS TO THIS SOURCE CODE IS STRICTLY RESTRICTED UNDER                  
  *  CONTRACT. THIS CODE IS TO BE KEPT STRICTLY CONFIDENTIAL                  
  *  UNAUTHORIZED MODIFICATIONS OF THIS FILE WILL VOID YOUR                   
  *  SUPPORT CONTRACT WITH NOKIA TECHNOLOGIES.  IF SUCH                       
  *  MODIFICATIONS ARE FOR THE PURPOSE OF CIRCUMVENTING                       
  *  LICENSING LIMITATIONS, LEGAL ACTION MAY RESULT.                          
  */                                                                          
                                                                              
 /*                                                                           
  *                                                                           
  * File:        ssp/ds/ims/ibcf/IBCFb2b_interwork_build.cpp                  
  *                                                                           
  * Description: This source file contains build/update the sip_ia_msg from MGC format parameters
  *               related functions for B2B IBCF SIPT feature.                
  *                                                                           
  */ 
#include "IBCF_interwork.h"

char *hist_info_text[] = {"Unknown", "Unconditional", "User busy", "No reply",
							  "Deflection immediate response", "Deflection during alerting",
							  "Mobile subscriber not reachable"};


char *sipaPEMParamTxt[] = {NULL, "sendrecv", "sendonly", "recvonly", "inactive", 
					  "gated", "supported", NULL};


UINT
IBCFb2b_iw_build_contact(B2BUAEvent *event_ptr, SipwLiConEvnt *p_SipConEvntPtr)
{
	UNIT16				len = 0;
	UCHAR				contactIncludeCgpn = 0;
	const IBCF_TRUNK_GROUP_DATA 	*tg_data_ptr = NULL_PTR;
	SIP_Header			*hc_ptr = NULL_PTR;
	Sip_URI				*suri_ptr = NULL_PTR;
	SIPia_List_Cursor		*cursor = NULL;


	if (IS_VALID_PTR(p_SipConEvntPtr) == FALSE)
	{
		IMS_DLOG(IMS_IBCF_LOGHIGH, "%s(): null input p_SipConEvntPtr.", __FUNCTION__);
		return (0);
	}

	if (IS_VALID_PTR(event_ptr) == FALSE ||
	    IS_VALID_PTR(event_ptr->m_ptr) == FALSE)
	{
		IMS_DLOG(IMS_IBCF_LOGHIGH, "%s(): invalid input event_ptr=%p.", 
			__FUNCTION__, event_ptr);
		return (0);
	}

	/* Get control flag */
	tg_data_ptr = IBCF_get_tg_data_by_sip_msg(event_ptr->m_ptr);
	if (IS_VALID_PTR(tg_data_ptr) == FALSE)
	{
		IMS_DLOG(IMS_IBCF_LOGHIGH, "%s(), Could not find trunk group data for sip_msg_ptr=[%p].",
			 __FUNCTION__, event_ptr->m_ptr);
		return (0);
	}

	contactIncludeCgpn = IMSdc_get_ibcf_sip_sipi_interworking_profile_contactincludecgpn(tg_data_ptr->sipi_profile_id);
	if (contactIncludeCgpn == FALSE)
	{
		IMS_DLOG(IMS_IBCF_LOGLOW, "%s(): contactIncludeCgpn flag is off.", __FUNCTION__);
		return (0);
	}

	if (p_SipConEvntPtr->fromHdr.fromVal.pres == NOTPRSNT) 
	{
		IMS_DLOG(IMS_IBCF_LOGLOW, "%s(): no available cgpn in fromHdr.", __FUNCTION__);
		return (0);
	}

	if (p_SipConEvntPtr->fromHdr.fromVal.len < 0 ||
	    IS_VALID_PTR(p_SipConEvntPtr->fromHdr.fromVal.val) == FALSE)
	{
		IMS_DLOG(IMS_IBCF_LOGHIGH, "%s(): invalid p_SipConEvntPtr->fromHdr, len=%u.",
			 __FUNCTION__, p_SipConEvntPtr->fromHdr.fromVal.len);
		return (0);
	}

        for (len=0; len < p_SipConEvntPtr->fromHdr.fromVal.len; len++)
        {
                if ( (p_SipConEvntPtr->fromHdr.fromVal.val[len] == SIP_ASCII_SEMICOLON) ||
                     (p_SipConEvntPtr->fromHdr.fromVal.val[len] == SIP_ASCII_SPACE) ||
                     (p_SipConEvntPtr->fromHdr.fromVal.val[len] == SIP_ASCII_AT) )
        	{
                	break;
		}
        }
        
	if (len <= 1)
	{
		IMS_DLOG(IMS_IBCF_LOGLOW, "%s(): useless cgpn in fromHdr.", __FUNCTION__);
		return (0);
	}

	SIPia_List *listMgr = reinterpret_cast<SIPia_List *>(SIPia_getContactList(event_ptr->m_ptr));
	if (IS_VALID_PTR(listMgr) == FALSE)
	{
		IMS_DLOG(IMS_IBCF_LOGLOW, "%s(), Invalid listMgr,no Contact header to decrypt", __FUNCTION__);
		return (0);
	}

	SIPia_List_cursor_init(listMgr, &cursor);
	while (cursor != NULL)
	{
		hc_ptr = reinterpret_cast<SIP_Header *>(SIPia_List_traverse(&cursor));
		if (IS_VALID_PTR(hc_ptr) == FALSE)
		{
			IMS_DLOG(IMS_IBCF_LOGHIGH, "%s(), Invalid SIP Header", __FUNCTION__);
			continue;
		}

		suri_ptr = SIPia_Contact_getSipUri(event_ptr->m_ptr, hc_ptr);
		if (IS_VALID_PTR(suri_ptr) == FALSE)
		{
			IMS_DLOG(IMS_IBCF_LOGLOW, "%s(): failed to get SIP URI from contact.", __FUNCTION__);
			break;
		}

		/* Update User Name of contact part to include cgpn. */
		DString_copy(event_ptr->m_ptr, 
			     Sip_URI_getUserName(suri_ptr),
			     reinterpret_cast<const char*>(p_SipConEvntPtr->fromHdr.fromVal.val),
			     len);
		SIPia_InvalidateRawText(event_ptr->m_ptr, SIPIA_CONTACT);
		return (len);
	}

	return (0);
}

template <class Type>
Type
stringConvert(const std::string &str)
{
    istringstream iss(str);
    Type          num;
    iss >> num;
    return num;
}

/* Build SIP Max-Forwards header from Hop Counter parameter in ISUP IAM based on
 * "maxFwdhopCntrRatio"
 * option settings in interworking profile associated with trunk group
 */
IMS_RETVAL
IBCFb2b_iw_build_maxforward(B2BUAEvent *event_ptr, SipwLiConEvnt *p_SipConEvntPtr)
{
    if (!IS_VALID_PTR(event_ptr) || !IS_VALID_PTR(event_ptr->m_ptr) ||
        !IS_VALID_PTR(p_SipConEvntPtr))
    {
        IMS_DLOG(IMS_IBCF_LOGHIGH, "%s: invalid input, event_ptr(%p), p_SipConEvntPtr(%p)",
                 __func__, event_ptr, p_SipConEvntPtr);
        return (IMS_FAIL);
    }

    SIPia_Msg *msg_ptr = event_ptr->m_ptr;

    /* Add Max forwards (make sure it is withing range (1-255): rfc3261) */
    /* if MaxForwards is not present in ISUP, set MaxForwards to default value: 70 */
    if (!p_SipConEvntPtr->maxForwards.pres)
    {
        SIPia_setMaxForwards(msg_ptr, 70);
        return (IMS_SUCCESS);
    }

    SIPia_setMaxForwards(msg_ptr, p_SipConEvntPtr->maxForwards.val);

    return (IMS_SUCCESS);
}

/* Function:      IBCFb2b_iw_build_charging_hdr
 *
 * Descriptions:  The function is used to build P-Charge-Info Header or P-DCS-Billing-Info Header
 *                according to the configuration of pcharging defined in SIPI profile:
 *
 *                NONE(0): neither P-DCS-Billing-Info header nor P-Charge-Info header will be
 *                         transmitted.
 *                PCHARGEINFO(1): P-Charge-Info header will be transmitted if available.
 *                PDCSBILLING(2): P-DCS-Billing-Info header will be transmitted if available.
 *
 *
 * Inputs:        p_SipConEvntPtr  - SipwLiConEvnt pointer store paidrn string
 *                msg_ptr       - output sip message
 *
 * Return:        IMS_SUCCESS/IMS_FAIL
 */
IMS_RETVAL
IBCFb2b_iw_build_charging_hdr(B2BUAEvent *event_ptr, SipwLiConEvnt *p_SipConEvntPtr)
{
    /* bug88730, change the logic to handle dependency between p-charge-info and p-dcs-billing-info
     * headers as following:
     * if either pChargeInfoHdr or pdcsBilling flag is Yes in og trkgrp, populate the supported
     * header out
     * if both flags are Yes, populate only one header based on ogChrgNum setting (PDCS/PCHRG)
     */
    B2BUALegData *               leg_ptr            = NULL_PTR;
    B2BUALegData *               mate_ptr           = NULL_PTR;
    SIPia_Msg *                  msg_ptr            = NULL_PTR;
    const IBCF_TRUNK_GROUP_DATA *tg_data_ptr        = NULL_PTR;
    IBCF_SIPI_PROFILE_DATA *     sipi_prof_data_ptr = NULL_PTR;
    const char *                 home_domain_ptr    = NULL_PTR;
    DMUSHORT                     id                 = 0;
    DMUSHORT                     variant            = 0;
    unsigned short               addrLen            = 0;
    std::string                  later_use_string;
    IBCF_VN_TG_ID                tg_info;
    IMS_RETVAL                   ret_val = IMS_SUCCESS;

    if (!IS_VALID_PTR(event_ptr) || !IS_VALID_PTR(event_ptr->m_ptr) ||
        !IS_VALID_PTR(event_ptr->calldata_ptr) || !IS_VALID_PTR(p_SipConEvntPtr))
    {
        IMS_DLOG(IMS_IBCF_LOGHIGH, "%s: invalid input, event_ptr(%p), p_SipConEvntPtr(%p)",
                 __func__, event_ptr, p_SipConEvntPtr);
        return (IMS_FAIL);
    }

    B2BUAEventToLegs(event_ptr, &leg_ptr, &mate_ptr, B2BUA_LOGICAL_MATE);
    if (IS_VALID_PTR(leg_ptr) == FALSE || IS_VALID_PTR(mate_ptr) == FALSE)
    {
        IMS_DLOG(IMS_IBCF_LOGHIGH, "%s: Invalid leg_ptr %p or mate_ptr %p", __func__, leg_ptr,
                 mate_ptr);
        return (IMS_FAIL);
    }

    msg_ptr = event_ptr->m_ptr;

    IBCFb2b_iw_get_sipt_profile_id(event_ptr, id, variant);
    sipi_prof_data_ptr = IBCF_get_sipi_profile_data_by_id(id);
    if (!IS_VALID_PTR(sipi_prof_data_ptr))
    {
        IMS_DLOG(IMS_IBCF_LOGLOW, "%s: could not found  sipi_prof_data_ptr ", __func__);
        return (IMS_SUCCESS);
    }

    if (/*sipi_prof_data_ptr->db_data.pcharging == PCHARGING_NONE ||*/
        sipi_prof_data_ptr->db_data.pcharging >= PCHARGING_FLAG_MAX)
    {
        IMS_DLOG(IMS_IBCF_LOGLOW, "%s: will not build P-Charge-Info Header, "
                                  "sipi_prof_data_ptr->db_data.pcharging(%d)",
                 __func__, sipi_prof_data_ptr->db_data.pcharging);
        return (IMS_SUCCESS);
    }

    IBCFget_tg_info_from_leg(leg_ptr, &tg_info);
    tg_data_ptr = IBCF_get_tg_data_by_id(tg_info.vn_id, tg_info.tg_id);
    if (!IS_VALID_PTR(tg_data_ptr))
    {
        IMS_DLOG(IMS_IBCF_LOGHIGH, "%s(), Could not find trunk group data", __func__);
        return (IMS_SUCCESS);
    }

    const char *   fed_ext_name_ptr = NULL_PTR;
    int fed_ext_name_len = 0;
    SIPIA_PORT_IDX sip_port_idx = B2BCsip_get_orig_port_idx(event_ptr->m_ptr);
    if (IS_VALID_SIPIA_PORT_IDX(sip_port_idx) == FALSE)
    {
        IMS_DLOG(IMS_IBCF_LOGHIGH, "%s: invalid sip_port_idx(%d)", __func__, sip_port_idx);
        return (IMS_SUCCESS);
    }

    NGSS_PORT_INDX port_idx         = SSgul_get_sipia_port_prov_key(sip_port_idx);
    if (port_idx == NGSS_DEFAULT_INDX)
    {
        IMS_DLOG(IMS_IBCF_LOGHIGH, "%s(), invalid port_idx=[%d].", __func__, port_idx);

        return IMS_SUCCESS;
    }

    IBCF_VIRTUAL_NETWORK_DATA *vn_data_ptr = IBCF_get_vn_data_by_id(tg_info.vn_id);
    if (IS_VALID_PTR(vn_data_ptr) == FALSE)
    {
        IMS_DLOG(IMS_IBCF_LOGHIGH, "%s(), Couldn't find VN data through SIP messsage.", __func__);
        return IMS_SUCCESS;
    }

    if (vn_data_ptr->vn_type == IBCF_VIRTUAL_NETWORK_CORE)
    {
        fed_ext_name_ptr =
            IMScfed_get_core_fed_port_ext_host_name(port_idx, &fed_ext_name_len, event_ptr->m_ptr);
    }
    else
    {
        fed_ext_name_ptr = IMSpfed_get_peer_fed_port_ext_host_name(port_idx, event_ptr->m_ptr);
    }

    IMS_DLOG(IMS_IBCF_LOGLOWEST, "%s():FED Host Name[%s]", __func__, fed_ext_name_ptr);

    IMS_DLOG(IMS_IBCF_LOGLOW, "%s: entry", __func__);

    /* Calculate the length for localAddr */
    home_domain_ptr = fed_ext_name_ptr;
    if (IS_VALID_PTR(home_domain_ptr) == FALSE || strlen(home_domain_ptr) <= 0)
    {
        IMS_DLOG(IMS_IBCF_LOGHIGH, "%s: could not get home domain", __func__);
        return (IMS_SUCCESS);
    }

    later_use_string.append(home_domain_ptr);

    /*
    else if (ipVersion == AF_INET6)
    {
        later_use_string.append(LOCAL.local_addr_ipv6);
    }
    else
    {
        later_use_string.append(LOCAL.local_addr);
    }
    */

    addrLen = later_use_string.length();

    /* pcharging in sipi profile specifies the SIP header to be transmitted in outgoing messages for
     * charging purpose.
     * NONE(0): neither P-DCS-Billing-Info header nor P-Charge-Info header will be transmitted.
     * PCHARGEINFO(1): P-Charge-Info header will be transmitted if available.
     * PDCSBILLING(2): P-DCS-Billing-Info header will be transmitted if available.
     */
    /* add while circle to break to P-DCS-Billing-Info build ,
     * good solution is to split this to two functions
     */
    while(1)
    {
    if ( (sipi_prof_data_ptr->db_data.pcharging == PCHARGING_PCHARGEINFO) ||
         (sipi_prof_data_ptr->db_data.pcharging == PCHARGING_NONE) )
         //sipi_prof_data_ptr->db_data.pcharging == PCHARGING_BOTH)
    {
        unsigned short  maxLen        = 0;
        SipPChargeInfo *chargeInfoPtr = &(p_SipConEvntPtr->chargeInfo);
        std::string     p_charge_info_string;

        if ((p_SipConEvntPtr->chargeInfo.eh.pres == NOTPRSNT) ||
            (p_SipConEvntPtr->chargeInfo.addrSpec.pres == NOTPRSNT) ||
            (p_SipConEvntPtr->chargeInfo.addrSpec.len <= 0))
        {
            IMS_DLOG(IMS_IBCF_LOGLOW, "%s: will not build P-Charge-Info, "
                                  "chargeInfo.eh.pres(%d), chargeInfo.addrSpec.pres(%d)",
                                  __func__, p_SipConEvntPtr->chargeInfo.eh.pres,
                                  p_SipConEvntPtr->chargeInfo.addrSpec.pres);
            break;
        }

        /* Calculate the max length of P-Charge-Info size */
        maxLen = sizeof(SIP_LAQUOT_SIP_COLON_TEXT) - 1 + 
		         chargeInfoPtr->addrSpec.len           +
                 sizeof(SIP_AT_TEXT) - 1               + 
				 addrLen                               + 
				 sizeof(SIP_RAQUOT_TEXT) - 1           +
                 sizeof(SIP_SEMI_NPI_EQUAL_TEXT) - 1   + 
				 sizeof(SIP_SEMI_NOA_EQUAL_TEXT) - 1   +
                 sizeof(SIP_PLUS_TEXT) - 1             + 
				 SIP_MAX_NOA_STRING_SIZE               + 
				 SIP_MAX_NPI_STRING_SIZE               +
                 MAX_COUNTRY_CODE_LENGTH               + 
				 SIP_RN_LEN; /* FID 17599 */

        if (maxLen >= SIP_MAX_CHARGE_INFO_STRING_SIZE)
        {
            IMS_DLOG(IMS_IBCF_LOGLOW, "%s: P-Charge-Info too big: %d\n", __func__, maxLen);
            break;
        }

        /* append "<sip:"*/
        p_charge_info_string.append(SIP_LAQUOT_SIP_COLON_TEXT);
        p_charge_info_string.append((char *)chargeInfoPtr->addrSpec.val,
                                    (int)chargeInfoPtr->addrSpec.len);

        /* Append ;npi=NpiString;noa=NoaString when npi is not 1 or noa is not 3 */
        if ((chargeInfoPtr->npi.pres == PRSNT_NODEF) && (chargeInfoPtr->npi.val < MAX_NPI) &&
            (chargeInfoPtr->noa.pres == PRSNT_NODEF) && (chargeInfoPtr->noa.val < 127) &&
            ((chargeInfoPtr->npi.val != SIP_PCHG_NPI_ISDN) || (chargeInfoPtr->noa.val != 3)))
        {
            char npinoaStr[16];
            memset(npinoaStr, 0, 16);
            sprintf(npinoaStr, ";npi=%d;noa=%d", chargeInfoPtr->npi.val, chargeInfoPtr->noa.val);
            p_charge_info_string.append(npinoaStr);
        }
        
        /* FID 17599 */
        if ((sipi_prof_data_ptr->db_data.jiprnmap == JIPRNMAP_PCHARGEINFO) &&
            (p_SipConEvntPtr->jurisInf.eh.pres == PRSNT_NODEF))
        {
            char strData[SIP_RN_LEN] = {0};

            sprintf(strData + 0, "%d", p_SipConEvntPtr->jurisInf.addrSig1.val);
            sprintf(strData + 1, "%d", p_SipConEvntPtr->jurisInf.addrSig2.val);
            sprintf(strData + 2, "%d", p_SipConEvntPtr->jurisInf.addrSig3.val);
            sprintf(strData + 3, "%d", p_SipConEvntPtr->jurisInf.addrSig4.val);
            sprintf(strData + 4, "%d", p_SipConEvntPtr->jurisInf.addrSig5.val);
            sprintf(strData + 5, "%d", p_SipConEvntPtr->jurisInf.addrSig6.val);

            IMS_DLOG(IMS_IBCF_LOGLOW, "%s: strData = %s", __func__, strData);
            p_charge_info_string.append(";rn=");
            p_charge_info_string.append(strData, SIP_RN_LEN);
        }

        /* Append @localAddr> */
        p_charge_info_string.append(SIP_AT_TEXT);
        p_charge_info_string.append(later_use_string);
        p_charge_info_string.append(SIP_RAQUOT_TEXT);

        IMS_DLOG(IMS_IBCF_LOGLOW, "%s: P-Charge-Info string:%s \n", __func__,
                 p_charge_info_string.c_str());
        
	SIPia_deleteOtherHeader(msg_ptr, "P-Charge-Info", strlen("P-Charge-Info"));
        SIPia_AddOtherHeader(msg_ptr, "P-Charge-Info", p_charge_info_string.c_str(),
                             sizeof("P-Charge-Info") - 1, p_charge_info_string.length());

        break;
    }
        break;
    }

    if ((sipi_prof_data_ptr->db_data.pcharging == PCHARGING_PDCSBILLING) ||
        (sipi_prof_data_ptr->db_data.pcharging == PCHARGING_NONE) )
        //sipi_prof_data_ptr->db_data.pcharging == PCHARGING_BOTH)
    {
        /* P-DCS-Billing-Info               */
        /************************************/
        char             billInfoId[80];
        unsigned short   len            = 0;
        unsigned short   maxLen         = 0;
        SipPDcsBillInfo *dcsBillInfoPtr = NULL;
        std::string      p_dcs_billing_info_string;

        bzero(billInfoId, sizeof(billInfoId));
        dcsBillInfoPtr = &(p_SipConEvntPtr->pDcsBillInfo);

        if (dcsBillInfoPtr->eh.pres == NOTPRSNT || 
            dcsBillInfoPtr->addrSpec.pres == NOTPRSNT || 
            dcsBillInfoPtr->addrSpec.len <= 0 ||
            strlen(reinterpret_cast<const char *>(dcsBillInfoPtr->addrSpec.val)) == 0)
        {
            IMS_DLOG(IMS_IBCF_LOGLOW, "%s: will not build P-DCS-Billing-Info", __func__);
            return (IMS_SUCCESS);
        }

        IMS_DLOG(IMS_IBCF_LOGLOW, "%s: P-DCS-Billing-Info to be populated", __func__);

        /* construct the BCID and FEID*/
        short ipVersion = 0;
        switch (tg_data_ptr->remote_addr_type)
        {
        case IBCF_TRUNK_GROUP_ADDRESS_TYPE_IPV6:
            ipVersion = AF_INET6;
            break;
        case IBCF_TRUNK_GROUP_ADDRESS_TYPE_IPV4:
        case IBCF_TRUNK_GROUP_ADDRESS_TYPE_FQDN:
        case IBCF_TRUNK_GROUP_ADDRESS_TYPE_INVALID:
            ipVersion = AF_INET;
            break;
        }

        len = IBCFb2b_iw_build_DcsBillingInfoId(msg_ptr, billInfoId, p_SipConEvntPtr->suConnId,
                                                ipVersion, tg_data_ptr->remote_port);

        /* Calculate the max length of P-DCS-Billing-Info size */
        maxLen = len                                    + 
		         sizeof(SIP_AT_TEXT) - 1                + 
		         addrLen                                + 
				 sizeof(SIP_SEMI_CHARGE_EQUAL_TEXT) - 1 +
                 sizeof(SIP_QUETO_SIP_COLON_TEXT) - 1   + 
				 dcsBillInfoPtr->addrSpec.len           +
                 sizeof(SIP_AT_TEXT) - 1                + 
				 addrLen                                + 
				 sizeof(SIP_DOUBLE_QUETO_TEXT) - 1;

        if (maxLen >= SIP_MAX_DCS_BILLING_INFO_STRING_SIZE)
        {
            IMS_DLOG(IMS_IBCF_LOGLOW, "%s: P-DCS-Billing-Info too big:%d", __func__, maxLen);
            return (IMS_SUCCESS);
        }

        /* Append the BCID and FEID */
        p_dcs_billing_info_string.append(billInfoId, len);

        /* Append @localAddr */
        p_dcs_billing_info_string.append(SIP_AT_TEXT);
        p_dcs_billing_info_string.append(later_use_string);

        /* Append ;charge="sip: */
        p_dcs_billing_info_string.append(SIP_SEMI_CHARGE_EQUAL_TEXT);
        p_dcs_billing_info_string.append(SIP_QUETO_SIP_COLON_TEXT);

        /* Append chargeNum */
        p_dcs_billing_info_string.append(reinterpret_cast<char *>(dcsBillInfoPtr->addrSpec.val),
                                         dcsBillInfoPtr->addrSpec.len);

        /* Append @localAddr" */
        p_dcs_billing_info_string.append(SIP_AT_TEXT);
        p_dcs_billing_info_string.append(later_use_string);
        p_dcs_billing_info_string.append(SIP_DOUBLE_QUETO_TEXT);

        IMS_DLOG(IMS_IBCF_LOGLOW, "%s: P-DCS-Billing-Info string:%s \n", __func__,
                 p_dcs_billing_info_string.c_str());

	SIPia_deleteOtherHeader(msg_ptr, "P-DCS-Billing-Info", strlen("P-DCS-Billing-Info"));
        SIPia_AddOtherHeader(msg_ptr, "P-DCS-Billing-Info", p_dcs_billing_info_string.c_str(),
                             sizeof("P-DCS-Billing-Info") - 1, p_dcs_billing_info_string.length());
    }

    IMS_DLOG(IMS_IBCF_LOGLOW, "%s: successfull returned", __func__);
    return (IMS_SUCCESS);
}

/* Function:      IBCFb2b_iw_build_reason
 *
 * Descriptions:  The function is used to build Reason Header from SipReason to SIPia_Msg
 *
 * Inputs:        reasonPtr  - SipReason, suppose stored in SipwLiConEvnt *p_SipConEvntPtr
 *                msg_ptr    - output sip message
 *
 *                RFC3326 says:
 *                A SIP message MAY contain more than one Reason value (i.e., multiple
 *                Reason lines), but all of them MUST have different protocol values
 *                (e.g., one SIP and another Q.850).
 *
 *                https://tools.ietf.org/html/rfc3326
 *
 * Return:        void
 */
void
IBCFb2b_iw_build_reason(B2BUAEvent *event_ptr, SipReason *reasonPtr)
{
    if (!IS_VALID_PTR(event_ptr) || !IS_VALID_PTR(event_ptr->m_ptr) || !IS_VALID_PTR(reasonPtr))
    {
        IMS_DLOG(IMS_IBCF_LOGHIGH, "%s: Invalid Input Parameters, event_ptr(%p), reasonPtr(%p)", 
                 __func__, event_ptr, reasonPtr);
        return;
    }

    SIPia_Msg *              msg_ptr             = event_ptr->m_ptr;
    SIP_Reason_Protocol_Enum sip_reason_protocol = RSN_PROT_UNDEFINED;

    if (reasonPtr->pres == NOTPRSNT)
    {
        IMS_DLOG(IMS_IBCF_LOGLOW, "%s: reason header not present", __func__);
        return;
    }

    sip_reason_protocol = reason_protocol_convert(reasonPtr->prtcl);
    if (sip_reason_protocol == RSN_PROT_UNDEFINED)
    {
        return;
    }
    
    const char* reasontext = reinterpret_cast<char *>(reasonPtr->text.val);
    int len = reasonPtr->text.len;

    if ((reasonPtr->text.pres == NOTPRSNT) || (reasonPtr->text.len <= 0) ||
        (strlen(reinterpret_cast<const char *>(reasonPtr->text.val)) == 0))
    {
        reasontext = NULL_PTR;
        len = 0;
    }

    if (IS_VALID_PTR(SIPia_getReason(msg_ptr, sip_reason_protocol)) == TRUE)
    {
        SIPia_removeReason(msg_ptr, sip_reason_protocol);
    }

    SIPia_addReason(msg_ptr, reason_protocol_convert(reasonPtr->prtcl), (UNIT32)reasonPtr->cause,
                    const_cast<char *>(reasontext), len);

    return;
}

void
IBCFb2b_iw_build_reason(B2BUAEvent *event_ptr, SipwLiConEvnt *p_SipConEvntPtr)
{
    /*
    if ((IS_VALID_PTR(msg_ptr) == FALSE) || (IS_VALID_PTR(p_SipConEvntPtr) == FALSE))
    {
        return;
    }

    IBCFb2b_iw_build_reason(msg_ptr, &(p_SipConEvntPtr->reason));
    */

    return;
}

/* Function:      IBCFb2b_iw_build_alertinfo
 *
 * Descriptions:  The function is used to build Alert-Info Header from SipwLiConEvnt to SIPia_Msg
 *
 * Inputs:        p_SipConEvntPtr  - SipwLiConEvnt
 *                msg_ptr       - output sip message
 *
 * Return:        void
 */
void
IBCFb2b_iw_build_alertinfo(B2BUAEvent *event_ptr, SipwLiConEvnt *p_SipConEvntPtr)
{
    if (!IS_VALID_PTR(event_ptr) || !IS_VALID_PTR(event_ptr->m_ptr) ||
        !IS_VALID_PTR(p_SipConEvntPtr))
    {
        IMS_DLOG(IMS_IBCF_LOGHIGH, "%s: Invalid Input Parameters", __func__);
        return;
    }

    SIPia_Msg *msg_ptr = event_ptr->m_ptr;

    if ((p_SipConEvntPtr->alertInfo.pres == NOTPRSNT) || (p_SipConEvntPtr->alertInfo.len == 0) ||
        (strlen(reinterpret_cast<const char *>(p_SipConEvntPtr->alertInfo.val)) <= 0))
    {
        IMS_DLOG(IMS_IBCF_LOGLOW, "%s: alertInfo.pres(%d),alertInfo.len(%d)", 
                 __func__,
                 p_SipConEvntPtr->alertInfo.pres,
                 p_SipConEvntPtr->alertInfo.len);
        return;
    }

    SIPia_DeleteHeaderContainer(msg_ptr, SIPIA_ALERT_INFO);

    SIPia_addHeader_String(msg_ptr, SIPIA_ALERT_INFO,
                           reinterpret_cast<const char *>(p_SipConEvntPtr->alertInfo.val));

    return;
}

/*  FID16462.0, The Billing-Correlation-ID, BCID, is specified in [PCEM] as a
 *  24-byte binary structure, containing 4 bytes of NTP timestamp, 8 bytes of
 *  the unique identifier of the network element that generated the ID, 8
 *  bytes giving the time zone, and 4 bytes of monotonically increasing
 *  sequence number at that network element. This identifier is chosen
 *  to be globally unique within the system for a window of several
 *  months. This MUST be encoded in the P-DCS-Billing-Info header as a
 *  hexadecimal string of up to 48 characters. Leading zeroes MAY be
 *  suppressed.
 */
int
IBCFb2b_iw_build_DcsBillingInfoId(
    SIPia_Msg *msg_ptr, char *bcid, unsigned long suId, short ipVersion, int p_ipPort)
{
    struct timeval  tv;
    struct timezone tz;
    long            dst, minuteswest;
    unsigned long   sec, addr;
    int             diffhour, diffmin;
    int             len = 0, asciilen = 0;
    int             BCID_LEN = 80;
    char            temp[17];
    double long     dun;
    DString *       local_ip_dp    = NULL_PTR;
    char            local_addr[64] = {0};

    gettimeofday(&tv, &tz);
    dst         = tz.tz_dsttime;
    minuteswest = tz.tz_minuteswest;
    sec         = tv.tv_sec;

    bzero(bcid, BCID_LEN);
    bzero(temp, sizeof(temp));

    /* 4 bytes of Timestamp */
    sprintf(temp, "%08X", (int)sec);
    len = strlen(temp);
    strncat(bcid, temp, len);

    /* 8 bytes of Element_ID, 4 bytes of ip, 2 bytes of port, and 2 bytes of "0" */

    /* originally, local_addr is copy from LOCAL.local_addr*/
    if (IS_VALID_PTR(msg_ptr) == TRUE)
    {
        local_ip_dp = SIPia_getRcvng_local_ip(msg_ptr);
    }
    if (IS_VALID_PTR(local_ip_dp) == TRUE)
    {
        strncpy(local_addr, DString_getStr(local_ip_dp), DString_getLen(local_ip_dp));
    }
    else
    {
        strcpy(local_addr, "127.0.0.1");
    }

    // addr = inet_addr(LOCAL.local_addr);
    bzero(temp, sizeof(temp));
    sprintf(temp, "%08X", (int)addr);
    strncat(bcid, temp, strlen(temp));
    bzero(temp, sizeof(temp));
    sprintf(temp, "%04X", p_ipPort);
    strncat(bcid, temp, strlen(temp));
    strncat(bcid, "0000", 4);

    /* 8 bytes of TimeZone */
    /* Time Zone - Daylight Saving */
    if (dst == 0)
    {
        strncat(bcid, "0", 1);
    }
    else
    {
        strncat(bcid, "1", 1);
    }

    /* Time Zone - UTC offset */
    if (minuteswest < 0)
    {
        strncat(bcid, "0", 1);
        minuteswest = -minuteswest;
    }
    else
    {
        strncat(bcid, "1", 1);
    }

    diffhour = minuteswest / 60;
    if (diffhour)
    {
        bzero(temp, sizeof(temp));
        asciilen = convert_int32_to_ascii(temp, diffhour);
        if (asciilen == 1)
        {
            strncat(bcid, "0", 1);
        }
        strncat(bcid, temp, asciilen);

        diffmin = minuteswest % 60;
        if (diffmin)
        {
            bzero(temp, sizeof(temp));
            asciilen = convert_int32_to_ascii(temp, diffmin);
            if (asciilen == 1)
            {
                strncat(bcid, "0", 1);
            }
            strncat(bcid, temp, asciilen);
        }
        else
        {
            strncat(bcid, "00", 2);
        }
    }
    else
    {
        strncat(bcid, "00", 2);
        diffmin = minuteswest % 60;
        if (diffmin)
        {
            bzero(temp, sizeof(temp));
            asciilen = convert_int32_to_ascii(temp, diffmin);
            if (asciilen == 1)
            {
                strncat(bcid, "0", 1);
            }
            strncat(bcid, temp, asciilen);
        }
        else
        {
            strncat(bcid, "00", 2);
        }
    }

    strncat(bcid, "0000000000", 10);

    /* 4 bytes of Event_Counter */
    bzero(temp, sizeof(temp));
    sprintf(temp, "%08X", (int)suId);
    strncat(bcid, temp, strlen(temp));

    /*  use "/" to separate the IDs */
    strncat(bcid, "/", 1);

    /* Add the hardcoded FEID as "2661018861" */
    strncat(bcid, SIP_DCS_BILLING_INFO_FEID, sizeof(SIP_DCS_BILLING_INFO_FEID) - 1);

    len       = strlen(bcid);
    bcid[len] = '\0';

    return len;
}

/*
 * Function:    IBCFb2b_iw_build_from()
 *
 * Description: This function builds the From header and update From header in SIPia_Msg 
 *
 * Parameters:  p_conEvntPtr   - Pointer to SipwLiConEvnt.
 *              sip_msg_ptr    - Pointer to SIPia_Msg.
 *
 */
void IBCFb2b_iw_build_from (SipwLiConEvnt *p_SipConEvntPtr, B2BUAEvent *event_ptr)
{
	char 		fromStr[256] = {0};
	char 		displaynameStr[IMS_DISPLAYNAME_MAX_SIZE+1] = {0};
	bool            restrictIpAddr = FALSE;
	bool            isPhone = TRUE;
	bool            oliSet = FALSE;
	SIP_Addr        *addr_ptr;
	DMUSHORT profile_id = 0;
	DMUSHORT variant    = 0;

	if (!IS_VALID_PTR(p_SipConEvntPtr))
	{
		ASRT_RPT(ASRTBADPARAM, 0, "invalid SipwLiConEvnt %p", p_SipConEvntPtr);
		return;
	}

	if(!IS_VALID_PTR(event_ptr))
	{
		ASRT_RPT(ASRTBADPARAM, 0, "event_ptr_: %p is not valid ", event_ptr);
		return;
	}

	SIPia_Msg *sip_msg_ptr = event_ptr->m_ptr;
	if (!IS_VALID_PTR(sip_msg_ptr))
	{
		ASRT_RPT(ASRTBADPARAM, 0, "invalid sip msg ptr %p", sip_msg_ptr);
		return;
	}

	IMS_DLOG(IMS_IBCF_LOGLOW, "%s: Build FROM header Entered", __func__);
	fromStr[0] = '\0';

	URI *uri_ptr = SIPia_From_getURI(sip_msg_ptr);
	if (IS_VALID_PTR(uri_ptr) == FALSE)
	{
		IMS_DLOG(IMS_IBCF_LOGHIGH, "%s(), fail to get From URI.", __func__);
		return;
	}

	URI_Type_Enum uriType = URI_getType(uri_ptr);

	IBCF_SIPI_PROFILE_DATA *sipi_profile_data_ptr = NULL_PTR;
	IBCFb2b_iw_get_sipt_profile_id(event_ptr, profile_id, variant);
	if(profile_id == 0)
	{
		IMS_DLOG(IMS_IBCF_LOGHIGH, "%s, get SIPI profile failed", __func__);
		return;
	}
	sipi_profile_data_ptr = IBCF_get_sipi_profile_data_by_id(profile_id);
	if (IS_VALID_PTR(sipi_profile_data_ptr)== FALSE)
	{
		IMS_DLOG(IMS_IBCF_LOGHIGH, "%s, Error: sipi_profile_data_ptr is invalid.", __func__);
		return;
	}

	if (p_SipConEvntPtr->fromHdr.eh.pres == PRSNT_NODEF &&
			p_SipConEvntPtr->fromHdr.fromVal.pres != NOTPRSNT &&
			p_SipConEvntPtr->fromHdr.fromVal.len != 0)
	{
		if (strcasecmp(reinterpret_cast<char *>(p_SipConEvntPtr->fromHdr.fromVal.val), "unavailable") != 0)
		{
			if (p_SipConEvntPtr->rpid.eh.pres == PRSNT_NODEF)
			{
				/* If rpid privacy is on, it can be either name or number privacy or both (Full): */
				/* Username= "anonymous" and/or userinfo = "anonymous" */
				IMS_DLOG(IMS_IBCF_LOGLOW, "%s,ignoreCgpnAPRI:%d\n", __func__, p_SipConEvntPtr->ignoreCgpnAPRI);

				/* FID14989.0-0897, Table27c/Q.1912.5 of ND1017:2006/07 PN/GN mapping to from hdr */
				if ((p_SipConEvntPtr->rpid.rpiPrivacy.pres == PRSNT_NODEF) &&
						(!p_SipConEvntPtr->ignoreCgpnAPRI))
					/*	(!((ip2TrnkPtr->cfgInfo.sipProfilePtr != NULL) &&
						   ((ip2TrnkPtr->cfgInfo.sipProfilePtr->gnumIwProcExt == SIPA_ANON_AURIOFF) ||
						    (ip2TrnkPtr->cfgInfo.sipProfilePtr->gnumIwProcExt == SIPA_ANOFF_AURIOFF)) &&
						   (OS_STRSTR(p_SipConEvntPtr->fromHdr.fromVal.val, "anonymous") == NULL))))*/
				{
					switch (p_SipConEvntPtr->rpid.rpiPrivacy.val)
					{
						case SIPRPI_PRIVACY_URI:
							/* Display name and restricted CIN */
							if (p_SipConEvntPtr->rpid.displayName.pres == PRSNT_NODEF && 
									p_SipConEvntPtr->rpid.displayName.len <= IMS_DISPLAYNAME_MAX_SIZE)
							{
								strcat(displaynameStr, "\"");
								strncat(displaynameStr,
									reinterpret_cast<char *>(p_SipConEvntPtr->rpid.displayName.val),
									p_SipConEvntPtr->rpid.displayName.len);
								strcat(displaynameStr, "\"");
							}
							else
							{
								/* If no DisplayName and CIN restricted,
								      set user-name="Anonymous" BUG:40446 */
								strcat(displaynameStr, "\"Anonymous\"");
							}
							/* FID17364.0 SIP over TLS */
							/*strcat(fromStr, "<");
							if (uriType == SIPS_URI)
								IBCF_iw_add_uri_scheme(fromStr, uriType);
							else
								IBCF_iw_add_uri_scheme(fromStr, SIP_URI);*/
							strlcpy(reinterpret_cast<char *>(p_SipConEvntPtr->fromHdr.fromVal.val), 
								"anonymous", strlen("anonymous")+1);
							p_SipConEvntPtr->fromHdr.fromVal.len = strlen("anonymous");
							restrictIpAddr = TRUE;

							/* Set fromHdr.type to avoid adding phone-context */
							p_SipConEvntPtr->fromHdr.type = SIPA_USER_PART;
							break;

						case SIPRPI_PRIVACY_NAME:
							strcat(displaynameStr, "\"Anonymous\"");
							/*strcat(fromStr, " <");
							IBCF_iw_add_uri_scheme(fromStr, uriType);
							sipa_add_tel_num(fromStr, p_SipConEvntPtr->fromHdr.fromVal.val, p_SipConEvntPtr->fromHdr.fromVal.len, &numType, uriType);*/
							break;

						case SIPRPI_PRIVACY_FULL:
							strcat(displaynameStr, "\"Anonymous\"");
							/* FID17364.0 SIP over TLS */
							/*strcat(fromStr, "<");
							if (uriType == SIPS_URI)
								sipa_add_uri_scheme(fromStr, uriType);
							else
								sipa_add_uri_scheme(fromStr, SIP_URI);*/
						//	strcat(fromStr, "anonymous");
							restrictIpAddr = TRUE;
							/* Set fromHdr.type to avoid adding phone-context */
							p_SipConEvntPtr->fromHdr.type = SIPA_USER_PART;
							break;

						case SIPRPI_PRIVACY_OFF:
							if (p_SipConEvntPtr->rpid.displayName.pres == PRSNT_NODEF &&
									p_SipConEvntPtr->rpid.displayName.len <= IMS_DISPLAYNAME_MAX_SIZE)
							{
								strcat(displaynameStr, "\"");
								strncat(displaynameStr,
									reinterpret_cast<char *>(p_SipConEvntPtr->rpid.displayName.val),
									p_SipConEvntPtr->rpid.displayName.len);
								strcat(displaynameStr, "\"");
							}

						/*	strcat(fromStr, " <");
							sipa_add_uri_scheme(fromStr, uriType);
							sipa_add_tel_num(fromStr, p_SipConEvntPtr->fromHdr.fromVal.val, p_SipConEvntPtr->fromHdr.fromVal.len, &numType, uriType);*/
							break;

						default:
							break;
					}
				} /* rpid privacy present */
				/* If display name present use it w/ calling # */
				else
				{
			        	if (p_SipConEvntPtr->rpid.displayName.pres == PRSNT_NODEF &&
							p_SipConEvntPtr->rpid.displayName.len <= IMS_DISPLAYNAME_MAX_SIZE)
					{
						strcat(displaynameStr, "\"");
						strncat(displaynameStr,
								reinterpret_cast<char *>(p_SipConEvntPtr->rpid.displayName.val),
								p_SipConEvntPtr->rpid.displayName.len);
						strcat(displaynameStr, "\"");
					}

				}
			} /* rpid present */
			IMS_DLOG(IMS_IBCF_LOGLOW, "%s: dispaly name is %s", __func__, displaynameStr);

			/* FID 14890 check trunk config  cpcTag and insert ;cpc=xxx */
			char   cpcStr[256] = {0};
			if(sipi_profile_data_ptr->db_data.cpctag == SIPI_CPCTAG_FROM || sipi_profile_data_ptr->db_data.cpctag == SIPI_CPCTAG_BOTH)
			{
				if (p_SipConEvntPtr->sipCpcStr.pres == PRSNT_NODEF)
				{
					/* check left space */
					if (sizeof(fromStr) > (strlen(fromStr) + sizeof(";cpc=")
								+ strlen(reinterpret_cast<char *>(p_SipConEvntPtr->sipCpcStr.val))))
					{
						strcat(fromStr, ";cpc=");
						strcat(fromStr, reinterpret_cast<char *>(p_SipConEvntPtr->sipCpcStr.val));
						strcpy(cpcStr, reinterpret_cast<char *>(p_SipConEvntPtr->sipCpcStr.val));
					}
					else
					{
						IMS_DLOG(IMS_IBCF_LOGLOW, "%s: Not enough space available for copy cpc tag, fromStr=%s\n.",__func__, fromStr);
					}
				} 
				else /* Build ordinary cpc tag */
				{
					/* check left space */
					if (sizeof(fromStr) > (strlen(fromStr) + sizeof(";cpc=")
								+ strlen("ordinary")))
					{
						strcat(fromStr, ";cpc=");
						strcat(fromStr, "ordinary");
						IMS_DLOG(IMS_IBCF_LOGLOW, "%s: CpcStr from GCC is not present, Build ordinary cpc tag, fromStr=%s\n.",__func__, fromStr);
						strcpy(cpcStr, "ordinary");
					}
					else
					{
						IMS_DLOG(IMS_IBCF_LOGLOW, "%s: Not enough space available for copy cpc tag, fromStr=%s\n.",__func__, fromStr);
					}
				}
			}  
			else
			{
				IMS_DLOG(IMS_IBCF_LOGLOW, "%s: Not build cpc tag in From header\n.",__func__);
			}

			char     oliStr[4] = {0};
			char     isupoliStr[4] = {0};
			int      oliLen = 0;
			int      oliVal = p_SipConEvntPtr->oli.val;
			memset(oliStr, 0, sizeof(oliStr));
			memset(isupoliStr, 0, sizeof(isupoliStr));
			if((p_SipConEvntPtr->oli.pres == PRSNT_NODEF)&&
				(sipi_profile_data_ptr->db_data.olitag == SIPI_OLITAG_OLI_FROM ||
				 sipi_profile_data_ptr->db_data.olitag == SIPI_OLITAG_OLI_BOTH))
			{

				oliLen = sizeof(";oli=") + ((oliVal>99? 3*sizeof(char):2*sizeof(char)));
				/* oliLen will be the with of oli parameter */

				if(sizeof(fromStr)>(strlen(fromStr) + oliLen))
				{
					strcat(fromStr, ";oli=");
					sprintf(oliStr, "%02d",p_SipConEvntPtr->oli.val);
					strcat(fromStr, oliStr);
					oliSet = TRUE;
				}
				else
				{
					IMS_DLOG(IMS_IBCF_LOGLOW, "%s: Not enough space available for copy oli, fromStr=%s\n.",__func__, fromStr);
				}
			}
			if((oliSet == FALSE)&&
				(p_SipConEvntPtr->oli.pres == PRSNT_NODEF)&&
				(sipi_profile_data_ptr->db_data.olitag == SIPI_OLITAG_ISUPOLI_FROM ||
				 sipi_profile_data_ptr->db_data.olitag == SIPI_OLITAG_ISUPOLI_BOTH))
			{
				/* oliLen will be the with of oli parameter */
				oliLen = sizeof(";isup-oli=") + (oliVal>9 ?(oliVal>99? 3*sizeof(char):2*sizeof(char)):sizeof(char));

				if(sizeof(fromStr)>(strlen(fromStr) + oliLen))
				{
					strcat(fromStr, ";isup-oli=");
					sprintf(isupoliStr, "%02d",p_SipConEvntPtr->oli.val);
					strcat(fromStr, isupoliStr);
				}
				else
				{
					IMS_DLOG(IMS_IBCF_LOGLOW, "%s: Not enough space available for copy oli, fromStr=%s\n.",__func__, fromStr);
				}
				
			}
				
			switch (uriType)
			{
				case SIP_URI:
				case SIPS_URI:
				{
					Sip_URI *sip_uri_ptr = URI_getSipUri(uri_ptr);
					if (IS_VALID_PTR(sip_uri_ptr) == FALSE)
					{
						IMS_DLOG(IMS_IBCF_LOGHIGH, "%s(), Can not get SIP/SIPS URI for uri_ptr: %p",__func__, uri_ptr);
						return;
					}
					if(restrictIpAddr == TRUE)
					{
						if (Sip_URI_setHost(sip_msg_ptr, sip_uri_ptr, "anonymous.invalid", strlen("anonymous.invalid"), FALSE) != SIPIA_SUCCESS)
						{
							IMS_DLOG(IMS_IBCF_LOGHIGH, "%s(), call to Sip_URI_setHost() failed.", __func__);
							return;
						}
						isPhone = FALSE;
					}

				/*	if (p_SipConEvntPtr->fromHdr.type == SIPA_TELE_SUB_PART && 
						isPhone == TRUE)
					{
						strcat(fromStr, ";user=phone");
					}*/
					IMS_DLOG(IMS_IBCF_LOGLOW, "%s: FROM str is %s\n.",__func__, fromStr);

					SIPia_List_Cursor       *param_cursor_ptr;
					DString                 *tmpDString_ptr;
					char tempobj[256];
					SIPia_List_cursor_init(&(sip_uri_ptr->other_param_list), &param_cursor_ptr);
					while (IS_VALID_PTR(param_cursor_ptr))
					{
						tmpDString_ptr = reinterpret_cast<DString *>(SIPia_List_traverse(&param_cursor_ptr));
						if (!IS_VALID_PTR(tmpDString_ptr))
						{
							IMS_DLOG(IMS_IBCF_LOGHIGH, "%s(): Fail to traverse parameters in From header",__func__);
							return;
						}
						strlcpy(tempobj, DString_getStr(tmpDString_ptr), DString_getLen(tmpDString_ptr)+1);
						if((IS_VALID_PTR(strstr(tempobj,"cpc")) || IS_VALID_PTR(strstr(tempobj,"oli")))
							|| IS_VALID_PTR(strstr(tempobj,"isup-oli")))
						{
							IMS_DLOG(IMS_IBCF_LOGLOW, "list remove entered\n.");
							(void) SIPia_List_remove(&(sip_uri_ptr->other_param_list), reinterpret_cast<void*>(tmpDString_ptr));	
						}
					}

					DString *ds_ptr = NULL;
					char     old_user_name[USERNAME_MAX_SIZE + 1];
					char     new_user_name[USERNAME_MAX_SIZE + 1];
					char     temp_user_name[USERNAME_MAX_SIZE + 1];
					old_user_name[0] = '\0';
					new_user_name[0] = '\0';
					temp_user_name[0] = '\0';
					ds_ptr = Sip_URI_getUserName(sip_uri_ptr);
					if ((IS_VALID_PTR(ds_ptr) == FALSE) || (DString_getLen(ds_ptr) <= 0))
					{
						IMS_DLOG(IMS_IBCF_LOGLOW,"No userinfo or failed to get it in SIP/SIPS URI, ds_ptr: %p", ds_ptr);
						return;
					}
					int old_user_name_len = CPsip_DString_to_String(ds_ptr, old_user_name, USERNAME_MAX_SIZE);
					if (old_user_name_len == 0)
					{
						IMS_DLOG(IMS_IBCF_LOGHIGH, "%s: Failed to copy from DString", __func__);
						return;
					}
					
					int tel_num = 0;
					for(int offset=0;((offset<old_user_name_len)&&(offset<sizeof(old_user_name)-1)); offset++)
					{
						if (old_user_name[offset]==SIP_ASCII_SEMICOLON)
						{
							break;
						}
						tel_num++;
					}
					if(tel_num == strlen(old_user_name))
					{
						strncpy(old_user_name, 
							reinterpret_cast<char *>(p_SipConEvntPtr->fromHdr.fromVal.val),
							p_SipConEvntPtr->fromHdr.fromVal.len);
						old_user_name[p_SipConEvntPtr->fromHdr.fromVal.len]='\0';
					}
					else
					{
						char *old_user_name_ptr = old_user_name + tel_num;
						strncpy(temp_user_name,
							reinterpret_cast<char *>(p_SipConEvntPtr->fromHdr.fromVal.val),
							p_SipConEvntPtr->fromHdr.fromVal.len);
						temp_user_name[p_SipConEvntPtr->fromHdr.fromVal.len]='\0';
						strncat(temp_user_name, old_user_name_ptr, strlen(old_user_name_ptr));
						strlcpy(old_user_name, temp_user_name, strlen(temp_user_name)+1);
					}
					IMS_DLOG(IMS_IBCF_LOGLOW, "old_user_name before deleting: %s", old_user_name);

					string oldnameStr = old_user_name;
					IBCFb2b_iw_dlt_tagStr(oldnameStr, "cpc");
					IBCFb2b_iw_dlt_tagStr(oldnameStr, "isup-oli");
					IBCFb2b_iw_dlt_tagStr(oldnameStr, "oli");
					//IBCFb2b_iw_dlt_tagStr(oldnameStr, "isub");
					strncpy(old_user_name,oldnameStr.c_str(),sizeof(old_user_name));
					IMS_DLOG(IMS_IBCF_LOGLOW, "old_user_name after deleting: %s", old_user_name);

					int len = strlen(old_user_name) + strlen(fromStr);
					if(len > USERNAME_MAX_SIZE)
					{
						IMS_DLOG(IMS_IBCF_LOGMED, "Not enough space to add string, len: %d",len);
						return;
					}
					strncpy(new_user_name,old_user_name,strlen(old_user_name));
					new_user_name[strlen(old_user_name)]='\0';
					strcat(new_user_name, fromStr);
					if(DString_copy(sip_msg_ptr, ds_ptr, new_user_name, strlen(new_user_name),0) < 0)
					{
						IMS_DLOG(IMS_IBCF_LOGHIGH,"%s: failed to update from header username", __func__);
						return;
					}
					SIPia_InvalidateRawText(sip_msg_ptr, FROM);
				}
					break;
				case TEL_URI:
				{
					SIPia_List	*other_list_ptr;
					DString *d_cpc_ptr = NULL;
					DString *d_oli_ptr = NULL;
					DString *d_isupoli_ptr = NULL;
					DString *ds_ptr = NULL;
					Tel_URI *tel_uri_ptr = URI_getTelUri(uri_ptr);
					if (IS_VALID_PTR(tel_uri_ptr) == FALSE)
					{
						IMS_DLOG(IMS_IBCF_LOGHIGH, "%s(), Can not get TEL URI for uri_ptr: %p",__func__,uri_ptr);
						return;
					}
					ds_ptr = Tel_URI_getSubscriber(tel_uri_ptr);
					if ((IS_VALID_PTR(ds_ptr) == FALSE) || (DString_getLen(ds_ptr) <= 0))
					{
						IMS_DLOG(IMS_IBCF_LOGLOW,"No Subscriber or failed to get it in TEL URI, ds_ptr: %p", ds_ptr);
						return;
					}
					if(DString_copy(sip_msg_ptr, ds_ptr,
							 	reinterpret_cast<char *>(p_SipConEvntPtr->fromHdr.fromVal.val),
							  	p_SipConEvntPtr->fromHdr.fromVal.len,0) < 0)
					{
						IMS_DLOG(IMS_IBCF_LOGHIGH,"%s: failed to update from header username", __func__);
						return;
					}

					d_cpc_ptr = TASutil_genuri_find_value("cpc", uri_ptr, sip_msg_ptr);
					if(IS_VALID_PTR(d_cpc_ptr) == TRUE && DString_getLen(d_cpc_ptr) > 0)
					{
						TASutil_genuri_remove_nvp("cpc", uri_ptr);
					}

					d_oli_ptr = TASutil_genuri_find_value("oli", uri_ptr, sip_msg_ptr);
					if(IS_VALID_PTR(d_oli_ptr) == TRUE && DString_getLen(d_oli_ptr) > 0)
					{
						TASutil_genuri_remove_nvp("oli", uri_ptr);
					}

					d_isupoli_ptr = TASutil_genuri_find_value("isup-oli", uri_ptr, sip_msg_ptr);
					if(IS_VALID_PTR(d_isupoli_ptr) == TRUE && DString_getLen(d_isupoli_ptr) > 0)
					{
						TASutil_genuri_remove_nvp("isup-oli", uri_ptr);
					}

					if(strlen(cpcStr) > 0)
					{
						TASutil_genuri_add_nvp("cpc", cpcStr, sip_msg_ptr, uri_ptr);
					}
					if(strlen(oliStr) > 0)
					{
						TASutil_genuri_add_nvp("oli", oliStr, sip_msg_ptr, uri_ptr);
					}
					if(strlen(isupoliStr) > 0)
					{
						TASutil_genuri_add_nvp("isup-oli", isupoliStr, sip_msg_ptr, uri_ptr);
					}
					TASutil_invalidateRawText(sip_msg_ptr, FROM);	
				}
					break;
				default:
					IMS_DLOG(IMS_IBCF_LOGHIGH, "Unsupported URI type: %d", uriType);
					return;
			}
		}
		else
		{
			DString *ds_ptr = NULL;
			switch (uriType)
			{
				case SIP_URI:
				case SIPS_URI:
				{
					Sip_URI *sip_uri_ptr = URI_getSipUri(uri_ptr);
					if (IS_VALID_PTR(sip_uri_ptr) == FALSE)
					{
						IMS_DLOG(IMS_IBCF_LOGHIGH, "%s(), Can not get SIP/SIPS URI for uri_ptr: %p",__func__, uri_ptr);	
						return;
					}
					ds_ptr = Sip_URI_getUserName(sip_uri_ptr);
					if ((IS_VALID_PTR(ds_ptr) == FALSE) || (DString_getLen(ds_ptr) <= 0))
					{
						IMS_DLOG(IMS_IBCF_LOGLOW,"No userinfo or failed to get it in SIP/SIPS URI, ds_ptr: %p", ds_ptr);
						return;
					}
					if(DString_copy(sip_msg_ptr, ds_ptr, "unavailable", strlen("unavailable"),0) < 0)
					{
						IMS_DLOG(IMS_IBCF_LOGHIGH,"%s: failed to update from header username", __func__);
						return;
					}

					SIPia_List_Cursor       *param_cursor_ptr;
					DString                 *tmpDString_ptr;
					char tempobj[256];
					SIPia_List_cursor_init(&(sip_uri_ptr->other_param_list), &param_cursor_ptr);
					while (IS_VALID_PTR(param_cursor_ptr))
					{
						tmpDString_ptr = reinterpret_cast<DString *>(SIPia_List_traverse(&param_cursor_ptr));
						if (!IS_VALID_PTR(tmpDString_ptr))
						{
							IMS_DLOG(IMS_IBCF_LOGHIGH, "%s(): Fail to traverse parameters in From header",__func__);
							return;
						}
						strlcpy(tempobj, DString_getStr(tmpDString_ptr), DString_getLen(tmpDString_ptr)+1);
						if((IS_VALID_PTR(strstr(tempobj,"cpc")) || IS_VALID_PTR(strstr(tempobj,"oli")))
							|| IS_VALID_PTR(strstr(tempobj,"isup-oli")))
						{
							IMS_DLOG(IMS_IBCF_LOGLOW, "list remove entered\n.");
							(void) SIPia_List_remove(&(sip_uri_ptr->other_param_list), reinterpret_cast<void*>(tmpDString_ptr));	
						}
					}

					SIPia_InvalidateRawText(sip_msg_ptr, FROM);
				}
					break;
				case TEL_URI:
				{	
					DString *d_cpc_ptr = NULL;
					DString *d_oli_ptr = NULL;
					DString *d_isupoli_ptr = NULL;
					Tel_URI *tel_uri_ptr = URI_getTelUri(uri_ptr);
					if (IS_VALID_PTR(tel_uri_ptr) == FALSE)
					{
						IMS_DLOG(IMS_IBCF_LOGHIGH, "%s(), Can not get TEL URI for uri_ptr: %p",__func__,uri_ptr);
						return;
					}
					ds_ptr = Tel_URI_getSubscriber(tel_uri_ptr);
					if ((IS_VALID_PTR(ds_ptr) == FALSE) || (DString_getLen(ds_ptr) <= 0))
					{
						IMS_DLOG(IMS_IBCF_LOGLOW,"No Subscriber or failed to get it in TEL URI, ds_ptr: %p", ds_ptr);
						return;
					}
					if(DString_copy(sip_msg_ptr, ds_ptr, "unavailable", strlen("unavailable"),0) < 0)
					{
						IMS_DLOG(IMS_IBCF_LOGHIGH,"%s: failed to update from header username", __func__);
						return;
					}

					d_cpc_ptr = TASutil_genuri_find_value("cpc", uri_ptr, sip_msg_ptr);
					if(IS_VALID_PTR(d_cpc_ptr) == TRUE && DString_getLen(d_cpc_ptr) > 0)
					{
						TASutil_genuri_remove_nvp("cpc", uri_ptr);
					}

					d_oli_ptr = TASutil_genuri_find_value("oli", uri_ptr, sip_msg_ptr);
					if(IS_VALID_PTR(d_oli_ptr) == TRUE && DString_getLen(d_oli_ptr) > 0)
					{
						TASutil_genuri_remove_nvp("oli", uri_ptr);
					}

					d_isupoli_ptr = TASutil_genuri_find_value("isup-oli", uri_ptr, sip_msg_ptr);
					if(IS_VALID_PTR(d_isupoli_ptr) == TRUE && DString_getLen(d_isupoli_ptr) > 0)
					{
						TASutil_genuri_remove_nvp("isup-oli", uri_ptr);
					}

					SIPia_InvalidateRawText(sip_msg_ptr, FROM);
				}
					break;
				default:
					IMS_DLOG(IMS_IBCF_LOGHIGH, "Unsupported URI type: %d", uriType);
					return;
			}

		 }
	}
	else /* No From Hdr present */
	{
		IMS_DLOG(IMS_IBCF_LOGLOW,"%s: no value in fromval, just pass through FROM header", __func__);
		return;
	}
	if ((sipi_profile_data_ptr->db_data.sipisubmap)
				&& (p_SipConEvntPtr->cgPtyIsubAddr.eh.pres)
				&& (p_SipConEvntPtr->cgPtyIsubAddr.addrInfo.pres))
	{
		IBCFb2b_iw_add_isub(reinterpret_cast<char *>(p_SipConEvntPtr->cgPtyIsubAddr.addrInfo.val),
				p_SipConEvntPtr->cgPtyIsubAddr.encode, sip_msg_ptr, uri_ptr, SIPIA_FROM);
	}

	SIP_From *from_header = SIPia_getFromHeader(sip_msg_ptr);
	addr_ptr = SIPia_From_getAddr( from_header );
	if(IS_VALID_PTR(addr_ptr) == TRUE && strlen(displaynameStr) > 0)
	{
		
		if(DString_copy(sip_msg_ptr, SIP_Addr_getDisplayName(addr_ptr), displaynameStr, strlen(displaynameStr),0) < 0)
		{
			IMS_DLOG(IMS_IBCF_LOGHIGH,"%s: failed to update from header dispalyname", __func__);
			return;
		}
		SIPia_InvalidateRawText(sip_msg_ptr, FROM);

	}

	return;
}

/*
 * Name:		IBCFb2b_iw_add_isub										 
 *																			 
 * Purpose:		Copy isubStr and isubEncode into sipaBldBuf					 
 *																		 
 *																			 
 * Params:		isubStr		- string contains ISDN subaddress,			 
 *					should be NULL-terminated					 
 *			isubEncode	- encoding type of isub						 
 *			sip_msg_ptr	- sip message ptr							 
 *			uri_ptr		- URI pointer to add isub					 
 *			header_type	- type of header to add isub 				 			 
 *																 
 */

void IBCFb2b_iw_add_isub(char* isubStr, short isubEncode, SIPia_Msg *sip_msg_ptr, URI *uri_ptr, SIP_Header_Type header_type)
{
	URI_Type_Enum	uri_type;
	Sip_URI 		*sip_uri_ptr = NULL;
	Tel_URI 		*tel_uri_ptr = NULL;
	DString 		*ds_ptr = NULL;
	char			old_user_name[USERNAME_MAX_SIZE + 1];
	char			new_user_name[USERNAME_MAX_SIZE + 1];
	char			*new_user_name_ptr = NULL;
	int 			old_user_name_len = 0;
	int 			new_user_name_len = 0; 
	int 			len = 0;
	string 			oldnameStr = old_user_name;
	DString 		*d_isub_ptr = NULL;

	IMS_DLOG_SIP_MSG(IMS_IBCF_LOGLOW, sip_msg_ptr, "%s(): sip message is.",__func__);

	if((IS_VALID_PTR(isubStr) == FALSE) || (isubStr[0] == '\0'))
	{
		IMS_DLOG(IMS_IBCF_LOGHIGH, "%s: isubStr is NULL or empty.", __func__, isubStr);
		return;
	}

	if ( isubEncode != SIP_ISUB_ENCODE_IA5
			&& isubEncode != SIP_ISUB_ENCODE_BCD
			&& isubEncode != SIP_ISUB_ENCODE_NSAP)
	{
		IMS_DLOG(IMS_IBCF_LOGHIGH, "%s: Incorrect isub encoding type %d when adding isub.", __func__, isubEncode);
		return;
	}
	
	uri_type = URI_getType(uri_ptr);
	switch (uri_type)
	{
	case SIP_URI:
	case SIPS_URI:
			IBCFutil_removeISUB_fromURI(uri_ptr, sip_msg_ptr);
			
			uri_ptr = SIPia_To_getURI(sip_msg_ptr); 
			if (IS_VALID_PTR(uri_ptr) == FALSE)
			{
				IMS_DLOG(IMS_IBCF_LOGHIGH, "%s, Invalid TO URI.", __func__);	
				return;
			}

			old_user_name[0] = '\0';
			sip_uri_ptr = URI_getSipUri(uri_ptr);
			if (IS_VALID_PTR(sip_uri_ptr) == FALSE)
			{
					IMS_DLOG(IMS_IBCF_LOGHIGH, "%s, Can not get SIP/SIPS URI for uri_ptr: %p", __func__, uri_ptr);
					return;
			}

			ds_ptr = Sip_URI_getUserName(sip_uri_ptr);
			if ((IS_VALID_PTR(ds_ptr) == FALSE) || (DString_getLen(ds_ptr) <= 0))
			{
					IMS_DLOG(IMS_IBCF_LOGLOW,
							"%s, No userinfo or failed to get it in SIP/SIPS URI, ds_ptr: %p", __func__, ds_ptr);
					return;
			}

			old_user_name_len = CPsip_DString_to_String(ds_ptr, old_user_name, USERNAME_MAX_SIZE);
			old_user_name_len = strlen(old_user_name);
			
			IMS_DLOG(IMS_IBCF_LOGLOW, "%s, old_user_name = %s, old_user_name_len = %d.",
						__func__, old_user_name, old_user_name_len);
			
			if (old_user_name_len == 0)
			{
					IMS_DLOG(IMS_IBCF_LOGHIGH, "%s, Failed to get it in SIP/SIPS URI", __func__);
					return;
			}
			
			new_user_name[0] = '\0';
			new_user_name_ptr = new_user_name;

			len = old_user_name_len + ISUB_TOKEN_LEN + strlen(isubStr) + 2
							+ ISUB_ENCODE_TOKEN_LEN + strlen(SIP_ISUB_ENCODE_TEXT_IA5) + 2;
			if (len > USERNAME_MAX_SIZE)
			{
					IMS_DLOG(IMS_IBCF_LOGMED, "%s, Not enough space to add string, len: %d",
									__func__, len);
					return;
			}

			// Copy the existing user name
			strNcat(&new_user_name_ptr, old_user_name, old_user_name_len);

			// Add ";"
			strNcat(&new_user_name_ptr, ";", 1);

			// Add "isub" token
			strNcat(&new_user_name_ptr, ISUB_TOKEN, ISUB_TOKEN_LEN);
			
			// Add "="
			strNcat(&new_user_name_ptr, "=", 1);

			// Add the new value 
			strNcat(&new_user_name_ptr, isubStr, strlen(isubStr));

			// Add ";"
			strNcat(&new_user_name_ptr, ";", 1);
			
			// Add ";"
			strNcat(&new_user_name_ptr, ISUB_ENCODE_TOKEN, ISUB_ENCODE_TOKEN_LEN);
			
			// Add "="
			strNcat(&new_user_name_ptr, "=", 1);
			
			switch(isubEncode)
			{
				case SIP_ISUB_ENCODE_IA5:
				{
					strNcat(&new_user_name_ptr, SIP_ISUB_ENCODE_TEXT_IA5, sizeof(SIP_ISUB_ENCODE_TEXT_IA5)-1);
					break;
				}
				case SIP_ISUB_ENCODE_BCD:
				{
					strNcat(&new_user_name_ptr, SIP_ISUB_ENCODE_TEXT_BCD, sizeof(SIP_ISUB_ENCODE_TEXT_BCD -1));
					break;
				}
				case SIP_ISUB_ENCODE_NSAP:
				{
					strNcat(&new_user_name_ptr, SIP_ISUB_ENCODE_TEXT_NSAP, sizeof(SIP_ISUB_ENCODE_TEXT_NSAP)-1);
					break;
				}
				default :
				{
					IMS_DLOG(IMS_IBCF_LOGHIGH, "%s, cdpIsubEncode from cc not supported. %d.", __func__, isubEncode);							
					break;
				} 
			}
				
			new_user_name_len = new_user_name_ptr - new_user_name;

			IMS_DLOG(IMS_IBCF_LOGLOW, "%s, New user name added to SIP/SIPS URI: %.*s",
							__func__, new_user_name_len, new_user_name);

			if (DString_copy(sip_msg_ptr, ds_ptr, new_user_name, new_user_name_len, 0) < 0)
			{
					IMS_DLOG(IMS_IBCF_LOGHIGH,
							"%s, Failed to update username parameter in URI", __func__);
					return;
			}
			
			SIPia_InvalidateRawText(sip_msg_ptr, header_type);

			break;
								
	case TEL_URI:
			tel_uri_ptr = URI_getTelUri(uri_ptr);
			if (IS_VALID_PTR(tel_uri_ptr) == FALSE)
			{
					IMS_DLOG(IMS_IBCF_LOGHIGH, "%s, Can not get TEL URI for uri_ptr: %p", __func__, uri_ptr);
					return;
			}

			IBCFutil_removeISUB_fromURI(uri_ptr, sip_msg_ptr);

			TASutil_genuri_add_nvp(ISUB_TOKEN, isubStr, sip_msg_ptr, uri_ptr);											
		
			switch(isubEncode)
			{
				case SIP_ISUB_ENCODE_IA5:
				{
					TASutil_genuri_add_nvp(ISUB_ENCODE_TOKEN, SIP_ISUB_ENCODE_TEXT_IA5,
											sip_msg_ptr, uri_ptr);
					break;
				}
				case SIP_ISUB_ENCODE_BCD:
				{
					TASutil_genuri_add_nvp(ISUB_ENCODE_TOKEN, SIP_ISUB_ENCODE_TEXT_BCD,
										sip_msg_ptr, uri_ptr);
					break;
				}
				case SIP_ISUB_ENCODE_NSAP:
				{
					TASutil_genuri_add_nvp(ISUB_ENCODE_TOKEN, SIP_ISUB_ENCODE_TEXT_NSAP,
											sip_msg_ptr, uri_ptr);
					break;
				}
				default :
				{
					IMS_DLOG(IMS_IBCF_LOGHIGH, "%s, cdpIsubEncode from cc not supported. %d.", __func__, isubEncode);							
					break;
				}
			}		  

			SIPia_InvalidateRawText(sip_msg_ptr, header_type);
			break;
		default:
				IMS_DLOG(IMS_IBCF_LOGHIGH, "%s, Invalid URI type supported. %d.", __func__, uri_type);	
				break;
		}	

		IMS_DLOG_SIP_MSG(IMS_IBCF_LOGLOW, sip_msg_ptr, "%s(): new sip message is.",__func__);	

	return;

}

/*
 *	Name:			IBCFb2b_iw_build_TO	
 *
 *	Parameters:		p_conEvntPtr	-	Pointer to SipwLiConEvnt
 *				event_ptr	-	Pintet to sip event
 *
 *	Returns:		IMS_FAIL / IMS_SUCCESS			
 */
IMS_RETVAL IBCFb2b_iw_build_TO(
		SipwLiConEvnt *p_SipConEvntPtr,
		B2BUAEvent *event_ptr)
{
	URI 		*uri_ptr = NULL;
	URI_Type_Enum	uriType;
	SIPia_Msg 	*sip_msg_ptr = NULL;
	DMUSHORT 	profile_id = 0;
	DMUSHORT 	variant = 0;
	IBCF_SIPI_PROFILE_DATA	*sipi_profile_data_ptr = NULL;
	Sip_URI 		*sip_uri_ptr = NULL;
	Tel_URI 		*tel_uri_ptr = NULL;
	DString 		*ds_ptr = NULL;
	
	if (IS_VALID_PTR(event_ptr) == FALSE || IS_VALID_PTR(p_SipConEvntPtr) == FALSE)
	{
		IMS_DLOG(IMS_IBCF_LOGHIGH, "%s, Invalid event_ptr or p_conEvntPtr.", __func__, event_ptr);	
		return (IMS_FAIL);
	}
	
	sip_msg_ptr = event_ptr->m_ptr;	
	if (IS_VALID_PTR(sip_msg_ptr) == FALSE)
	{
		IMS_DLOG(IMS_IBCF_LOGHIGH, "%s, Invalid sip_msg_ptr.", __func__);	
		return (IMS_FAIL);
	}
	
	uri_ptr = SIPia_To_getURI(sip_msg_ptr); 
	if (IS_VALID_PTR(uri_ptr) == FALSE)
	{
		IMS_DLOG(IMS_IBCF_LOGHIGH, "%s, Invalid TO URI.", __func__);	
		return (IMS_FAIL);
	}
	
	IMS_DLOG(IMS_IBCF_LOGLOW, "%s,toHdr.toVal.val is %s, len = %d",
 							__func__, reinterpret_cast<char *>(p_SipConEvntPtr->toHdr.toVal.val), p_SipConEvntPtr->toHdr.toVal.len);

	if (p_SipConEvntPtr->toHdr.toVal.val != NULL && p_SipConEvntPtr->toHdr.toVal.len != 0)
	{
		uriType = URI_getType(uri_ptr);
		switch (uriType)
		{
		case SIP_URI:
		case SIPS_URI:			
			sip_uri_ptr = URI_getSipUri(uri_ptr);
			if (IS_VALID_PTR(sip_uri_ptr) == FALSE)
			{
					IMS_DLOG(IMS_IBCF_LOGHIGH, "%s, Can not get SIP/SIPS URI for uri_ptr: %p", __func__, uri_ptr);
					return (IMS_FAIL);
			}
			
			ds_ptr = Sip_URI_getUserName(sip_uri_ptr);
			if ((IS_VALID_PTR(ds_ptr) == FALSE) || (DString_getLen(ds_ptr) <= 0))
			{
					IMS_DLOG(IMS_IBCF_LOGHIGH,
							"%s, No userinfo or failed to get it in SIP/SIPS URI, ds_ptr: %p", __func__, ds_ptr);
					return (IMS_FAIL);
			}
			break;
			
		case TEL_URI:
			tel_uri_ptr = URI_getTelUri(uri_ptr);
			if (IS_VALID_PTR(tel_uri_ptr) == FALSE)
			{
					IMS_DLOG(IMS_IBCF_LOGHIGH, "%s, Can not get TEL URI for uri_ptr: %p", __func__, uri_ptr);
					return (IMS_FAIL);
			}
			
			ds_ptr = Tel_URI_getSubscriber(tel_uri_ptr);
			if ((IS_VALID_PTR(ds_ptr) == FALSE) || (DString_getLen(ds_ptr) <= 0))
			{
					IMS_DLOG(IMS_IBCF_LOGHIGH,
							"%s, No telenum or failed to get it in TEL URI, ds_ptr: %p", __func__, ds_ptr);
					return (IMS_FAIL);
			}
			break;
			
		default:
			IMS_DLOG(IMS_IBCF_LOGHIGH, 
				"%s, unexpected URI type = %d.", __func__, uriType);
			return (IMS_FAIL);
		}		
					
		if (DString_copy(sip_msg_ptr, ds_ptr, reinterpret_cast<char *>(p_SipConEvntPtr->toHdr.toVal.val),
				(UNIT16)(p_SipConEvntPtr->toHdr.toVal.len), 0) < 0)
		{
					IMS_DLOG(IMS_IBCF_LOGHIGH,
							"%s, Failed to update username parameter in URI", __func__);
					return (IMS_FAIL);
		}
			
		SIPia_InvalidateRawText(sip_msg_ptr, SIPIA_TO);		
		
		IMS_DLOG_SIP_MSG(IMS_IBCF_LOGLOW, sip_msg_ptr, "%s(): sip message is.", __func__);
	}

	IBCFb2b_iw_get_sipt_profile_id(event_ptr, profile_id, variant);
	if (profile_id == 0)
	{
		IMS_DLOG(IMS_IBCF_LOGHIGH, "%s, get SIPI profile failed", __func__);
		return (IMS_FAIL);
	}
	
	sipi_profile_data_ptr = IBCF_get_sipi_profile_data_by_id(profile_id);
	if (IS_VALID_PTR(sipi_profile_data_ptr)== FALSE)
	{
		IMS_DLOG(IMS_IBCF_LOGHIGH, "%s, Error: sipi_profile_data_ptr is invalid.", __func__);
		return (IMS_FAIL);
	}

	IMS_DLOG(IMS_IBCF_LOGLOW, "%s, sipisubmap = %d, cdPtyIsubAddr.eh.pres = %d, cdPtyIsubAddr.addrInfo.pres = %d.",
					__func__, sipi_profile_data_ptr->db_data.sipisubmap,
					p_SipConEvntPtr->cdPtyIsubAddr.eh.pres,
					p_SipConEvntPtr->cdPtyIsubAddr.addrInfo.pres);
					
	IMS_DLOG_SIP_MSG(IMS_IBCF_LOGLOW, sip_msg_ptr, "%s(): sip message is.", __func__);


	/* FID16205.0 Add called party subaddress to isub parameter */
	if ((sipi_profile_data_ptr->db_data.sipisubmap == DBYES) &&
                (p_SipConEvntPtr->cdPtyIsubAddr.eh.pres) &&
                (p_SipConEvntPtr->cdPtyIsubAddr.addrInfo.pres))
	{	 

		IMS_DLOG(IMS_IBCF_LOGLOW, "%s, cdPtyIsubAddr.addrInfo.val = %s, p_SipConEvntPtr->cdPtyIsubAddr.encode = %d.",
							__func__, p_SipConEvntPtr->cdPtyIsubAddr.addrInfo.val,
							p_SipConEvntPtr->cdPtyIsubAddr.encode);

		IBCFb2b_iw_add_isub(reinterpret_cast<char *>(p_SipConEvntPtr->cdPtyIsubAddr.addrInfo.val), 
				(short)p_SipConEvntPtr->cdPtyIsubAddr.encode, sip_msg_ptr, uri_ptr, SIPIA_TO);
	}	 
	/* enf of FID 16205.0 */
}

IMS_RETVAL
IBCFb2b_iw_build_AcceptContact(SIPia_Msg  *sip_msg_ptr, SipwLiConEvnt *p_SipConEvntPtr)
{ 
	char						feature_tag[256];
	char						feature_tag_tmp[256];
	char						*feature_tag_ptr = NULL;
	int 						feature_tag_len = 0;
	BOOL						header_add_ret = FALSE;
	SIPia_List					*ac_list_ptr = NULL;

	if (IS_VALID_PTR(sip_msg_ptr) == FALSE || IS_VALID_PTR(p_SipConEvntPtr) == FALSE)
	{
		ASRT_RPT(ASRTBADPARAM, 0,
				 "Bad parameter(s): sip_msg_ptr=%p or p_SipConEvntPtr", sip_msg_ptr);
		return (IMS_FAIL);
	}
	
	/* Find whether there is Accept-Contact header in the INVITE message */
	ac_list_ptr = SIPia_getAcceptContactList(sip_msg_ptr);
	if (IS_VALID_PTR(ac_list_ptr) == TRUE)
	{
		IMS_DLOG(IMS_IBCF_LOGLOW, "%s: Accept-Contact header in the SIP messge, pass through.", __func__);
		return (IMS_SUCCESS);
	}
	
	memset(feature_tag, 0, sizeof(feature_tag));	 
	memset(feature_tag_tmp, 0, sizeof(feature_tag_tmp));	
	feature_tag_ptr = feature_tag;

	IMS_DLOG(IMS_IBCF_LOGLOW, "%s, acceptLangStr is [%s], length is %d.",
					__func__,
					p_SipConEvntPtr->sipAcceptContactHdr.acceptLangStr.val,
					p_SipConEvntPtr->sipAcceptContactHdr.acceptLangStr.len); 
	

	if ((p_SipConEvntPtr->sipAcceptContactHdr.eh.pres == PRSNT_NODEF)
	   && (p_SipConEvntPtr->sipAcceptContactHdr.acceptLangStr.pres == PRSNT_NODEF))
	{
		//	SIPia_DeleteHeaderContainer(sip_msg_ptr, SIPIA_ACCEPT_CONTACT);

			strNcat(&feature_tag_ptr, "language=", sizeof("language=")-1);
			strNcat(&feature_tag_ptr, (const char*)p_SipConEvntPtr->sipAcceptContactHdr.acceptLangStr.val,
					   p_SipConEvntPtr->sipAcceptContactHdr.acceptLangStr.len);

	} 
	else
	{
		IMS_DLOG(IMS_IBCF_LOGLOW, "%s NO Accept Contact needs to be updated.", __func__);
				   
		return (IMS_SUCCESS);
	}  
	  
	feature_tag_len = feature_tag_ptr - feature_tag;
	IMS_DLOG(IMS_IBCF_LOGLOW, "%s, feature_tag_len = %d.", __func__, feature_tag_len);
	
	strlcpy(feature_tag_tmp, feature_tag, feature_tag_len + 1);
	IMS_DLOG(IMS_IBCF_LOGLOW, "%s, feature_tag_tmp = %s.", __func__, feature_tag_tmp);
	
	/* add accept contact header */
	header_add_ret = SIPia_addHeader_String(sip_msg_ptr,
			SIPIA_ACCEPT_CONTACT,feature_tag_tmp,feature_tag_len,LIST_FRONT);
	
	if( TRUE ==  header_add_ret )
	{
		IMS_DLOG(IMS_IBCF_LOGLOW, "%s returns: "
					"add the feature tag [%s] success.", feature_tag_tmp);

		IMS_DLOG_SIP_MSG(IMS_IBCF_LOGLOW, sip_msg_ptr, "%s(): new sip message is.",__func__);	
	}

   return (IMS_SUCCESS);
}

/**PROC+**********************************************************************/
/* Name:      IBCFb2b_iw_build_uri                                           */
/*                                                                           */
/* Purpose:   IBCF added parameters to request URI 		  	     */
/*            based on the data in structure SipwLiConEvnt 		     */
/*            which had already been handled by GCC.			     */
/*	      Also should refer to the egress TG's interworking profile	     */
/* Returns:   Nothing.                                                       */
/*                                                                           */
/* Params:                                                                   */
/*                                                                           */
/* Operation:                                                                */
/*                                                                           */
/**PROC-**********************************************************************/
void
IBCFb2b_iw_build_uri(B2BUAEvent *event_ptr, SipwLiConEvnt *p_SipConEvntPtr)
{
    SIPia_Msg *            sip_msg_ptr        = NULL_PTR;
    B2BUACallData *        calldata_ptr       = NULL_PTR;
    B2BUALegData *         egress_leg_ptr     = NULL_PTR;
    IBCF_TRUNK_GROUP_DATA *egress_tg_data_ptr = NULL_PTR;
    URI *                  req_uri_ptr        = NULL_PTR;
    Tel_URI *              teluri_ptr         = NULL_PTR;
    Sip_URI *              sipuri_ptr         = NULL_PTR;
    TknU8                  npdi;
    char                   bs_buff[BSTRING_BUFSZ(SIP_MAX_HEADER_LEN)];
    char                   cic_text[30];
    SIPia_List *           param_list = NULL_PTR;
    char                   dai[30];
    DString *              tel_dai;
    unsigned short         sipi_profile_id;

    if (IS_VALID_PTR(event_ptr) == FALSE)
    {
        IMS_DLOG(IMS_IBCF_LOGHIGH, "%s, event_ptr null.", __func__);
        return;
    }

    sip_msg_ptr = event_ptr->m_ptr;

    if (IS_VALID_PTR(sip_msg_ptr) == FALSE)
    {
        IMS_DLOG(IMS_IBCF_LOGHIGH, "%s, sip_msg_ptr null.", __func__);
        return;
    }

    calldata_ptr = event_ptr->calldata_ptr;
    if (IS_VALID_PTR(calldata_ptr) == FALSE)
    {
        IMS_DLOG(IMS_IBCF_LOGHIGH, "%s, calldata_ptr null.", __func__);
        return;
    }

    egress_leg_ptr = IBCF_EGRESS_LEG_PTR(calldata_ptr);

    if (IS_VALID_PTR(egress_leg_ptr) == FALSE)
    {
        IMS_DLOG(IMS_IBCF_LOGHIGH, "%s, egress_leg_ptr null.", __func__);
        return;
    }

    egress_tg_data_ptr = IBCF_get_tg_data_by_id(IBCF_get_vn_id_from_leg(egress_leg_ptr),
                                                IBCF_get_tg_id_from_leg(egress_leg_ptr));

    if (IS_VALID_PTR(egress_tg_data_ptr) == FALSE)
    {
        IMS_DLOG(IMS_IBCF_LOGHIGH, "%s, egress_tg_data_ptr null.", __func__);
        return;
    }

    // SIP-I phase1 did not consider the ACC (Automatic Collect Call) M bit.
    memcpy(&npdi, &p_SipConEvntPtr->npdi, sizeof(TknU8));
    IMS_DLOG(IMS_IBCF_LOGLOWEST, "%s, print p_SipConEvntPtr->npdi info:pres is %d, val is %d",
             __func__, p_SipConEvntPtr->npdi.pres, p_SipConEvntPtr->npdi.val);
    IMS_DLOG(IMS_IBCF_LOGLOWEST, "%s, print npdi info:pres is %d, val is %d", __func__, npdi.pres,
             npdi.val);
    // npdi[sizeof(TknU8)] = '\0';
    // maybe above is not needed, for TknU8 is a strucutre

    // Get URI from R-URI.
    req_uri_ptr = SIPia_Msg_getReq_URI(sip_msg_ptr);

    if (IS_VALID_PTR(req_uri_ptr) == FALSE)
    {
        IMS_DLOG(IMS_IBCF_LOGHIGH, "%s, req_uri_ptr invalid pointer.", __func__);
        return;
    }

    switch (URI_getType(req_uri_ptr))
    {
    case TEL_URI:
    {
        teluri_ptr = URI_getTelUri(req_uri_ptr);

        if (IS_VALID_PTR(teluri_ptr) == FALSE)
        {
            IMS_DLOG(IMS_IBCF_LOGHIGH, "%s, teluri_ptr invalid pointer.", __func__);
            return;
        }

        param_list = Tel_URI_getOtherParameter(teluri_ptr);

        if (IS_VALID_PTR(param_list) == FALSE)
        {
            IMS_DLOG(IMS_IBCF_LOGHIGH, "%s, Other parameter list in tel uri is invalid.", __func__);
            return;
        }

        // If egress TG is SIPI TG, then IAM is encapsulated into INVITE MIME
        // if either egress or ingress TG is SIPI TG,
        // R-URI's called party DN should be gotten from GCC, for GCC already
        // got the idea of what called DN in R-URI should be
        sipi_profile_id = egress_tg_data_ptr->sipi_profile_id;

        IMS_DLOG(IMS_IBCF_LOGLOWEST,
                 "%s, p_SipConEvntPtr->reqUri.reqUriVal.val(i.e userPart) is %s", __func__,
                 p_SipConEvntPtr->reqUri.reqUriVal.val);
        // maybe should add a check here to see if replacement is good.
        BString *uri_raw_text =
            SIPUtil_setURI_user(sip_msg_ptr, req_uri_ptr,
                                reinterpret_cast<char *>(p_SipConEvntPtr->reqUri.reqUriVal.val),
                                p_SipConEvntPtr->reqUri.reqUriVal.len);
        SIPia_InvalidateRawText(sip_msg_ptr, TOP_LINE);
        BSTRING_FREE(uri_raw_text);

        if ((p_SipConEvntPtr->rn.eh.pres == PRSNT_NODEF) &&
            (p_SipConEvntPtr->rn.rnVal.pres == PRSNT_NODEF))
        {
            IMS_DLOG(IMS_IBCF_LOGLOW, "%s: need to build rn", __func__);
            DString_set(&teluri_ptr->routingNumber,
                        reinterpret_cast<char *>(p_SipConEvntPtr->rn.rnVal.val),
                        static_cast<int>(p_SipConEvntPtr->rn.rnVal.len));
            SIPia_InvalidateRawText(sip_msg_ptr, TOP_LINE);
        }
        else
        {
            IMS_DLOG(IMS_IBCF_LOGLOW, "%s: remove rn", __func__);
            DString_reset(&teluri_ptr->routingNumber);
            SIPia_InvalidateRawText(sip_msg_ptr, TOP_LINE);
        }

        if ((npdi.pres == PRSNT_NODEF) && (npdi.val != 0))
        {
            IMS_DLOG(IMS_IBCF_LOGLOWEST, "%s,build npdi in TEL URI.", __func__);
            teluri_ptr->npdi = TRUE;
            // standard and SIP stack only support npdi in tel uri
            // npdi=yes is not supported
            SIPia_InvalidateRawText(sip_msg_ptr, TOP_LINE);
        }
        else
        {
            IMS_DLOG(IMS_IBCF_LOGLOWEST, "%s: remove npdi from TEL URI", __func__);
            SIPia_GenParam_deleteParam(sip_msg_ptr, param_list, "npdi", strlen("npdi"));
            SIPia_InvalidateRawText(sip_msg_ptr, TOP_LINE);
        }

        if ((p_SipConEvntPtr->cic.eh.pres == PRSNT_NODEF) &&
            (p_SipConEvntPtr->cic.cicVal.pres == PRSNT_NODEF))
        {
            IMS_DLOG(IMS_IBCF_LOGLOWEST, "%s, TEL URI, build CIC", __func__);
            DString *tel_cic = Tel_URI_getCIC(teluri_ptr);

            if (IS_VALID_PTR(tel_cic) == FALSE)
            {
                IMS_DLOG(IMS_LOGHIGH, "%s: invalid tel_cic pointer\n", __func__);
                return;
            }
            if (strstr((char *)tel_cic, "cic=") == NULL)
            {
                const int tel_cic_len               = sizeof(p_SipConEvntPtr->cic.cicVal.val);
                char      tel_cic_text[tel_cic_len] = {0};
                strlcpy(tel_cic_text, reinterpret_cast<char *>(p_SipConEvntPtr->cic.cicVal.val),
                        p_SipConEvntPtr->cic.cicVal.len + 1);
                IMS_DLOG(IMS_IBCF_LOGLOW, "%s, TEL URI, tel_cic_text is %s", __func__,
                         tel_cic_text);

                DString_copy(sip_msg_ptr, tel_cic, &tel_cic_text[0], strlen(tel_cic_text), 0);
            }

            if (sipi_profile_id > 0)
            {
                IBCF_SIPI_PROFILE_DATA *sipi_data_ptr =
                    IBCF_get_sipi_profile_data_by_id(sipi_profile_id);

                if (IS_VALID_PTR(sipi_data_ptr) == FALSE)
                {
                    IMS_DLOG(IMS_LOGHIGH, "%s: invalid sipi_data_ptr:\n", __func__);
                    return;
                }

                // populate dai related info
                if (sipi_data_ptr->db_data.dai)
                {
                    if (p_SipConEvntPtr->dai.pres)
                    {
                        IMS_DLOG(IMS_IBCF_LOGLOWEST, "%s, need to add dai in tel uri.", __func__);
                        /* Add "dai" to other parameters list.*/

                        int   dai_len;
                        char *dai_ptr = NULL_PTR;
                        if (IS_VALID_PTR(dai_ptr = SIPia_GenParam_getParamToken(
                                             sip_msg_ptr, param_list, &dai_len, "dai",
                                             strlen("dai"))) == TRUE)
                        {
                            IMS_DLOG(IMS_IBCF_LOGLOWEST, "%s, tel_uri has dai,delete it", __func__);
                            // delte npdi, then add dai again based on the p_SipConEvntPtr->dai.val
                            SIPia_GenParam_deleteParam(sip_msg_ptr, param_list, "dai",
                                                       strlen("dai"));
                            SIPia_InvalidateRawText(sip_msg_ptr, TOP_LINE);
                        }

                        switch (p_SipConEvntPtr->dai.val)
                        {
                        case SIP_DAI_NOIND:

                            strcpy(dai, "dai=no-ind");
                            break;
                        case SIP_DAI_PRESUB:

                            strcpy(dai, "dai=presub");
                            break;
                        case SIP_DAI_PRESUB_DA:

                            strcpy(dai, "dai=presub-da");
                            break;
                        case SIP_DAI_PRESUB_DAUNKWN:

                            strcpy(dai, "dai=presub-da-unkwn");
                            break;
                        case SIP_DAI_DA:

                            strcpy(dai, "dai=da");
                            break;
                        default:

                            strcpy(dai, "dai=no-ind");
                            break;
                        }
                        IMS_DLOG(IMS_IBCF_LOGLOW, "%s, tel uri: dai is %s", __func__, dai);

                        if ((tel_dai = DString_new(sip_msg_ptr, dai, COMPUTE_STRING_LENGTH, 0)) ==
                            NULL)
                        {
                            IMS_DLOG(IMS_IBCF_LOGHIGH,
                                     "%s, failed to create dstring for dai in tel uri.", __func__);
                            return;
                        }

                        SIPia_List_add(sip_msg_ptr, param_list, (void *)tel_dai, LIST_END);
                        SIPia_InvalidateRawText(sip_msg_ptr, TOP_LINE);

                        IMS_DLOG(IMS_IBCF_LOGLOW, "%s, successfully add dai in tel uri.", __func__);
                    }
                }
                else
                {
                    IMS_DLOG(IMS_IBCF_LOGLOWEST, "%s: remove dai from TEL URI", __func__);
                    SIPia_GenParam_deleteParam(sip_msg_ptr, param_list, "dai", strlen("dai"));
                    SIPia_InvalidateRawText(sip_msg_ptr, TOP_LINE);
                }
            }
        }
        else
        {
            IMS_DLOG(IMS_IBCF_LOGLOWEST, "%s: remove cic from TEL URI", __func__);
            SIPia_GenParam_deleteParam(sip_msg_ptr, param_list, "cic", strlen("cic"));
            SIPia_InvalidateRawText(sip_msg_ptr, TOP_LINE);
        }
    }
    break;

    case SIP_URI:
    case SIPS_URI:
    {
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

        std::string usr_name_string(DString_getStr(ds_user_name), DString_getLen(ds_user_name));

        // If egress TG is SIPI TG, then IAM is encapsulated into INVITE MIME
        // if either egress or ingress TG is SIPI TG,
        // R-URI's called party DN should be gotten from GCC, for GCC already
        // got the idea of what called DN in R-URI should be
        sipi_profile_id = egress_tg_data_ptr->sipi_profile_id;

        IMS_DLOG(IMS_IBCF_LOGLOWEST, "%s, before replace, user_name=[%s].", __func__,
                 usr_name_string.c_str());

        // need to replace reqUriVal for user_name
        // e.g. if the top line of the sip message is:
        // INVITE sip:+1605088031111;rn=5081709123;npdi@10.84.2.13:50003;transport=udp SIP/2.0
        // then, user_name will be +1605088031111;rn=5081709123;npdi
        while (1)
        {
            // if there reqUri len is zero from sipw p_SipConEvntPtr,
            // will keep the existing reqUri in the message, from the
            // above reqUriVal could be +1605088031111, or 12346789 e.g.
            if (p_SipConEvntPtr->reqUri.reqUriVal.len == 0)
            {
                IMS_DLOG(IMS_IBCF_LOGLOWEST, "%s, reqUriVal.len == 0, keep exisitng", __func__);
                break;
            }

            std::string new_usr_string;
            new_usr_string.assign(reinterpret_cast<char *>(p_SipConEvntPtr->reqUri.reqUriVal.val),
                                  p_SipConEvntPtr->reqUri.reqUriVal.len);

            // the if means the user_name e.g. is +1605088031111 or 1605088031111,
            // there is no other parameters in the user_name
            if (usr_name_string.find(';') == std::string::npos)
            {
                usr_name_string = new_usr_string;
                break;
            }

            // means there is parameter in user_name, such as
            // +1605088031111;rn=172727272;npdi;****, just replace
            // +1605088031111 to the reqUriVal from sipw
            usr_name_string = usr_name_string.replace(0, usr_name_string.find(';'), new_usr_string);

            break;
        }

        IMS_DLOG(IMS_IBCF_LOGLOWEST, "%s, after replace number, user_name[%s].", __func__,
                 usr_name_string.c_str());

        // trying to find if rn is needed
        while (1)
        {
            std::string            new_rn_string;
            std::string::size_type rn_pos = usr_name_string.find(";rn=");

            IMS_DLOG(IMS_IBCF_LOGLOWEST, "%s, before replace rn, user_name[%s].", __func__,
                     usr_name_string.c_str());
            // need to remove rn if existing in R-URI
            if (rn_pos != std::string::npos)
            {
                std::string rn_string =
                    string_find_value(usr_name_string, ";rn=", 5, '>', ' ', '@', ';', '\0');

                usr_name_string = usr_name_string.replace(usr_name_string.find(";rn="),
                                                          strlen(";rn=") + rn_string.length(), "");

                IMS_DLOG(IMS_IBCF_LOGLOWEST, "%s, after replace rn, user_name[%s].", __func__,
                         usr_name_string.c_str());
            }

            if (!((p_SipConEvntPtr->rn.eh.pres == PRSNT_NODEF) &&
                  (p_SipConEvntPtr->rn.rnVal.pres == PRSNT_NODEF)))
            {
                IMS_DLOG(IMS_IBCF_LOGLOWEST, "%s, return: rn.eh.pres[%d], rn.rnVal.pres[%d].",
                         __func__, p_SipConEvntPtr->rn.eh.pres, p_SipConEvntPtr->rn.rnVal.pres);
                break;
            }

            new_rn_string.assign(reinterpret_cast<char *>(p_SipConEvntPtr->rn.rnVal.val),
                                 p_SipConEvntPtr->rn.rnVal.len);
            if (new_rn_string.length() == 0)
            {
                IMS_DLOG(IMS_IBCF_LOGLOWEST, "%s, return: rn.rnVal.len == 0", __func__);
                break;
            }

            // need to add rn
            new_rn_string   = ";rn=" + new_rn_string;
            usr_name_string = usr_name_string + new_rn_string;

            IMS_DLOG(IMS_IBCF_LOGLOWEST, "%s, after add rn, user_name[%s].", __func__,
                     usr_name_string.c_str());

            break;
        }

        IMS_DLOG(IMS_IBCF_LOGLOWEST, "%s, after replace rn, user_name[%s].", __func__,
                 usr_name_string.c_str());

        // try to check if npdi is needed!
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

            param_list = Sip_URI_getOtherParameter(sipuri_ptr);

            if (IS_VALID_PTR(param_list) == TRUE)
            {
                IMS_DLOG(IMS_IBCF_LOGLOW, "%s: find other parameter list in sip uri.", __func__);

                int   npdi_len;
                char *npdi_ptr = NULL_PTR;
                if (SIPia_GenParam_searchParam(param_list, "npdi", strlen("npdi")) != FALSE)
                {
                    IMS_DLOG(IMS_IBCF_LOGLOWEST, "%s, sip_uri has npdi, delete it", __func__);

                    SIPia_GenParam_deleteParam(sip_msg_ptr, param_list, "npdi", strlen("npdi"));
                }
            }

            // need to remove npdi
            if ((npdi.pres != PRSNT_NODEF) || (npdi.val == 0))
            {
                break;
            }

            // need to add npdi=yes
            usr_name_string = usr_name_string.append(";npdi");
            break;
        }

        IMS_DLOG(IMS_IBCF_LOGLOWEST, "%s, after replace npdi, user_name[%s].", __func__,
                 usr_name_string.c_str());

        if ((p_SipConEvntPtr->cic.eh.pres == PRSNT_NODEF) &&
            (p_SipConEvntPtr->cic.cicVal.pres == PRSNT_NODEF))
        {
            IMS_DLOG(IMS_IBCF_LOGLOWEST, "%s, build cic in SIP/SIPS URI.", __func__);
            DString *sip_userName_cic = Sip_URI_getUserName(sipuri_ptr);
            if (IS_VALID_PTR(sip_userName_cic) == FALSE)
            {
                IMS_DLOG(IMS_IBCF_LOGHIGH,
                         "%s, when building cic, sip_userName_cic pointer is invalid.", __func__);
                return;
            }

            if ((p_SipConEvntPtr->cic.eh.pres == PRSNT_NODEF) &&
                (p_SipConEvntPtr->cic.cicVal.pres == PRSNT_NODEF))
            {
                IMS_DLOG(IMS_IBCF_LOGLOWEST, "%s, build cic in SIP/SIPS URI.", __func__);
                if (usr_name_string.find(";cic=") != std::string::npos)
                {
                    IMS_DLOG(IMS_IBCF_LOGLOWEST, "%s, r-uri userName has cic in sip uri.",
                             __func__);

                    int cic_pos;
                    cic_pos = usr_name_string.find(";cic=", 0);
                    // cic value may be followed by semicolon or at
                    int cic_valueend_pos;
                    if ((cic_valueend_pos = usr_name_string.find(";", cic_pos)) !=
                        std::string::npos)
                    {
                        // cic is followed by semicolon
                        IMS_DLOG(IMS_IBCF_LOGLOWEST, "%s, cic in sip uri is followed by semicolon.",
                                 __func__);
                    }
                    else
                    {
                        IMS_DLOG(IMS_IBCF_LOGLOWEST, "%s, cic is the end of the userName.",
                                 __func__);
                        cic_valueend_pos = usr_name_string.length();
                    }

                    IMS_DLOG(IMS_IBCF_LOGLOWEST, "%s, cic_pos is %d, cic_valueend_pos is %d",
                             __func__, cic_pos, cic_valueend_pos);

                    usr_name_string.replace(
                        cic_pos + 5, (cic_valueend_pos - cic_pos - 5),
                        reinterpret_cast<char *>(p_SipConEvntPtr->cic.cicVal.val),
                        p_SipConEvntPtr->cic.cicVal.len);
                }
                else
                {
                    // if there is no cic= in r-uri raw text, add cic into username.
                    IMS_DLOG(IMS_LOGLOWEST, "%s: no cic in sip uri user name:\n", __func__);

                    usr_name_string.append(
                        reinterpret_cast<char *>(p_SipConEvntPtr->cic.cicVal.val),
                        p_SipConEvntPtr->cic.cicVal.len);
                }
                // populate dai related info
                IBCF_SIPI_PROFILE_DATA *sipi_data_ptr =
                    IBCF_get_sipi_profile_data_by_id(sipi_profile_id);

                if (IS_VALID_PTR(sipi_data_ptr) == FALSE)
                {
                    IMS_DLOG(IMS_LOGHIGH, "%s: invalid sipi_data_ptr:\n", __func__);
                    return;
                }

                if (sipi_data_ptr->db_data.dai)
                {
                    IMS_DLOG(IMS_LOGLOWEST, "%s: egress tg dai is set for sip uri\n", __func__);
                    if (p_SipConEvntPtr->dai.pres)
                    {
                        IMS_DLOG(IMS_LOGLOWEST, "%s: dai value is sip uri: %d\n", __func__,
                                 p_SipConEvntPtr->dai.val);

                        // if TOP LINE already has dai, replace it.
                        if (usr_name_string.find(";dai=") != std::string::npos)
                        {
                            IMS_DLOG(IMS_IBCF_LOGLOWEST, "%s, r-uri user name has dai in sip uri.",
                                     __func__);

                            int dai_pos;
                            dai_pos = usr_name_string.find(";dai=", 0);
                            // dai value may be followed by semicolon or at
                            int dai_valueend_pos;
                            if ((dai_valueend_pos = usr_name_string.find(";", dai_pos)) != -1)
                            {
                                IMS_DLOG(IMS_IBCF_LOGLOWEST,
                                         "%s, dai in sip uri is followed by semicolon.", __func__);
                            }
                            else
                            {
                                IMS_DLOG(IMS_IBCF_LOGLOWEST,
                                         "%s, dai in sip uri is the end of the username.",
                                         __func__);
                                dai_valueend_pos = usr_name_string.length();
                            }

                            IMS_DLOG(IMS_IBCF_LOGLOWEST,
                                     "%s, dai_pos is %d, dai_valueend_pos is %d", __func__, dai_pos,
                                     dai_valueend_pos);

                            switch (p_SipConEvntPtr->dai.val)
                            {
                            case SIP_DAI_NOIND:

                                usr_name_string.replace(dai_pos + 5,
                                                        (dai_valueend_pos - dai_pos - 5), "no-ind",
                                                        strlen("no-ind"));
                                break;
                            case SIP_DAI_PRESUB:

                                usr_name_string.replace(dai_pos + 5,
                                                        (dai_valueend_pos - dai_pos - 5), "presub",
                                                        strlen("presub"));
                                break;
                            case SIP_DAI_PRESUB_DA:

                                usr_name_string.replace(dai_pos + 5,
                                                        (dai_valueend_pos - dai_pos - 5),
                                                        "presub-da", strlen("presub-da"));
                                break;
                            case SIP_DAI_PRESUB_DAUNKWN:

                                usr_name_string.replace(
                                    dai_pos + 5, (dai_valueend_pos - dai_pos - 5),
                                    "presub-da-unkwn", strlen("presub-da-unkwn"));
                                break;
                            case SIP_DAI_DA:

                                usr_name_string.replace(dai_pos + 5,
                                                        (dai_valueend_pos - dai_pos - 5), "da",
                                                        strlen("da"));
                                break;
                            default:

                                usr_name_string.replace(dai_pos + 5,
                                                        (dai_valueend_pos - dai_pos - 5), "no-ind",
                                                        strlen("no-ind"));
                                break;
                            }
                        }
                        else
                        {
                            IMS_DLOG(IMS_IBCF_LOGLOWEST,
                                     "%s, r-uri user name has no dai in sip uri.", __func__);

                            switch (p_SipConEvntPtr->dai.val)
                            {
                            case SIP_DAI_NOIND:
                                usr_name_string.append(";dai=no-ind");
                                break;

                            case SIP_DAI_PRESUB:
                                usr_name_string.append(";dai=presub");
                                break;

                            case SIP_DAI_PRESUB_DA:
                                usr_name_string.append(";dai=presub-da");
                                break;

                            case SIP_DAI_PRESUB_DAUNKWN:
                                usr_name_string.append(";dai=presub-da-unkwn");
                                break;

                            case SIP_DAI_DA:
                                usr_name_string.append(";dai=da");
                                break;

                            default:
                                usr_name_string.append(";dai=no-ind");
                                break;
                            }
                        }
                    }
                }
            }
        }
        else
        {
            if (usr_name_string.find(";cic=") != std::string::npos)
            {
                IMS_DLOG(IMS_IBCF_LOGLOWEST, "%s: remove cic from TEL URI", __func__);

                std::string cic_string =
                    string_find_value(usr_name_string, ";cic=", 5, '>', ' ', '@', ';', '\0');
                usr_name_string.replace(usr_name_string.find(";cic="),
                                        strlen(";cic=") + cic_string.length(), "");
            }

            if (usr_name_string.find(";dai=") != std::string::npos)
            {
                IMS_DLOG(IMS_IBCF_LOGLOWEST, "%s: remove dai from TEL URI", __func__);

                std::string dai_string =
                    string_find_value(usr_name_string, ";dai=", 5, '>', ' ', '@', ';', '\0');
                usr_name_string.replace(usr_name_string.find(";dai="),
                                        strlen(";dai=") + dai_string.length(), "");
            }
        }

        IMS_DLOG(IMS_IBCF_LOGLOW, "%s: username after updating is [%s]", __func__,
                 usr_name_string.c_str());
        if (Sip_URI_setUserName(sip_msg_ptr, sipuri_ptr, usr_name_string.c_str(),
                                usr_name_string.length()) != SIPIA_SUCCESS)
        {
            IMS_DLOG(IMS_IBCF_LOGHIGH, "%s: failed to update username in SIP/SIPS URI", __func__);
            return;
        }
        break;
    }
    }
}

/*
 * Function:    IBCFb2b_iw_build_rsc_priority()
 *
 * Description: This function build the Resource-Priority header, populate the resource_priority
 *              buffer 
 *              Attention: this function follows the previous MGC-8 parameter scope, which avoid the change of 
 *              calling functions. while since currently MGC calling functions use 132 as the resource_priority
 *              size, so a new MACRO MAX_RSC_PRIORITY_SIZE is created to hold this number, the calling functions
 *              should assure the size for resource_priority parameter is at least 132
 *
 * Parameters:  rscPriorityPtr    - Pointer to SipRscPriority.
 *              resource_priority - Buffer to hold the resource priority, size at least MAX_RSC_PRIORITY_SIZE.          
 *
 * Return:      void
 */
 void IBCFb2b_iw_build_rsc_priority(SipRscPriority *rscPriorityPtr, char *resource_priority)
 {
 	IMS_DLOG(IMS_IBCF_LOGLOW, "%s: Entered", __func__);

	if ((IS_VALID_PTR(rscPriorityPtr) == FALSE) || (IS_VALID_PTR(resource_priority) == FALSE))
	{
		ASRTA_S(ASRTBADPARAM, "rscPriorityPtr=[%p], resource_priority=[%p]", 
			rscPriorityPtr, resource_priority);
		return;
	}
 	    					            	
	IMS_DLOG(IMS_IBCF_LOGLOW, "%s: eh.pres=[%d], ets.pres=[%d], ets.val=[%d], wps.pres=[%d], wps.val=[%d], \ 
		RscPriorityHdr.pres=[%d], RscPriorityHdr.val=[%s]", __func__, rscPriorityPtr->eh.pres, rscPriorityPtr->ets.pres,
		rscPriorityPtr->ets.val, rscPriorityPtr->wps.pres, rscPriorityPtr->wps.val,
		rscPriorityPtr->RscPriorityHdr.pres, reinterpret_cast<char *>(rscPriorityPtr->RscPriorityHdr.val)
		);

	if (rscPriorityPtr->eh.pres == PRSNT_NODEF)
	{
 		char tmpStr[16];
		resource_priority[0] = '\0';

		if (rscPriorityPtr->ets.pres == PRSNT_NODEF &&
			rscPriorityPtr->RscPriorityHdr.pres == PRSNT_NODEF &&
			(strcasestr(reinterpret_cast<char *>(rscPriorityPtr->RscPriorityHdr.val), "ets.") == NULL))
		{
			/* 
			 * If raw RPH from opposite leg does not contain ets.X string,
			 * build the RPH string using binary ets and wps r-values.
			 */
			 snprintf(resource_priority, MAX_RSC_PRIORITY_SIZE, "ets.%d", rscPriorityPtr->ets.val);
			 if (rscPriorityPtr->wps.pres == PRSNT_NODEF)
			 {
				snprintf(tmpStr, sizeof(tmpStr), ", wps.%d", rscPriorityPtr->wps.val);
				strlcat(resource_priority, tmpStr, MAX_RSC_PRIORITY_SIZE);
			 }
		}
		else if (rscPriorityPtr->RscPriorityHdr.pres == PRSNT_NODEF)
		{
			/* Use raw RPH string that came in from far leg */
			strlcpy(resource_priority, reinterpret_cast<char *>(rscPriorityPtr->RscPriorityHdr.val), 
				MAX_RSC_PRIORITY_SIZE);
		}
		else
		{
			/* Build the RPH string using the ets and wps r-values (wps r-value cannot appear without ets)*/
			if (rscPriorityPtr->ets.pres == PRSNT_NODEF)
			{
				snprintf(resource_priority, MAX_RSC_PRIORITY_SIZE, "ets.%d", rscPriorityPtr->ets.val);
				if (rscPriorityPtr->wps.pres == PRSNT_NODEF)
				{
					snprintf(tmpStr, sizeof(tmpStr), ", wps.%d", rscPriorityPtr->wps.val);
					strlcat(resource_priority, tmpStr, MAX_RSC_PRIORITY_SIZE);
				}
			}
		}
		IMS_DLOG(IMS_IBCF_LOGLOW, "%s: Built Resource-Priority Hdr: %s", __func__, resource_priority);
	}
}

/**PROC+**********************************************************************/
/* Name:         IBCF_build_diversion                                           */
/*                                                                           */
/* Purpose:                                                                  */
/*                                                                           */
/* Returns:   Nothing.                                                       */
/*                                                                           */
/* Params:                                                                   */
/*                                                                           */
/* Operation:                                                                */
/* Stardard format Diversion header                                          */
/*                                                                           */
/**PROC-**********************************************************************/

IMS_RETVAL IBCFb2b_iw_build_diversion (SipwLiConEvnt *p_conEvntPtr,B2BUAEvent *event_ptr,
                               bool notification)
{
    const char *           fed_ext_name_ptr = NULL_PTR;
    SIPia_List_Cursor *    cursor           = NULL_PTR;
    SIP_OtherHeader *      current_obj      = NULL_PTR;
    SIPia_List *           pOtherList       = NULL_PTR;
    IBCF_TRUNK_GROUP_DATA *tg_data_ptr      = NULL_PTR;
    SIPA_NUM_TYPE          numType          = SIPA_NUMTYPE_UKNWN;
    int                    fed_ext_name_len = 0;
    int                    i                = 0;
    char                   tmpStr[16]       = {0};
    IBCF_VN_TG_ID          tg_info;

    if (!IS_VALID_PTR(event_ptr) || !IS_VALID_PTR(p_conEvntPtr) || !IS_VALID_PTR(event_ptr->m_ptr))
    {
		IMS_DLOG(IMS_IBCF_LOGHIGH, "%s: Invalid input parameters", __func__);
        return IMS_FAIL;
    }

    SIPia_Msg *sipiaMsg_ptr = event_ptr->m_ptr;
    
    IBCFget_tg_info_from_leg(event_ptr->leg_ptr, &tg_info);
    tg_data_ptr = IBCF_get_tg_data_by_id(tg_info.vn_id, tg_info.tg_id);
    if (IS_VALID_PTR(tg_data_ptr) == FALSE)
    {
        IMS_DLOG(IMS_IBCF_LOGLOW, "%s(), Could not find trunk group data", __func__);
        return IMS_FAIL;
    }

    IBCF_SIP_PROFILE_DATA *sip_profile_ptr =
        IBCF_get_sip_profile_data_by_id(tg_data_ptr->sip_profile_id);
    if (IS_VALID_PTR(sip_profile_ptr) == FALSE)
    {
        IMS_DLOG(IMS_IBCF_LOGLOW, "%s(), Could not find sip profile data", __func__);
        return IMS_SUCCESS;
    }

    SIPIA_PORT_IDX sip_port_idx = B2BCutil_get_sipia_port_idx(event_ptr->leg_ptr);
    if (IS_VALID_SIPIA_PORT_IDX(sip_port_idx) == FALSE)
    {
        IMS_DLOG(IMS_IBCF_LOGHIGH, "%s(), invalid sip_port_idx=[%u] from SIP message type (request or response)=[%d].",
            __func__, sip_port_idx, SIPia_getMsgType(event_ptr->m_ptr));
        return IMS_SUCCESS;
    }

    NGSS_PORT_INDX port_idx = SSgul_get_sipia_port_prov_key(sip_port_idx);
    if (port_idx == NGSS_DEFAULT_INDX)
    {
        IMS_DLOG(IMS_IBCF_LOGHIGH, "%s(), invalid port_idx=[%d].", __func__, port_idx);
        return IMS_SUCCESS;
    }

    const IBCF_VIRTUAL_NETWORK_DATA *vn_data_ptr = IBCF_get_vn_data_by_sip_msg(event_ptr->m_ptr);
    if (!IS_VALID_PTR(vn_data_ptr))
    {
        IMS_DLOG(IMS_IBCF_LOGHIGH, "%s(), Couldn't find VN data through SIP messsage.", __func__);
        return IMS_SUCCESS;
    }

    if (vn_data_ptr->vn_type == IBCF_VIRTUAL_NETWORK_CORE)
    {
        fed_ext_name_ptr =
            IMScfed_get_core_fed_port_ext_host_name(port_idx, &fed_ext_name_len, event_ptr->m_ptr);
    }
    else
    {
        fed_ext_name_ptr = IMSpfed_get_peer_fed_port_ext_host_name(port_idx, event_ptr->m_ptr);
    }

    unsigned long uriType      = convertImsUri2IsupUri(sip_profile_ptr->uri_type);
    SipDiversion  div          = p_conEvntPtr->div;
    char          top_div[128] = {0};
    char          bot_div[128] = {0};

    if ( ((div.botDivType == SIPA_USER_PART) || (div.topDivType == SIPA_USER_PART)) && (uriType == SIP_TOKEN_TEL) )
    {
        IMS_DLOG(IMS_IBCF_LOGHIGH, "%s: invaild DivType ", __FUNCTION__);
        return IMS_SUCCESS;
    }
#if 0

    bzero(&srcAddrStr, sizeof(srcAddrStr));
    /* Gary FID 17602 use lpoc IP */
    if(SIPA_CHECK_SFW_EXIST(ip2TrnkPtr))
    {
       srcAddrPtr = sipaGetLpocIPStr(ip2TrnkPtr, &lpocPort);
       if (srcAddrPtr)
       {
          OS_SPRINTF (srcAddrStr, "%s", srcAddrPtr);
       }
       else
       {
          SIPA_TRACE (NBB_ERROR_TRC, ("%s: Invalid LPOC IP address  \n", __FUNCTION__));
          return;
       }
    }
    else
    {
#ifndef CLOUD
        sipaSetIPStrFromLocal(srcAddrStr, ipVersion, sipInterface);
#else
        /* FID 17120.0 Outgoing sip headers should have SFED IP */
        SIPA_TRACE(NBB_DETAIL_TRC, ("%s: Populate top_Div header with SFED's address, intfc %d, ipv %d\n",
                                         __FUNCTION__, sipInterface, ipVersion));
        sipa_set_header_with_sfedip (ipVersion, sipInterface, srcAddrStr);
#endif
    }

    /* Need to handle the case of only bottom-most diversion: (bug 36745) */
#endif // end of #if 0
	pOtherList = reinterpret_cast<SIPia_List *>(SIPia_getHeaderContainer(sipiaMsg_ptr, SIPIA_OTHER_HEADER));
	if(IS_VALID_PTR(pOtherList) == TRUE)
	{
		IMS_DLOG(IMS_IBCF_LOGLOWEST, "%s: Other header list found.", __func__);
		SIPia_List_cursor_init(pOtherList, &cursor);
		while(IS_VALID_PTR(cursor) == TRUE)
		{
			current_obj = reinterpret_cast<SIP_OtherHeader *>(SIPia_List_traverse(&cursor));
			if((DString_getLen(&current_obj->headerName) == IMS_HEADER_DIVERSION_LEN) &&
				(SIPstrcasecmp(DString_getStr(&current_obj->headerName), IMS_HEADER_DIVERSION, IMS_HEADER_DIVERSION_LEN) == TRUE))
			{
				 return IMS_SUCCESS;
			}
		}
	}

    if (div.topDivNum.pres == PRSNT_NODEF)
    {

        /* Build the top diversion string */

        strcpy(top_div, "<");
        switch (uriType)
        {
        case SIP_TOKEN_TEL:
            strcat(top_div, "tel:");
            break;
        case SIP_TOKEN_SIP:
            strcat(top_div, "sip:");
            break;
        /* FID 17021 URN service */
        case SIP_TOKEN_URN:
            strcat(top_div, "urn:");
            break;
        /* FID17364.0 SIP Over TLS */
        case SIP_TOKEN_SIPS:
            strcat(top_div, "sips:");
            break;
        default:
            IMS_DLOG(IMS_IBCF_LOGHIGH, "%s:Unsupported uri scheme:%ld, treated as sip",
                                __FUNCTION__, uriType);

            strcpy(top_div, "sip:");
            break;
        }

        if (IBCF_add_tel_num(top_div, reinterpret_cast<char *>(div.topDivNum.val),div.topDivNum.len, &numType, uriType) == 0)
        {
                return IMS_SUCCESS;
        }
        /* FID17364.0 SIP over TLS */
        if ((uriType == SIP_TOKEN_SIP) || (uriType == SIP_TOKEN_SIPS) || (uriType == SIP_TOKEN_TEL) )
        {
            strcat(top_div, "@");
            strcat(top_div, fed_ext_name_ptr);   
	}

        /* Require user=phone for SIP uri */
        /* FID17364.0 SIP over TLS */
        if (((uriType == SIP_TOKEN_SIP) || (uriType == SIP_TOKEN_SIPS)) &&
             (div.topDivType == SIPA_TELE_SUB_PART) )
        {
            strcat(top_div, ";user=phone");
        }

        strcat(top_div, ">");

        if (notification)
        {
           /*
            * If the topDivCause present and the value is 302,
            * and the peerHistInfoFmt is TS29163.746
            * Change the deflection to unconditional per the requriment.
            */

           if( (div.topDivCause.pres) && (div.peerHistInfoFmt.pres) &&
               (div.peerHistInfoFmt.val == HISTINFOFMT_29163_746) &&
               (div.topDivReason.val == SIPDIV_RSN_CODE_DEFLECTION) &&
               (div.topDivCause.val == 302) )
           {
              div.topDivReason.val = SIPDIV_RSN_CODE_UNCONDITIONAL;
              IMS_DLOG (IMS_IBCF_LOGLOW, "%s: deflection.unconditional\n", __FUNCTION__);
           }

           if (div.topDivReason.pres)
           {
              strcat(top_div, ";reason=");
              for (i = 0; i < MAX_HI_REASONS; i++)
              {
                if (div.topDivReason.val == divReasonVal2Str[i].reasonVal )
                {
                   strcat(top_div, divReasonVal2Str[i].reasonStr);
                   break;
                }
              }
              /* if we could not find reason string set to default "unknown" */
              if (i == MAX_HI_REASONS)
              {
                   strcat(top_div, "unknown");
              }
           }
        }

        else if(div.topDivReason.pres)
        {
            strcat(top_div, ";reason=");
            strcat(top_div, reasonStr[div.topDivReason.val]);
        }
        else
        {
            strcat(top_div, ";reason=");
            strcat(top_div, "unknown");
        }

        if (div.count.pres  == PRSNT_NODEF)
        {
            strcat(top_div, ";counter=");
            IMS_DLOG(IMS_IBCF_LOGLOW,"diversion count's value:%d", div.count.val);
            strcat(top_div, reinterpret_cast<char *>(&(div.count.val)));
        }

        if (div.topPrivacy.pres == PRSNT_NODEF)
        {
           if (notification == FALSE)
           {
            strcat(top_div, ";privacy=\"");
            if (div.topPrivacy.val == SIPDIV_PRIVACY_FULL)
                strcat(top_div, "full");
            else if (div.topPrivacy.val == SIPDIV_PRIVACY_NAME)
                strcat(top_div, "name");
            else if (div.topPrivacy.val == SIPDIV_PRIVACY_URI)
                strcat(top_div, "uri");
            else /* default SIPDIV_PRIVACY_OFF */
                strcat(top_div, "off");

            /* add '" ' in last */
            strcat(top_div, "\"");
          }
          else
          {
            strcat(top_div, ";notification=\"");
            if (div.topPrivacy.val == SIPDIV_PRIVACY_OFF)
            {
              strcat(top_div, "presentation allowed\"");
            }
            else
            {
              strcat(top_div, "presentation not allowed\"");
            }
          }
        }

        if (div.topScreen.pres == PRSNT_NODEF)
        {
            strcat(top_div, ";screen=\"");
            if (div.topScreen.val == SIPDIV_SCREEN_YES)
                strcat(top_div, "yes");
            else /* default SIPDIV_SCREEN_NO */
                strcat(top_div, "no");

            /* add '" ' in last */
            strcat(top_div, "\"");
        }
    }
    /* 16661.0 no redirection number, only add SIP uri, shall we add tel URI? */
    /* Only 181 and divHdr181 is TRUE, this flag will be set */
    else if (notification)
    {

        /* Build the top diversion string */
        /* FID17364.0 SIP over TLS */
        strcat(top_div, "<");
        if (uriType == SIP_TOKEN_SIPS)

            strcat(top_div, "sips:");

        else
                strcat(top_div, "sip:");

        strcat(top_div, "unavailable@");

        strcat(top_div, fed_ext_name_ptr);

        strcat(top_div, ">");

        if (div.topDivReason.pres)
        {
           strcat(top_div, ";reason=");

           for (i = 0; i < MAX_HI_REASONS; i++)
           {
             if (div.topDivReason.val == divReasonVal2Str[i].reasonVal )
             {
                strcat(top_div, divReasonVal2Str[i].reasonStr);
                break;
             }
           }
           /* if we could not find reason string set to default "unknown" */
           if (i == MAX_HI_REASONS)
           {
                strcat(top_div, "unknown");
           }
        }

        if (div.count.pres  == PRSNT_NODEF)
        {
            strcat(top_div, ";counter=");


            strcat(top_div, reinterpret_cast<char *>(&(div.count.val)));
        }

        if (div.topPrivacy.pres == PRSNT_NODEF)
        {
            strcat(top_div, ";notification=\"");
            if (div.topPrivacy.val == SIPDIV_PRIVACY_OFF)
            {
              strcat(top_div, "presentation allowed\"");
            }
            else
            {
              strcat(top_div, "presentation not allowed\"");
            }
        }

        if (div.topScreen.pres == PRSNT_NODEF)
        {
            strcat(top_div, ";screen=\"");
            if (div.topScreen.val == SIPDIV_SCREEN_YES)
                strcat(top_div, "yes");
            else /* default SIPDIV_SCREEN_NO */
                strcat(top_div, "no");

            /* add '" ' in last */
            strcat(top_div, "\"");
        }
    }

    if (div.botDivNum.pres == PRSNT_NODEF)
    {
        /* Build the bottom diversion string */
        strcpy(bot_div, "<");
                switch (uriType)
        {
        case SIP_TOKEN_TEL:
            strcat(bot_div, "tel:");
            break;
        case SIP_TOKEN_SIP:
            strcat(bot_div, "sip:");
            break;
        /* FID 17021 URN service */
        case SIP_TOKEN_URN:
            strcat(bot_div, "urn:");
            break;
        /* FID17364.0 SIP Over TLS */
        case SIP_TOKEN_SIPS:
            strcat(bot_div, "sips:");
            break;
        default:
            IMS_DLOG(IMS_IBCF_LOGHIGH, "%s:Unsupported uri scheme:%ld, treated as sip",
                                __FUNCTION__, uriType);
            //telicaAbort(4);
            strcpy(bot_div, "sip:");
            break;
        }

        if (IBCF_add_tel_num(bot_div,reinterpret_cast<char *>(div.botDivNum.val), div.botDivNum.len,
                                &numType, uriType) == 0)
        {
                return IMS_SUCCESS;
        }
        /* FID17364.0 SIP over TLS */
        if ((uriType == SIP_TOKEN_SIP) || (uriType == SIP_TOKEN_SIPS)|| uriType == SIP_TOKEN_TEL)
        {
            strcat(bot_div, "@");
            strcat(bot_div, fed_ext_name_ptr);
        }
                #if 0
        else if (uriType == SIP_TOKEN_TEL)
        {
#ifndef CLOUD
            /* FID 14341.0 */
            if ( ipVersion == AF_INET6 )
            {
                sipa_add_phone_context(bot_div, LOCAL.local_addr_ipv6, NULL, numType);
            }
            /* FID 15720.4 */
            else if (sipInterface == SIPA_INTF_INTERNAL)
            {
                sipa_add_phone_context(bot_div, SIPA_INTERNAL_IP_ADDR, NULL, numType);
            }
            else if (sipInterface == SIPA_INTF_SIGC)
            {
                sipa_add_phone_context(bot_div, LOCAL.local_addr3, NULL, numType);
            }
            else
            {
                sipa_add_phone_context(bot_div, LOCAL.local_addr, NULL, numType);
            }
#else
            /* FID 17120.0 Outgoing sip headers should have SFED IP */
            SIPA_TRACE(NBB_DETAIL_TRC, ("%s: Populate bot_Div header with SFED's address, intfc %d, ipv %d\n",
                                           __FUNCTION__, sipInterface, ipVersion));
            sipa_build_ipaddr_with_sfedip (ipVersion, sipInterface, bot_div, numType);
#endif
        }
                #endif

       //SIPA_TRACE (NBB_DETAIL_TRC, ("%s:bot_DivStr:%s for ipVersion of %d, sipInterface:%d\n",
        //                    __FUNCTION__, bot_div, ipVersion, sipInterface));

        strcat(bot_div, ">;reason=");

        if (div.botDivReason.pres)
            strcat(bot_div, reasonStr[div.botDivReason.val]);
        else
            strcat(bot_div, "unknown");

        strcat(bot_div, ";counter=1");

        if (div.botPrivacy.pres == PRSNT_NODEF)
        {
          if(notification == FALSE)
          {
            strcat(bot_div, ";privacy=\"");
            if (div.botPrivacy.val == SIPDIV_PRIVACY_FULL)
                strcat(bot_div, "full");
            else if (div.botPrivacy.val == SIPDIV_PRIVACY_NAME)
                strcat(bot_div, "name");
            else if (div.botPrivacy.val == SIPDIV_PRIVACY_URI)
                strcat(bot_div, "uri");
            else /* default SIPDIV_PRIVACY_OFF */
                strcat(bot_div, "off");

            /* add '" ' in last */
            strcat(bot_div, "\"");
          }
          else
          {
            strcat(bot_div, ";notification=\"");
            if (div.botPrivacy.val == SIPDIV_PRIVACY_OFF)
            {
              strcat(bot_div, "presentation allowed\"");
            }
            else
            {
              strcat(bot_div, "presentation not allowed\"");
            }
          }
        }

        if (div.botScreen.pres == PRSNT_NODEF)
        {
            strcat(bot_div, ";screen=\"");
            if (div.botScreen.val == SIPDIV_SCREEN_YES)
                strcat(bot_div, "yes");
            else /* default SIPDIV_SCREEN_NO */
                strcat(bot_div, "no");

            /* add '" ' in last */
            strcat(bot_div, "\"");
        }
    }

    IMS_DLOG(IMS_IBCF_LOGLOW, "%s: top_div:%s \n bot_div:%s\n", __func__, top_div, bot_div);

    if (strLen(bot_div)!=0)
    {
        SIPia_AddOtherHeader(sipiaMsg_ptr,"Diversion",bot_div,sizeof("Diversion") - 1,strLen(bot_div),LIST_END);
    }

    if(strLen(top_div)!=0)
    {
        SIPia_AddOtherHeader(sipiaMsg_ptr,"Diversion",top_div,sizeof("Diversion") - 1,strLen(top_div),LIST_FRONT);
    }

    return IMS_SUCCESS;

}




/**PROC+**********************************************************************/
/* Name:         IBCFb2b_iw_build_uui                                        */
/*                                                                           */
/* Purpose:                                                                  */
/*                                                                           */
/* Returns:   ture or false.                                                 */
/*                                                                           */
/* Params: p_conEvntPtr                                                        */
/*      event_ptr                                                          */
/*                                                                           */
/* Operation:                                                                */
/* This function is to build user to user header from sipw. */
/*                                                                              */
/*                                                                           */
/**PROC-**********************************************************************/

IMS_RETVAL IBCFb2b_iw_build_uui(SipwLiConEvnt *p_conEvntPtr,B2BUAEvent *event_ptr)
{

        U16 len;
        char *ptr = NULL;
        char  uuiHeader[SIP_UUSINFO_MAX_LEN+1];
        char * rest_param = NULL;
        B2BUALegData    *leg_ptr  = NULL;
	 DMUSHORT sipi_profile_id = 0;
	DMUSHORT variant = 0;

        if (IS_VALID_PTR(p_conEvntPtr)== FALSE|| IS_VALID_PTR(event_ptr)== FALSE)
        {
                return IMS_FAIL;
        }

        B2BUACallData* calldata_ptr = event_ptr->calldata_ptr;
        if (calldata_ptr == NULL)
        {
                /* Can't get the sip message from current event */
                IMS_DLOG(IMS_IBCF_LOGHIGH, "%s() -  Can't get the calldata_ptr from current event, event type is %d", __func__, event_ptr->event);
                return IMS_FAIL;
        }

        SIPia_Msg              *sip_msg_ptr = event_ptr->m_ptr;
        if(IS_VALID_PTR(sip_msg_ptr) == FALSE)
        {
                return IMS_FAIL;
        }
	/* Get con info */
	CcConCb *con_ptr = IBCFb2b_iw_get_con(calldata_ptr);
	if(IS_VALID_PTR(con_ptr) == FALSE)
	{
		IMS_DLOG(IMS_IBCF_LOGHIGH, "%s() -	Can't get con", __func__);
		return IMS_FAIL;
	}
	DString *uui_ptr = IBCFutil_GetOtherHdr(sip_msg_ptr,USERTOUSER);
        if (IS_VALID_PTR(uui_ptr)== TRUE && DString_getLen (uui_ptr)>0 && DString_getLen(uui_ptr)<SIP_UUSINFO_MAX_LEN + 1)
        {
            IMS_DLOG(IMS_IBCF_LOGLOW, "%s(): uui header already there", __func__);
            /* If we are going to send to SIP, we want to map the ISUP to SIP header*/
            if ((IBCFb2b_iw_call_type(calldata_ptr) == CALL_TYPE_SIPT2SIP) &&
                (ccBothSupportUui(con_ptr) == TRUE) &&
                (event_ptr->leg_ptr == IBCF_EGRESS_LEG_PTR_ENHANCE(calldata_ptr)))
            {
                IMS_DLOG(IMS_IBCF_LOGLOW, "%s(): SIPT2SIP call and sending message to SIP", __func__);
                if ((p_conEvntPtr->sipUuiData.pres == PRSNT_NODEF) &&
                        p_conEvntPtr->sipUuiData.len && p_conEvntPtr->sipUuiData.val)
                {
                    IMS_DLOG(IMS_IBCF_LOGLOW, "%s(): seems like we also have uui data in ISUP: %s, will replace with it",
                                    __func__, p_conEvntPtr->sipUuiData.val);
                    SIPia_deleteOtherHeader(sip_msg_ptr,USERTOUSER);
                }
                else
                {
                    IMS_DLOG(IMS_IBCF_LOGLOW, "%s(): looks like no uui data in ISUP, just return", __func__);
                    return(IMS_SUCCESS);
                }
            }
            else if ((IBCFb2b_iw_call_type(calldata_ptr)  == CALL_TYPE_SIP2SIPT) &&
                     (event_ptr->leg_ptr == IBCF_INGRESS_LEG_PTR(calldata_ptr)) &&
                     (ccBothSupportUui(con_ptr) == TRUE))
            {
                IMS_DLOG(IMS_IBCF_LOGLOW, "%s(): SIP2SIPT call and sending message to SIP", __func__);
                if ((p_conEvntPtr->sipUuiData.pres == PRSNT_NODEF) &&
                        p_conEvntPtr->sipUuiData.len && p_conEvntPtr->sipUuiData.val)
                {
                    IMS_DLOG(IMS_IBCF_LOGLOW, "%s(): seems like we also have uui data in ISUP: %s, will replace with it",
                                    __func__, p_conEvntPtr->sipUuiData.val);
                    SIPia_deleteOtherHeader(sip_msg_ptr,USERTOUSER);
                }
                else
                {
                    IMS_DLOG(IMS_IBCF_LOGLOW, "%s(): looks like no uui data in ISUP, just return", __func__);
                    return(IMS_SUCCESS);
                }
            }
            else
            {
                IMS_DLOG(IMS_IBCF_LOGLOW, "%s(): just return", __func__);
                return IMS_SUCCESS;
            }
        }

	IBCFb2b_iw_get_sipt_profile_id(event_ptr,sipi_profile_id,variant);
        if (sipi_profile_id <0 || sipi_profile_id > 2048)
        {
                IMS_DLOG(IMS_IBCF_LOGHIGH, "%s: Invalide sipi_id", __func__, sipi_profile_id);
                return IMS_SUCCESS;
        }
        IBCF_SIPI_PROFILE_DATA* sipi_data_ptr = IBCF_get_sipi_profile_data_by_id(sipi_profile_id);
        if (IS_VALID_PTR(sipi_data_ptr)== false)
        {
                IMS_DLOG(IMS_IBCF_LOGHIGH,"%s:can't get the sipi_data",__func__);
                return IMS_SUCCESS;
        }
	if (IMSdc_get_ibcf_sip_sipi_interworking_profile_sipuuimap(sipi_profile_id) == FALSE)
	{
		return IMS_SUCCESS;
	}
	
	 len =  p_conEvntPtr->sipUuiData.len;

        if ( len !=0)
        {
                strncpy(uuiHeader,reinterpret_cast<const char *>(p_conEvntPtr->sipUuiData.val),len);
		uuiHeader[len] = SIP_ASCII_NULL;
#if 0/* sipuuifmt should not be used here */
                if (sipi_data_ptr->db_data.sipuuifmt != SIPUUIFORMAT_HEXASCII)
                {
			IMS_DLOG(IMS_IBCF_LOGLOW,"%s:sipUUIfmat isn't hex",__func__);
                        return IMS_SUCCESS;
                }
#endif
                memcpy(uuiHeader+len,";encoding=hex",sizeof(";encoding=hex")-1);
		uuiHeader[len + sizeof(";encoding=hex")-1]=SIP_ASCII_NULL;
                SIPia_AddOtherHeader(sip_msg_ptr,
                               USERTOUSER,
                                uuiHeader,
                                sizeof(USERTOUSER) -1,
                                strlen(uuiHeader) );
		IMS_DLOG(IMS_IBCF_LOGLOW,"%s:finish build the UUI header with raw data %s", __func__, p_conEvntPtr->sipUuiData.val);

        }
	if(p_conEvntPtr->sipUuiData.val != NULL)
	{
		RET_MEM_NULLCHK(p_conEvntPtr->sipUuiData.val);
		p_conEvntPtr->sipUuiData.val  = NULL;
		p_conEvntPtr->sipUuiData.pres = NOTPRSNT;
		p_conEvntPtr->sipUuiData.len  = 0;
		p_conEvntPtr->sipUuiData.size = 0;
	}
        return IMS_SUCCESS;
}

IMS_RETVAL IBCF_iw_build_called_party_id(B2BUAEvent *event_ptr, SipwLiConEvnt *p_SipConEvntPtr, NGSS_PORT_INDX port_idx, ULONG uriType)
{
	if(IS_VALID_PTR(event_ptr) == FALSE)
	{
		IMS_DLOG(IMS_IBCF_LOGHIGH, "%s(): Invalid event_ptr.", __func__);
		return IMS_FAIL;
	}
	
	B2BUALegData *leg_ptr = event_ptr->leg_ptr;
	if(IS_VALID_PTR(leg_ptr) == FALSE)
	{
		IMS_DLOG(IMS_IBCF_LOGHIGH, "%s(): Invalid leg_ptr from event_ptr.", __func__);
		return IMS_FAIL;
	}
	
	DMUSHORT id = 0;
	DMUSHORT variant = 0;
	IBCFb2b_iw_get_sipt_profile_id(event_ptr, id, variant);
	if(IMSdc_get_ibcf_sip_sipi_interworking_profile_pcdpid(id) == 0)
	{
		IMS_DLOG(IMS_IBCF_LOGLOWEST, "%s(): pcdpid in SIPI profile is 0. No need to build P_CALLED_PARTY_ID.", __func__);
		return IMS_SUCCESS;
	}
	
	SIPia_Msg *sip_msg_ptr = event_ptr->m_ptr;
	if(IS_VALID_PTR(sip_msg_ptr) == FALSE)
	{
		IMS_DLOG(IMS_IBCF_LOGHIGH, "%s(): Invalid sip_msg_ptr from event_ptr.", __func__);
		return IMS_FAIL;
	}
	
	if(IS_VALID_PTR(p_SipConEvntPtr) == FALSE)
	{
		IMS_DLOG(IMS_IBCF_LOGHIGH, "%s(): Invalid p_SipConEvntPtr.", __func__);
		return IMS_FAIL;
	}

	if(CPsip_Fix_Header_Empty(sip_msg_ptr, P_CALLED_PARTY_ID) == FALSE)
	{
		IMS_DLOG(IMS_IBCF_LOGLOWEST, "%s(): P_CALLED_PARTY_ID exists in SIP message. No need to add.", __func__);
		return IMS_SUCCESS;
	}

	if(p_SipConEvntPtr->pclId.pclIdVal.pres)
	{
		char p_called_party_id[256] = {0};
		SIPA_NUM_TYPE numType = SIPA_NUMTYPE_UKNWN;
		strcat(p_called_party_id, "<");
		IBCF_iw_add_uri_scheme(p_called_party_id, uriType);
		IBCF_add_tel_num(p_called_party_id, reinterpret_cast<char *>(p_SipConEvntPtr->pclId.pclIdVal.val), p_SipConEvntPtr->pclId.pclIdVal.len, &numType, uriType);
		if((uriType == SIP_TOKEN_SIP) || (uriType == SIP_TOKEN_SIPS))
		{
			strcat(p_called_party_id, "@");
			size_t len = strlen(p_called_party_id);
			IMS_RETVAL ims_retval = IBCF_get_external_name(port_idx, sip_msg_ptr, p_called_party_id + len, 256 - len, FALSE, FALSE);
			if(ims_retval != IMS_SUCCESS)
			{
				IMS_DLOG(IMS_IBCF_LOGHIGH, "%s(), failed to get external name.", __func__);
				return IMS_FAIL;
			}

			if(p_called_party_id[len] == '[')
			{ 
				//remove '[' & ']' from IPV6 address
				IBCF_strip_ipv6(p_called_party_id + len);
			}
		}
		else if(uriType == SIP_TOKEN_TEL)
		{
			char context[128];
			IMS_RETVAL ims_retval = IBCF_get_external_name(port_idx, sip_msg_ptr, context, sizeof(context), FALSE, FALSE);
			if(ims_retval != IMS_SUCCESS)
			{
				IMS_DLOG(IMS_IBCF_LOGHIGH, "%s(), failed to get external name.", __func__);
				return IMS_FAIL;
			}
			if(strlen(";phone-context=")+strlen(context)+1 > 256-strlen(p_called_party_id))
			{
				IMS_DLOG(IMS_IBCF_LOGHIGH, "%s(), buffer not enough to add phone-context.", __func__);
				return IMS_FAIL;
			}
			IBCF_iw_add_phone_context(p_called_party_id, context, NULL, numType);
		}
		else
		{
			IMS_DLOG(IMS_IBCF_LOGLOW, "%s(), unsupported URI type.", __func__);
		}
		if(strlen(p_called_party_id) >= 256-1)
		{
			IMS_DLOG(IMS_IBCF_LOGHIGH, "%s(), buffer not enough to generate p-called-party-id.", __func__);
			return IMS_FAIL;
		}
		strcat(p_called_party_id, ">");
		
		if(FALSE == SIPia_addHeader_String(sip_msg_ptr, P_CALLED_PARTY_ID, p_called_party_id, strlen(p_called_party_id)))
		{
			IMS_DLOG(IMS_IBCF_LOGHIGH, "%s(), failed to add P_CALLED_PARTY_ID.", __func__);
		}
	}
	else
	{
		IMS_DLOG(IMS_IBCF_LOGLOWEST, "%s(), pclIdVal is not present. no need to build.", __func__);
	}
}


/*
 * Function: IBCFb2b_iw_build_called_redirect_rpid
 *
 * Description: This function is to build Remote-Party-Id header
 *		with "party" of "called" or "redirect".
 *
 * Output: Add two Remote-Party-ID headers into SIPia message
 * 	one with "party=called" and another one with "party=redirect"
 *
 * Return: IMS_SUCCESS -- Successfully build the Remote-Party-ID headers
 * 	   IMS_FAIL	 -- Failed to build the Remote-Party-ID headers
 *
 */

IMS_RETVAL IBCFb2b_iw_build_called_redirect_rpid(SipwLiConEvnt *p_SipConEvntPtr, B2BUAEvent *event_ptr)
{
	const char	*fed_ext_name_ptr;
	DString		*rpid_body_ptr;
	char		tmp_hostport_buf[IMS_MAX_HOSTNAME];
	std:string	local_rpid_buffer;
	DMULONG		uri_type = 0;
	int		fed_ext_name_len = 0;
	int		rc;
	SIPA_NUM_TYPE	numType = SIPA_NUMTYPE_UKNWN;
	IBCF_URI_TYPE	uriType;
	SIPIA_RETURN	retval = SIPIA_SUCCESS;

	if ((IS_VALID_PTR(p_SipConEvntPtr) == FALSE) ||
		(IS_VALID_PTR(event_ptr) == FALSE))
	{
		IMS_DLOG(IMS_IBCF_LOGHIGH, "%s(): invalid input, event_ptr(%p), p_SipConEvntPtr(%p)",
			__func__, event_ptr, p_SipConEvntPtr);
		return IMS_FAIL;
	}

	/* Check to see if Remote-Party-ID header already
	 * included in the incoming SIP message. If so,
	 * pass it through and don't build a new one.
	 */
	rpid_body_ptr = IBCFutil_GetOtherHdr(event_ptr->m_ptr, "Remote-Party-ID");
	if (IS_VALID_PTR(rpid_body_ptr))
	{
		/* Header is available in incoming SIP message.
		 * Don't try to build a new one.
		 */
		IMS_DLOG(IMS_IBCF_LOGLOWEST, "%s(): RPID header(s) exists, no need to build new ones.",
			__func__);
		return IMS_SUCCESS;
	}

	const IBCF_TRUNK_GROUP_DATA *tg_data_ptr = IBCF_get_tg_data_by_sip_msg(event_ptr->m_ptr);
	if (IS_VALID_PTR(tg_data_ptr) == FALSE)
	{
		IMS_DLOG(IMS_IBCF_LOGHIGH, "%s(): Couldn't get Trunk Group Data from SIP message", __func__);
		return IMS_FAIL;
	}

	const IBCF_SIP_PROFILE_DATA *sip_profile_ptr = IBCF_get_sip_profile_data_by_id(tg_data_ptr->sip_profile_id);
	if (IS_VALID_PTR(sip_profile_ptr) == FALSE)
	{
		IMS_DLOG(IMS_IBCF_LOGHIGH, "%s(): Couldn't get SIP profile Data from SIP message", __func__);
		return IMS_FAIL;
	}

	uriType = sip_profile_ptr->uri_type;

	local_rpid_buffer.clear();

	/* Get SIP stack port index. */
	SIPIA_PORT_IDX sip_port_idx = B2BCutil_get_sipia_port_idx(event_ptr->leg_ptr);

	if (IS_VALID_SIPIA_PORT_IDX(sip_port_idx) == FALSE)
	{
		IMS_DLOG(IMS_IBCF_LOGHIGH, "%s(), invalid sip_port_idx=[%u] from SIP message type (request or response)=[%d].",
			 __func__, sip_port_idx, SIPia_getMsgType(event_ptr->m_ptr));

		return IMS_FAIL;
	}

	/* Get provisioned SIPia port index. */
	NGSS_PORT_INDX port_idx = SSgul_get_sipia_port_prov_key(sip_port_idx);
	if (port_idx == NGSS_DEFAULT_INDX)
	{
		IMS_DLOG(IMS_IBCF_LOGHIGH, "%s(), invalid port_idx=[%d].", __func__, port_idx);

		return IMS_FAIL;
	}

	const IBCF_VIRTUAL_NETWORK_DATA *vn_data_ptr = IBCF_get_vn_data_by_sip_msg(event_ptr->m_ptr);
	if (IS_VALID_PTR(vn_data_ptr) == FALSE)
	{
		IMS_DLOG(IMS_IBCF_LOGHIGH, "%s(), Couldn't find VN data through SIP messsage.", __func__);
		return IMS_FAIL;
	}

	if (vn_data_ptr->vn_type == IBCF_VIRTUAL_NETWORK_CORE)
	{
		fed_ext_name_ptr = IMScfed_get_core_fed_port_ext_host_name(port_idx, &fed_ext_name_len, event_ptr->m_ptr);
	}
	else
	{
		fed_ext_name_ptr = IMSpfed_get_peer_fed_port_ext_host_name(port_idx, event_ptr->m_ptr);
	}

	IMS_DLOG(IMS_IBCF_LOGLOWEST, "%s():FED Host Name[%s]",
		__func__, fed_ext_name_ptr);

	IMS_DLOG(IMS_IBCF_LOGLOWEST, "%s(): top Div entry is[%d]",
		__func__, p_SipConEvntPtr->div.topDivNum.pres);

	/* Build redirect RPID */
	if (p_SipConEvntPtr->div.topDivNum.pres == PRSNT_NODEF)
	{
		local_rpid_buffer.append("<");

		switch(uriType)
		{
		case IBCF_SIP_URI:
			local_rpid_buffer.append("sip:");
			uri_type = SIP_TOKEN_SIP;
			break;
		case IBCF_TEL_URI:
			local_rpid_buffer.append("tel:");
			uri_type = SIP_TOKEN_TEL;
			break;
		case IBCF_SIPS_URI:
			local_rpid_buffer.append("sips:");
			uri_type = SIP_TOKEN_SIPS;
			break;
		default:
			IMS_DLOG(IMS_IBCF_LOGHIGH, "%s(): invalid URI Type(%d) provided in SIP profile for TG(%d)",
					__func__, uriType, tg_data_ptr->tg_id);
			return IMS_FAIL;
		}/* End of switch (uri_type) */

		rc = IBCF_add_tel_num(tmp_hostport_buf, reinterpret_cast<char *>(p_SipConEvntPtr->div.topDivNum.val),
				p_SipConEvntPtr->div.topDivNum.len, &numType, uri_type);
		if (rc > 0)
		{
			local_rpid_buffer.append(tmp_hostport_buf, 
				strlen(tmp_hostport_buf));
		}
		else
		{
			IMS_DLOG(IMS_IBCF_LOGHIGH, "%s: building RPID User-Info part failed.", __func__);
		}

		/* FID17364.0 SIP over TLS */
		if (uriType == IBCF_SIP_URI || uriType == IBCF_SIPS_URI)
		{
			local_rpid_buffer.append("@");

			if (IS_VALID_PTR(fed_ext_name_ptr))
			{
				local_rpid_buffer.append(fed_ext_name_ptr, 0, strlen(fed_ext_name_ptr));
			}
			else
			{
				IMS_DLOG(IMS_IBCF_LOGHIGH, "%s(), No external host name provisioned for port(%d)",
						__func__, port_idx);
				return IMS_FAIL;
			}

		}
		else if (uriType == SIP_TOKEN_TEL)
		{
			/* "phone-context" should be appended to the URI
			 * Not supported it now.
			 */
		}

		local_rpid_buffer.append(">;reason=");
		switch (p_SipConEvntPtr->div.topDivReason.val)
		{
		case SIPDIV_RSN_CODE_USER_BUSY:
			local_rpid_buffer.append("busy");
			break;

		case SIPDIV_RSN_CODE_NO_ANSWER:
			local_rpid_buffer.append("noAnswer");
			break;

		case SIPDIV_RSN_CODE_UNCONDITIONAL:
		default:
			local_rpid_buffer.append("unconditional");
			break;
		}/* End of switch(div.topDivReason.val) */

		if (p_SipConEvntPtr->div.count.pres  == PRSNT_NODEF)
		{
			local_rpid_buffer.append(";counter=");

			char count;
			count = p_SipConEvntPtr->div.count.val + '0';
			local_rpid_buffer.append(1, count);
		}

		if (p_SipConEvntPtr->div.topPrivacy.pres == PRSNT_NODEF)
		{
			local_rpid_buffer.append(";privacy=");

			switch (p_SipConEvntPtr->div.topPrivacy.val)
			{
			case SIPDIV_PRIVACY_FULL:
				local_rpid_buffer.append("full");
				break;

			case SIPDIV_PRIVACY_NAME:
				local_rpid_buffer.append("name");
				break;

			case SIPDIV_PRIVACY_URI:
				local_rpid_buffer.append("uri");
				break;

			case SIPDIV_PRIVACY_OFF:
			default:
				local_rpid_buffer.append("off");
				break;
			}
		}/* End if (div.topPrivacy.pres) */

		if (p_SipConEvntPtr->div.topScreen.pres == PRSNT_NODEF)
		{
			local_rpid_buffer.append(";screen=");
			switch (p_SipConEvntPtr->div.topScreen.val)
			{
			case SIPDIV_SCREEN_YES:
				local_rpid_buffer.append("yes");
				break;

			case SIPDIV_SCREEN_NO:
			default:
				local_rpid_buffer.append("no");
				break;
			}
		}/* End if (div.topScreen.pres) */

		local_rpid_buffer.append(";party=redirect");

		IMS_DLOG(IMS_IBCF_LOGLOWEST, "%s(): RPID header string [%s]",
			__func__, local_rpid_buffer.c_str());

		/* add RPID to SIPia message */
		retval = SIPia_AddOtherHeader(event_ptr->m_ptr, "Remote-Party-ID",
				local_rpid_buffer.c_str(),
				COMPUTE_STRING_LENGTH,
				COMPUTE_STRING_LENGTH,
				LIST_END);

		if (retval != SIPIA_SUCCESS)
		{
			IMS_DLOG(IMS_IBCF_LOGMED, "%s():Failed to add redirect RPID header, RTN[%d]",
				__func__, retval);
		}
	}/* End of if (div.topDivNum.pres == PRSNT_NODEF) */

	/* Clear the buffer for following header build */
	local_rpid_buffer.clear();

	IMS_DLOG(IMS_IBCF_LOGLOWEST, "%s(): bottom Div entry is[%d]",
		__func__, p_SipConEvntPtr->div.botDivNum.pres);

	/* Build the called rpid */
	if (p_SipConEvntPtr->div.botDivNum.pres == PRSNT_NODEF)
	{
		local_rpid_buffer.append("<");

		switch(uriType)
		{
		case IBCF_SIP_URI:
			local_rpid_buffer.append("sip:");
			uri_type = SIP_TOKEN_SIP;
			break;
		case IBCF_TEL_URI:
			local_rpid_buffer.append("tel:");
			uri_type = SIP_TOKEN_TEL;
			break;
		case IBCF_SIPS_URI:
			local_rpid_buffer.append("sips:");
			uri_type = SIP_TOKEN_SIPS;
			break;
		default:
			IMS_DLOG(IMS_IBCF_LOGHIGH, "%s(): invalid URI Type(%d) provided in SIP profile for TG(%d)",
					__func__, uriType, tg_data_ptr->tg_id);
			return IMS_FAIL;
		}/* End of switch (uri_type) */

		rc = IBCF_add_tel_num(tmp_hostport_buf, reinterpret_cast<char *>(p_SipConEvntPtr->div.botDivNum.val), 
				p_SipConEvntPtr->div.botDivNum.len, &numType, uri_type);
		if (rc > 0)
		{
			local_rpid_buffer.append(tmp_hostport_buf, 
				strlen(tmp_hostport_buf));
		}
		else
		{
			IMS_DLOG(IMS_IBCF_LOGHIGH, "%s: building RPID User-Info part failed.", __func__);
		}

		/* FID17364.0 SIP over TLS */
		if (uriType == IBCF_SIP_URI || uriType == IBCF_SIPS_URI)
		{
			local_rpid_buffer.append("@");

			if (IS_VALID_PTR(fed_ext_name_ptr))
			{
				local_rpid_buffer.append(fed_ext_name_ptr, 0, strlen(fed_ext_name_ptr));
			}
			else
			{
				IMS_DLOG(IMS_IBCF_LOGHIGH, "%s(), No external host name provisioned for port(%d)",
						__func__, port_idx);
				return IMS_FAIL;
			}

		}
		else if (uriType == SIP_TOKEN_TEL)
		{
			/* "phone-context" should be appended to the URI
			 * Not supported it now.
			 */
		}

		local_rpid_buffer.append(">;reason=");
		switch (p_SipConEvntPtr->div.botDivReason.val)
		{
		case SIPDIV_RSN_CODE_USER_BUSY:
			local_rpid_buffer.append("busy");
			break;

		case SIPDIV_RSN_CODE_NO_ANSWER:
			local_rpid_buffer.append("noAnswer");
			break;

		case SIPDIV_RSN_CODE_UNCONDITIONAL:
		default:
			local_rpid_buffer.append("unconditional");
			break;
		} /* End switch of (div.botDivReason.val) */

		local_rpid_buffer.append(";counter=1");

		if (p_SipConEvntPtr->div.botPrivacy.pres == PRSNT_NODEF)
		{
			local_rpid_buffer.append(";privacy=");

			switch (p_SipConEvntPtr->div.botPrivacy.val)
			{
			case SIPDIV_PRIVACY_FULL:
				local_rpid_buffer.append("full");
				break;

			case SIPDIV_PRIVACY_NAME:
				local_rpid_buffer.append("name");
				break;

			case SIPDIV_PRIVACY_URI:
				local_rpid_buffer.append("uri");
				break;

			case SIPDIV_PRIVACY_OFF:
			default:
				local_rpid_buffer.append("off");
				break;
			}
		} /* End of if (div.botPrivacy.pres) */

		if (p_SipConEvntPtr->div.botScreen.pres == PRSNT_NODEF)
		{
			local_rpid_buffer.append(";screen=");
			switch (p_SipConEvntPtr->div.botScreen.val)
			{
			case SIPDIV_SCREEN_YES:
				local_rpid_buffer.append("yes");
				break;

			case SIPDIV_SCREEN_NO:
			default:
				local_rpid_buffer.append("no");
			}
		}/* End if (div.botScreen.val) */

		local_rpid_buffer.append(";party=called");

		IMS_DLOG(IMS_IBCF_LOGLOWEST, "%s(): RPID header string [%s]",
			__func__, local_rpid_buffer.c_str());

		/* add RPID to SIPia message */
		retval = SIPia_AddOtherHeader(event_ptr->m_ptr, "Remote-Party-ID",
				local_rpid_buffer.c_str(),
				COMPUTE_STRING_LENGTH,
				COMPUTE_STRING_LENGTH,
				LIST_END);

		if (retval != SIPIA_SUCCESS)
		{
			IMS_DLOG(IMS_IBCF_LOGMED, "%s():Failed to add called RPID header, RTN[%d]",
				__func__, retval);
		}
	} /* End of if (div.botDivNum.pres == PRSNT_NODEF) */

	return IMS_SUCCESS;
}/* End of function IBCFb2b_iw_build_called_redirect_rpid */


/*
 * Function:	IBCFb2b_iw_build_rpid
 *
 * Description:	This function is to build Remote-Party-ID header
 * 		from "rpid" structure in SipwLiConEvnt
 *
 * Output:	Add "Remote-Party-ID" header into SIPia message
 *
 * Return:	IMS_SUCCESS -- Successfully build RPID header
 * 		IMS_FAIL    -- Faled to build RPID header
 */

IMS_RETVAL IBCFb2b_iw_build_rpid (SipwLiConEvnt *p_SipConEvntPtr, B2BUAEvent *event_ptr)
{
	const char	*fed_ext_name_ptr;
	DString		*rpid_body_ptr;
	std:string	local_rpid_buffer;
	int		fed_ext_name_len = 0;
	SIPA_NUM_TYPE	numType = SIPA_NUMTYPE_UKNWN;
	IBCF_URI_TYPE	uriType;
	SIPIA_RETURN	retval = SIPIA_SUCCESS;

	if ((IS_VALID_PTR(p_SipConEvntPtr) == FALSE) ||
		(IS_VALID_PTR(event_ptr) == FALSE))
	{
		IMS_DLOG(IMS_IBCF_LOGHIGH, "%s(): invalid input, event_ptr(%p), p_SipConEvntPtr(%p)",
			__func__, event_ptr, p_SipConEvntPtr);
		return IMS_FAIL;
	}

	/* Check to see if Remote-Party-ID header already
	 * included in the incoming SIP message. If so,
	 * pass it through and don't build a new one.
	 */
	rpid_body_ptr = IBCFutil_GetOtherHdr(event_ptr->m_ptr, "Remote-Party-ID");
	if (IS_VALID_PTR(rpid_body_ptr))
	{
		/* Header is available in incoming SIP message.
		 * Don't try to build a new one.
		 */
		IMS_DLOG(IMS_IBCF_LOGLOWEST, "%s(): RPID header exists, no need to build one.",
			__func__);
		return IMS_SUCCESS;
	}

	const IBCF_TRUNK_GROUP_DATA *tg_data_ptr = IBCF_get_tg_data_by_sip_msg(event_ptr->m_ptr);
	if (IS_VALID_PTR(tg_data_ptr) == FALSE)
	{
		IMS_DLOG(IMS_IBCF_LOGHIGH, "%s(): Couldn't get Trunk Group Data from SIP message", __func__);
		return IMS_FAIL;
	}

	const IBCF_SIP_PROFILE_DATA *sip_profile_ptr = IBCF_get_sip_profile_data_by_id(tg_data_ptr->sip_profile_id);
	if (IS_VALID_PTR(sip_profile_ptr) == FALSE)
	{
		IMS_DLOG(IMS_IBCF_LOGHIGH, "%s(): Couldn't get SIP profile Data from SIP message", __func__);
		return IMS_FAIL;
	}

	uriType = sip_profile_ptr->uri_type;

	local_rpid_buffer.clear();

	/* Get SIP stack port index. */
	SIPIA_PORT_IDX sip_port_idx = B2BCutil_get_sipia_port_idx(event_ptr->leg_ptr);

	if (IS_VALID_SIPIA_PORT_IDX(sip_port_idx) == FALSE)
	{
		IMS_DLOG(IMS_IBCF_LOGHIGH, "%s(), invalid sip_port_idx=[%u] from SIP message type (request or response)=[%d].",
			 __func__, sip_port_idx, SIPia_getMsgType(event_ptr->m_ptr));

		return IMS_FAIL;
	}

	/* Get provisioned SIPia port index. */
	NGSS_PORT_INDX port_idx = SSgul_get_sipia_port_prov_key(sip_port_idx);
	if (port_idx == NGSS_DEFAULT_INDX)
	{
		IMS_DLOG(IMS_IBCF_LOGHIGH, "%s(), invalid port_idx=[%d].", __func__, port_idx);

		return IMS_FAIL;
	}

	const IBCF_VIRTUAL_NETWORK_DATA *vn_data_ptr = IBCF_get_vn_data_by_sip_msg(event_ptr->m_ptr);
	if (IS_VALID_PTR(vn_data_ptr) == FALSE)
	{
		IMS_DLOG(IMS_IBCF_LOGHIGH, "%s(), Couldn't find VN data through SIP messsage.", __func__);
		return IMS_FAIL;
	}

	if (vn_data_ptr->vn_type == IBCF_VIRTUAL_NETWORK_CORE)
	{
		fed_ext_name_ptr = IMScfed_get_core_fed_port_ext_host_name(port_idx, &fed_ext_name_len, event_ptr->m_ptr);
	}
	else
	{
		fed_ext_name_ptr = IMSpfed_get_peer_fed_port_ext_host_name(port_idx, event_ptr->m_ptr);
	}

	IMS_DLOG(IMS_IBCF_LOGLOWEST, "%s():FED Host Name[%s]",
		__func__, fed_ext_name_ptr);

	IMS_DLOG(IMS_IBCF_LOGLOWEST, "%s():RPID Entry is[%d]",
		__func__, p_SipConEvntPtr->rpid.eh.pres);

	IMS_DLOG(IMS_IBCF_LOGLOWEST, "%s():From Header Entry is[%d]",
		__func__, p_SipConEvntPtr->fromHdr.eh.pres);

	if (p_SipConEvntPtr->rpid.eh.pres == PRSNT_NODEF)
	{
		if (p_SipConEvntPtr->rpid.displayName.pres == PRSNT_NODEF)
		{
			/* remote party id shall not include Anonymous - bug 44752 */
			/* As with From Hdr set rpid name to anonymous if needed (req from MCI - bug 27253) */
			char *tmp_ptr;
			local_rpid_buffer.append(1, '"');
			tmp_ptr = reinterpret_cast<char *>(p_SipConEvntPtr->rpid.displayName.val);
			local_rpid_buffer.append(tmp_ptr, 0, p_SipConEvntPtr->rpid.displayName.len);
			local_rpid_buffer.append(1, '"');
			IMS_DLOG(IMS_IBCF_LOGLOWEST, "%s(): RPID Display-Name [%s]",
				__func__, local_rpid_buffer.c_str());
		}

		local_rpid_buffer.append("<");

		/* FID17364.0 SIP over TLS */
		if (uriType == IBCF_SIPS_URI)
		{
			local_rpid_buffer.append("sips:");
		}
		else
		{
			local_rpid_buffer.append("sip:");
		}

		IMS_DLOG(IMS_IBCF_LOGLOWEST, "%s(): RPID header string [%s]",
			__func__, local_rpid_buffer.c_str());

		/* RPID Addr Spec */
		if (p_SipConEvntPtr->rpid.addrSpec.pres == PRSNT_NODEF)
		{
			char *tmp_ptr;
			tmp_ptr = reinterpret_cast<char *>(p_SipConEvntPtr->rpid.addrSpec.val);
			local_rpid_buffer.append(tmp_ptr, 0, p_SipConEvntPtr->rpid.addrSpec.len);
		}
		else
		{
			local_rpid_buffer.append("anonymous");
		}

		IMS_DLOG(IMS_IBCF_LOGLOWEST, "%s(): RPID header string [%s]",
			__func__, local_rpid_buffer.c_str());

		local_rpid_buffer.append("@");

		if (IS_VALID_PTR(fed_ext_name_ptr))
		{
			local_rpid_buffer.append(fed_ext_name_ptr, 0, strlen(fed_ext_name_ptr));
		}
		else
		{
			IMS_DLOG(IMS_IBCF_LOGHIGH, "%s(), No external host name provisioned for port(%d)",
					__func__, port_idx);
			return IMS_FAIL;
		}

		IMS_DLOG(IMS_IBCF_LOGLOWEST, "%s(): RPID header string [%s]",
			__func__, local_rpid_buffer.c_str());

		if ((p_SipConEvntPtr->rpid.addrSpec.pres == PRSNT_NODEF) &&
			(uriType != IBCF_TEL_URI) &&
			(p_SipConEvntPtr->rpid.type == SIPA_TELE_SUB_PART))
		{
			local_rpid_buffer.append(";user=phone");
		}

		/* FID17364.0 SIP over TLS */
		if ((tg_data_ptr->remote_addr_type == IBCF_TRUNK_GROUP_ADDRESS_TYPE_FQDN) &&
			(tg_data_ptr->transport_type == IBCF_TRANSPT_TLS))
		{
			local_rpid_buffer.append(";transport=tcp");
		}

		local_rpid_buffer.append(">");

		IMS_DLOG(IMS_IBCF_LOGLOWEST, "%s(): RPID header string [%s]",
			__func__, local_rpid_buffer.c_str());

		/* RPI Pty Type */
		if (p_SipConEvntPtr->rpid.rpiPtyType.pres == PRSNT_NODEF)
		{
			switch(p_SipConEvntPtr->rpid.rpiPtyType.val)
			{
			case SIPRPI_PTY_TYPE_CALLED:
				local_rpid_buffer.append(";party=called");
				break;
			case SIPRPI_PTY_TYPE_CALLING:
			default:
				local_rpid_buffer.append(";party=calling");
				break;
			}
		}/* End of if (rpid.rpiPtyType.pres == PRSNT_NODEF) */

		IMS_DLOG(IMS_IBCF_LOGLOWEST, "%s(): RPID header string [%s]",
			__func__, local_rpid_buffer.c_str());

		/* RPI Id Type */
		if (p_SipConEvntPtr->rpid.rpiIdType.pres == PRSNT_NODEF)
		{
			switch(p_SipConEvntPtr->rpid.rpiIdType.val)
			{
			case SIPRPI_ID_TYPE_TERM:
				local_rpid_buffer.append(";id-type=name");
				break;
			case SIPRPI_ID_TYPE_USER:
				local_rpid_buffer.append(";id-type=uri");
				break;
			case SIPRPI_ID_TYPE_SUBSCRIBER:
			default:
				local_rpid_buffer.append(";id-type=subscriber");
				break;
			}
		}/* End of if (rpid.rpiIdType.pres == PRSNT_NODEF) */

		IMS_DLOG(IMS_IBCF_LOGLOWEST, "%s(): RPID header string [%s]",
			__func__, local_rpid_buffer.c_str());

		/* RPI Privacy */
		if (p_SipConEvntPtr->rpid.rpiPrivacy.pres == PRSNT_NODEF)
		{
			switch(p_SipConEvntPtr->rpid.rpiPrivacy.val)
			{
			case  SIPRPI_PRIVACY_FULL:
				local_rpid_buffer.append(";privacy=full");
				break;
			case  SIPRPI_PRIVACY_NAME:
				local_rpid_buffer.append(";privacy=name");
				break;
			case  SIPRPI_PRIVACY_URI:
				local_rpid_buffer.append(";privacy=uri");
				break;
			case  SIPRPI_PRIVACY_OFF:
			default:
				local_rpid_buffer.append(";privacy=off");
				break;
			}
		} /* End of if (rpid.rpiPrivacy.pres == PRSNT_NODEF) */

		IMS_DLOG(IMS_IBCF_LOGLOWEST, "%s(): RPID header string [%s]",
			__func__, local_rpid_buffer.c_str());

		/* RPI Screen */
		if (p_SipConEvntPtr->rpid.rpiScreen.pres == PRSNT_NODEF)
		{
			switch(p_SipConEvntPtr->rpid.rpiScreen.val)
			{
			case  SIPRPI_SCREEN_YES:
				local_rpid_buffer.append(";screen=yes");
				break;
			case  SIPRPI_SCREEN_NO:
			default:
				local_rpid_buffer.append(";screen=no");
				break;
			}
		} /* End of if (rpid.rpiScreen.pres == PRSNT_NODEF) */

		IMS_DLOG(IMS_IBCF_LOGLOWEST, "%s(): RPID header string [%s]",
			__func__, local_rpid_buffer.c_str());
	}
	else if (p_SipConEvntPtr->fromHdr.eh.pres != NOTPRSNT) /* gdixit-08/03/2005-BUG:40015 */
	{
		local_rpid_buffer.append("<");

		/* FID17364.0 SIP over TLS */
		if (uriType == IBCF_SIPS_URI)
		{
			local_rpid_buffer.append("sips:");
		}
		else
		{
			local_rpid_buffer.append("sip:");
		}

		if (p_SipConEvntPtr->fromHdr.fromVal.pres == PRSNT_NODEF)
		{
			char *tmp_ptr;
			tmp_ptr = reinterpret_cast<char *>(p_SipConEvntPtr->fromHdr.fromVal.val);
			local_rpid_buffer.append(tmp_ptr, 0, p_SipConEvntPtr->fromHdr.fromVal.len);
		}

		local_rpid_buffer.append("@");

		if (IS_VALID_PTR(fed_ext_name_ptr))
		{
			local_rpid_buffer.append(fed_ext_name_ptr, 0, strlen(fed_ext_name_ptr));
		}
		else
		{
			IMS_DLOG(IMS_IBCF_LOGHIGH, "%s(), No external host name provisioned for port(%d)",
					__func__, port_idx);
			return IMS_FAIL;
		}

		if (p_SipConEvntPtr->fromHdr.type != SIPA_USER_PART)
		{
			local_rpid_buffer.append(";user=phone");
		}

		/* FID17364.0 SIP over TLS */
		if ((tg_data_ptr->remote_addr_type == IBCF_TRUNK_GROUP_ADDRESS_TYPE_FQDN) &&
			(tg_data_ptr->transport_type == IBCF_TRANSPT_TLS))
		{
			local_rpid_buffer.append(";transport=tcp");
		}

		local_rpid_buffer.append(";party=calling");
		local_rpid_buffer.append(";id-type=subscriber");
		local_rpid_buffer.append(";privacy=off");
		local_rpid_buffer.append(";screen=no");
	} /* End of if (fromHdr.pres.eh.pres != NOTPRSNT) */

	IMS_DLOG(IMS_IBCF_LOGLOWEST, "%s(): RPID header string [%s]",
		__func__, local_rpid_buffer.c_str());

	/* Add header to SIPia message */
	retval = SIPia_AddOtherHeader(event_ptr->m_ptr, "Remote-Party-ID",
			local_rpid_buffer.c_str(),
			COMPUTE_STRING_LENGTH,
			COMPUTE_STRING_LENGTH,
			LIST_END);

	if (retval != SIPIA_SUCCESS)
	{
		IMS_DLOG(IMS_IBCF_LOGMED, "%s(): Failed to add RPID header, RTN[%d]",
			__func__, retval);
	}

	return IMS_SUCCESS;
} /* End of function IBCFb2b_iw_build_rpid */

/* Function:      IBCFb2b_iw_build_p_asserted_id
 *
 * Descriptions:  The function is used to build P-Asserted-Identity Header from
 *                SipwLiConEvnt to SIPia_Msg
 *
 * Inputs:        p_SipConEvntPtr  - SipwLiConEvnt
 *                event_ptr       - output sip message
 *
 * Return:        IMS_SUCCESS/IMS_FAIL
 */
IMS_RETVAL
IBCFb2b_iw_build_p_asserted_id(B2BUAEvent *   event_ptr,
                                SipwLiConEvnt *p_SipConEvntPtr)
{
    SIPA_NUM_TYPE              numType          = SIPA_NUMTYPE_UKNWN;
    unsigned long              uriType          = 0;
    BOOL                       passThruPAID     = FALSE; /* FID 15903.0 */
    BOOL                       oliSet           = FALSE; /* FID 16466.0 */
    BOOL                       privacyProcessed = FALSE;
    IBCF_URI_TYPE              uri_type         = IBCF_MAX_URI;
    std::string                p_asserted_id_string;
    std::string                srcAddrStr;
    B2BUALegData *             leg_ptr        = NULL_PTR;
    B2BUALegData *             mate_ptr       = NULL_PTR;
    const char *               host_str       = NULL_PTR;
    int                        host_len       = 0;
    NGSS_PORT_INDX             ngss_port_idx  = 0;
    SIPIA_PORT_IDX             sipia_port_idx = 0;
    IBCF_VN_TG_ID              tg_info;
    IBCF_VIRTUAL_NETWORK_DATA *vn_data_ptr = NULL_PTR;

    if (!IS_VALID_PTR(p_SipConEvntPtr) || !IS_VALID_PTR(event_ptr) ||
        !IS_VALID_PTR(event_ptr->m_ptr))
    {
        IMS_DLOG(IMS_IBCF_LOGLOW, "%s: Invalid input, msg_ptr(%p), p_SipConEvntPtr(%p)", __func__,
                 event_ptr, p_SipConEvntPtr);
        return (IMS_FAIL);
    }

    B2BUAEventToLegs(event_ptr, &leg_ptr, &mate_ptr);
    if (IS_VALID_PTR(leg_ptr) == FALSE || IS_VALID_PTR(mate_ptr) == FALSE)
    {
        IMS_DLOG(IMS_IBCF_LOGHIGH, "%s: Invalid leg_ptr %p or mate_ptr %p", __func__, leg_ptr,
                 mate_ptr);
        return (IMS_FAIL);
    }

    if (IS_VALID_PTR(event_ptr->calldata_ptr) == FALSE)
    {
        IMS_DLOG(IMS_IBCF_LOGHIGH, "%s: invalid calldata in event_ptr", __func__);
        return (IMS_FAIL);
    }

    sipia_port_idx = B2BCutil_get_sipia_port_idx(event_ptr->calldata_ptr);
    if (IS_VALID_SIPIA_PORT_IDX(sipia_port_idx) == FALSE)
    {
        IMS_DLOG(IMS_IBCF_LOGHIGH, "%s: Can't get valid sipia_port_idx:%d", __func__,
                 sipia_port_idx);
        return (IMS_FAIL);
    }

    ngss_port_idx = SSgul_get_sipia_port_prov_key(sipia_port_idx);

    SIPia_Msg *msg_ptr = event_ptr->m_ptr;

    IBCF_SIP_PROFILE_DATA *sip_profile_ptr = IBCF_get_sip_prof_data_by_sipmsg(msg_ptr);
    if (!IS_VALID_PTR(sip_profile_ptr))
    {
        IMS_DLOG(IMS_IBCF_LOGLOW, "%s(), Could sip profile ", __func__);
        return (IMS_FAIL);
    }

    DMUSHORT id = 0;
    DMUSHORT variant = 0;

    IBCFb2b_iw_get_sipt_profile_id(event_ptr, id, variant);

    IBCF_SIPI_PROFILE_DATA *sipi_prof_data_ptr = IBCF_get_sipi_profile_data_by_id(id);
    if (!IS_VALID_PTR(sipi_prof_data_ptr))
    {
        IMS_DLOG(IMS_IBCF_LOGHIGH, "%s: could not found  sipi_prof_data_ptr", __func__);
        return (IMS_FAIL);
    }

    tg_info     = IBCF_get_tginfo_from_leg(leg_ptr);
    vn_data_ptr = IBCF_get_vn_data_by_id(tg_info.vn_id);
    if (IS_VALID_PTR(vn_data_ptr) == FALSE)
    {
        IMS_DLOG(IMS_IBCF_LOGHIGH, "%s: invalid vn_data_ptr with vn_id(%d)", __func__,
                 tg_info.vn_id);
        return (IMS_FAIL);
    }

    /* FID 15903.0 */
    /* FID 17028.0 when pass thru PAID, use PAID_pass_thru instead of */
    /* p_asserted_id in CCInfoCB */
    /* only when there is PAID in original message, and paid_population is set pass through,
     * we set passThruPAID to TRUE
     */
    SIPia_List  *list_ptr = reinterpret_cast<SIPia_List *>(SIPia_getHeaderContainer(event_ptr->m_ptr, 
                            SIPIA_P_ASSERTED_IDENTITY));
    if (sip_profile_ptr->paid_population == IBCF_HEADER_HANDLING_PASS_THROUGH ||
        sip_profile_ptr->paid_population == IBCF_HEADER_HANDLING_REMOVE_WITH_PRIVACY)
    {
        if ((IS_VALID_PTR(list_ptr) == TRUE) && (SIPia_List_num_items(list_ptr) > 0))
        {
            /* when the flag is false , it means PAID is not inserted by route service in
             * IBCFb2b_PAID_handling, keep the PAID.
             */
            if (IBCFb2b_get_paid_inserted_flag(event_ptr->leg_ptr) == FALSE)
            {
                passThruPAID = TRUE;
            }
        }
    }
    else
    {   /* else means: IBCF_HEADER_HANDLING_REMOVE */
        passThruPAID = TRUE; /* means no need to build P-Asserted_identity Header */
    }

    IBCFb2b_reset_paid_inserted_flag(event_ptr->leg_ptr);

    std::string privacyHeader;

    /* Bug71618, at first populate Privacy from sipRawPrivacy */
    if ((p_SipConEvntPtr->sipRawPrivacy.eh.pres == PRSNT_NODEF) &&
        (p_SipConEvntPtr->sipRawPrivacy.privacy.pres == PRSNT_NODEF) &&
        (p_SipConEvntPtr->sipRawPrivacy.privacy.len != 0))
    {
        /* length of privacy is limited to 128 - 1 */
        privacyHeader.append(
            reinterpret_cast<const char *>(p_SipConEvntPtr->sipRawPrivacy.privacy.val));

        IMS_DLOG(IMS_IBCF_LOGLOW, "%s: handle sipRawPrivacy:%s", __func__, privacyHeader.c_str());

        /* TODO:: taken farEndTrustDomain as ? is it the same as
         * IBCF_VIRTUAL_NETWORK_CORE or
         * IBCF_VIRTUAL_NETWORK_PEER?  take farEndTrustDomain==FALSE as
         * IBCF_VIRTUAL_NETWORK_PEER?
         */
        std::string::size_type pos = std::string::npos;
        if (vn_data_ptr->vn_type == IBCF_VIRTUAL_NETWORK_PEER)
        {
            /* remove "history" from Privacy Header if there is */
            pos = privacyHeader.find("history;");
            if (pos != std::string::npos)
            {
                privacyHeader.erase(pos, strlen("history;"));
            }

            pos = privacyHeader.find("history");
            if (pos != std::string::npos)
            {
                privacyHeader.erase(pos, strlen("history"));
            }

            /* if there is only "critical" now, renove it. */
            if (privacyHeader == "critical")
            {
                privacyHeader.assign("");
            }
        }

        /* if there is no Privacy header in sip message, now, we need to add a
         * fresh new Privacy header;
         * BUT, if there is Privacy header, just append values present in the
         * variable privacyHeader not present in SIPia_Msg Privacy header.
         */
        /* now we need to remove existing Privacy header before add new one */
        SIPia_DeleteHeaderContainer(msg_ptr, SIPIA_PRIVACY);
	IBCFb2b_iw_add_privacy_value(msg_ptr, privacyHeader);
        privacyProcessed = TRUE;
    }

    /* NOTE:  REMOVED - keywaord "ISDN CLI parameters" associating code */

    if (p_SipConEvntPtr->rpid.eh.pres == NOTPRSNT)
    {
        /* Do not build a P-Asserted-Identity nor Privacy if no rpid */
        IMS_DLOG(IMS_IBCF_LOGLOW, "%s: No p_asserted-Identity to build\n", __func__);
        return (IMS_SUCCESS);
    }

    /* Build pAssertedId and Privacy headers from data in rpid */

    uri_type = sip_profile_ptr->uri_type;

    /* FID 15903.0 */
    if ((!passThruPAID) && (p_SipConEvntPtr->rpid.addrSpec.pres == PRSNT_NODEF))
    {
        /* P-Asserted_identity: name-addr */
        if (p_SipConEvntPtr->rpid.displayName.pres == PRSNT_NODEF)
        {
            /*BUG65259 Chage PAID display name according to privacy */
            /* P-Asserted-Identity should include all info (ie no Anonymous etc)
             */
            /* As with From Hdr set p_asserted name to anonymous (not clear in
             * AT&T
             * NGBE reqs)
             */
            if ((p_SipConEvntPtr->rpid.rpiPrivacy.pres == PRSNT_NODEF) &&
                ((p_SipConEvntPtr->rpid.rpiPrivacy.val == SIPRPI_PRIVACY_FULL) ||
                 (p_SipConEvntPtr->rpid.rpiPrivacy.val == SIPRPI_PRIVACY_NAME)))
            {
                p_asserted_id_string.append("\"Anonymous\"");
            }
            else
            {
                p_asserted_id_string.append("\"");
                p_asserted_id_string.append(
                    reinterpret_cast<char *>(p_SipConEvntPtr->rpid.displayName.val),
                    p_SipConEvntPtr->rpid.displayName.len);
                p_asserted_id_string.append("\"");
            }
        }

        /* P-Asserted-Identity: addr-spec */
        p_asserted_id_string.append("<");

        char uri_type_string[16] = {0};
        IBCF_iw_add_uri_scheme(uri_type_string, convertImsUri2IsupUri(uri_type));
        p_asserted_id_string.append(uri_type_string);

        char num_string[64] = {0};
        IBCF_add_tel_num(num_string, reinterpret_cast<char *>(p_SipConEvntPtr->rpid.addrSpec.val),
                         p_SipConEvntPtr->rpid.addrSpec.len, &numType,
                         convertImsUri2IsupUri(uri_type));
        p_asserted_id_string.append(num_string);

        /* FID 14890 check trunk config  cpcTag and insert ;cpc=xxx */
        if ((sipi_prof_data_ptr->db_data.cpctag == CPC_TAG_P_ASSERTED) ||
            (sipi_prof_data_ptr->db_data.cpctag == CPC_TAG_BOTH))
        {
            if (p_SipConEvntPtr->sipCpcStr.pres == PRSNT_NODEF)
            {
                p_asserted_id_string.append(";cpc=");
                p_asserted_id_string.append(
                    reinterpret_cast<char *>(p_SipConEvntPtr->sipCpcStr.val));
            }
            else /* Build ordinary cpc tag */
            {
                p_asserted_id_string.append(";cpc=ordinary");
            }
        }

        IMS_DLOG(IMS_IBCF_LOGLOW, "%s: Now source address: %s", __func__,
                 p_asserted_id_string.c_str());

        /* NOTE: REMOVED - gnumIwProcExt in sipi profile associating code  */

        /*
         * there is complex logic to descide how to make source address in original
         * mgc-8, to simple the functionality, we get the original address from sip
         * message(IBCF sip stack.
         */

        switch (vn_data_ptr->vn_type)
        {
        case IBCF_VIRTUAL_NETWORK_PEER:
            host_str = IMSpfed_get_peer_fed_port_ext_host_name(ngss_port_idx, msg_ptr);
            break;
        case IBCF_VIRTUAL_NETWORK_CORE:
            host_str = IMScfed_get_core_fed_port_ext_host_name(ngss_port_idx, &host_len, msg_ptr);
            break;
        default:
            IMS_DLOG(IMS_IBCF_LOGHIGH, "%s: Invalid network type peer/core(%d)", __func__,
                     vn_data_ptr->vn_type);
            return (IMS_FAIL);
        }

        if (IS_VALID_PTR(host_str) == FALSE)
        {
            IMS_DLOG(IMS_IBCF_LOGHIGH, "%s: can not get host string with fed", __func__);
            return (IMS_FAIL);
        }

        srcAddrStr.assign(host_str);

        /* FID15428, put RN "at" sign  ++ 17599 */
        if ((sipi_prof_data_ptr->db_data.jiprnmap == JIPRNMAP_PAID) &&
            (p_SipConEvntPtr->jurisInf.eh.pres == PRSNT_NODEF))
        {
            char strData[SIP_RN_LEN] = {0};

            sprintf(strData, "%d", p_SipConEvntPtr->jurisInf.addrSig1.val);
            sprintf(strData + 1, "%d", p_SipConEvntPtr->jurisInf.addrSig2.val);
            sprintf(strData + 2, "%d", p_SipConEvntPtr->jurisInf.addrSig3.val);
            sprintf(strData + 3, "%d", p_SipConEvntPtr->jurisInf.addrSig4.val);
            sprintf(strData + 4, "%d", p_SipConEvntPtr->jurisInf.addrSig5.val);
            sprintf(strData + 5, "%d", p_SipConEvntPtr->jurisInf.addrSig6.val);

            p_asserted_id_string.append(";rn=");
            p_asserted_id_string.append(strData, SIP_RN_LEN);
        }

        /* FID17364.0 SIP over TLS */
        if ((convertImsUri2IsupUri(uri_type) == SIP_TOKEN_SIP ||
             convertImsUri2IsupUri(uri_type) == SIP_TOKEN_SIPS) &&
            (p_SipConEvntPtr->oli.pres == PRSNT_NODEF) &&
            ((sipi_prof_data_ptr->db_data.olitag == OLI_PAID) ||
             (sipi_prof_data_ptr->db_data.olitag == OLI_BOTH))) /* FID 16466.0 */
        {
            char oliStr[4];
            memset(oliStr, 0, sizeof(oliStr));

            p_asserted_id_string.append(";oli=");
            sprintf(oliStr, "%02d", p_SipConEvntPtr->oli.val);
            p_asserted_id_string.append(oliStr);
            oliSet = TRUE;
        }

        /* FID17364.0 SIP over TLS */
        if ((convertImsUri2IsupUri(uri_type) == SIP_TOKEN_SIP) ||
            (convertImsUri2IsupUri(uri_type) == SIP_TOKEN_SIPS))
        {
            /* FID16205.0 Add calling party subaddress to isub parameter */
            if ((sipi_prof_data_ptr->db_data.sipisubmap == DBYES) &&
                (p_SipConEvntPtr->cgPtyIsubAddr.eh.pres) &&
                (p_SipConEvntPtr->cgPtyIsubAddr.addrInfo.pres))
            {
                char buff[128] = {0};
                sipa_add_isub(reinterpret_cast<char *>(p_SipConEvntPtr->cgPtyIsubAddr.addrInfo.val),
                              p_SipConEvntPtr->cgPtyIsubAddr.encode, 128, buff);
                p_asserted_id_string.append(buff);
            }
            /* end of FID 16205.0 */

            p_asserted_id_string.append("@");
            p_asserted_id_string.append(srcAddrStr);
        }
        else if (convertImsUri2IsupUri(uri_type) == SIP_TOKEN_TEL)
        {
            char buff[128] = {0};
            IBCF_iw_add_phone_context(buff, const_cast<char *>(srcAddrStr.c_str()), NULL, numType);
            p_asserted_id_string.append(buff);

            /* FID16205.0 Add calling party subaddress to isub parameter */
            if ((sipi_prof_data_ptr->db_data.sipisubmap == DBYES) &&
                (p_SipConEvntPtr->cgPtyIsubAddr.eh.pres) &&
                (p_SipConEvntPtr->cgPtyIsubAddr.addrInfo.pres))
            {
                char buff[128] = {0};
                sipa_add_isub(reinterpret_cast<char *>(p_SipConEvntPtr->cgPtyIsubAddr.addrInfo.val),
                              p_SipConEvntPtr->cgPtyIsubAddr.encode, 128, buff);
                p_asserted_id_string.append(buff);
            }
            /* end of FID 16205.0 */
        }

        if ((oliSet == FALSE) && (p_SipConEvntPtr->oli.pres == PRSNT_NODEF) &&
            ((sipi_prof_data_ptr->db_data.olitag == OLI_PAID) ||
             (sipi_prof_data_ptr->db_data.olitag == OLI_BOTH))) /* FID 16466.0 */
        {
            char oliStr[4] = {0};

            if ((sipi_prof_data_ptr->db_data.olitag == ISUPOLI_FROM) ||
                (sipi_prof_data_ptr->db_data.olitag == ISUPOLI_PAID) ||
                (sipi_prof_data_ptr->db_data.olitag == ISUPOLI_BOTH) ||
                (sipi_prof_data_ptr->db_data.olitag == ISUPOLI_NONE))
            {
                p_asserted_id_string.append(";isup-oli=");
                sprintf(oliStr, "%02d", p_SipConEvntPtr->oli.val);
                p_asserted_id_string.append(oliStr);
            }

            if ((sipi_prof_data_ptr->db_data.olitag == OLI_FROM) ||
                (sipi_prof_data_ptr->db_data.olitag == OLI_PAID) ||
                (sipi_prof_data_ptr->db_data.olitag == OLI_BOTH) ||
                (sipi_prof_data_ptr->db_data.olitag == OLI_NONE))
            {
                p_asserted_id_string.append(";oli=");
                sprintf(oliStr, "%02d", p_SipConEvntPtr->oli.val);
                p_asserted_id_string.append(oliStr);
            }
        }

        if (convertImsUri2IsupUri(uri_type) != SIP_TOKEN_TEL &&
            p_SipConEvntPtr->rpid.type == SIPA_TELE_SUB_PART)
        {
            p_asserted_id_string.append(";user=phone");
        }

        /* FID17364.0 SIP over TLS */
        if (IBCF_get_tg_transport_type(msg_ptr) == IBCF_TRANSPT_TCP)
        {
            p_asserted_id_string.append(";transport=tcp");
        }

        p_asserted_id_string.append(">");

        SIPia_DeleteHeaderContainer(event_ptr->m_ptr, SIPIA_P_ASSERTED_IDENTITY);
        (void)SIPia_addHeader_String(event_ptr->m_ptr, 
                                     SIPIA_P_ASSERTED_IDENTITY,
                                     p_asserted_id_string.c_str(),
                                     p_asserted_id_string.length()); 
    }

    /* Privacy header */
    if (privacyProcessed == TRUE || p_SipConEvntPtr->rpid.rpiPrivacy.pres == NOTPRSNT)
    {
        return (IMS_SUCCESS);
    }

    std::string privacy_value;
    switch (p_SipConEvntPtr->rpid.rpiPrivacy.val)
    {
    case SIPRPI_PRIVACY_FULL:
    case SIPRPI_PRIVACY_URI:
    case SIPRPI_PRIVACY_DUMMY_PAID:
        IMS_DLOG(IMS_IBCF_LOGLOW, "%s: Setting privacy to id with rpiPrivacy val = %d", __func__,
                 p_SipConEvntPtr->rpid.rpiPrivacy.val);

        privacy_value.assign("id");
        break;
    case SIPRPI_PRIVACY_OFF:
    case SIPRPI_PRIVACY_NAME: /*BUG65259 set privacy only based on calling
                               * number
                               * presentation
                                 */
    default:
        IMS_DLOG(IMS_IBCF_LOGLOW, "%s: Setting privacy to none with rpiPrivacy val = %d", __func__,
                 p_SipConEvntPtr->rpid.rpiPrivacy.val);

        privacy_value.assign("none");
        break;
    }

    /* now we need to remove the existing Privacy header before add one */
    SIPia_DeleteHeaderContainer(msg_ptr, SIPIA_PRIVACY);
    IBCFb2b_iw_add_privacy_value(msg_ptr, privacy_value);

    return (IMS_SUCCESS);
}

/* Function:      IBCFb2b_iw_build_rsc_priority
 *
 * Descriptions:  The function is used to build Resource-Priority header.
 *                If there is already a Resource-Priority header, just pass through without any modification
 *
 * Inputs:        p_SipConEvntPtr    - SipwLiConEvnt
 *                event_ptr          - output sip message
 *
 * Return:        IMS_SUCCESS/IMS_FAIL
 */
IMS_RETVAL
IBCFb2b_iw_build_rsc_priority(B2BUAEvent *event_ptr, SipwLiConEvnt *p_SipConEvntPtr)
{
    if (IS_VALID_PTR(event_ptr) == FALSE || IS_VALID_PTR(event_ptr->m_ptr) == FALSE ||
        IS_VALID_PTR(p_SipConEvntPtr) == FALSE)
    {
        IMS_DLOG(IMS_IBCF_LOGHIGH, "%s: invalid input, event_ptr(%p), p_SipConEvntPtr(%p)",
                 __func__, event_ptr, p_SipConEvntPtr);
        return (IMS_FAIL);
    }

    char resource_priority[MAX_RSC_PRIORITY_SIZE] = {0};
    IBCFb2b_iw_get_RPH_val(event_ptr->m_ptr, resource_priority, sizeof(resource_priority));
    if (strlen(resource_priority) > 0)
    {
        IMS_DLOG(IMS_IBCF_LOGLOW, "%s: pass through existing RPH header", __func__);
        return IMS_SUCCESS;
    }

    memset(resource_priority, 0, sizeof(resource_priority));

    IBCFb2b_iw_build_rsc_priority(&p_SipConEvntPtr->rscPriority, resource_priority);
    if (strlen(resource_priority) == 0)
    {
        IMS_DLOG(IMS_IBCF_LOGMED, "%s: build Resource-Priority Header string fails", __func__);
        return (IMS_SUCCESS); // Let the call continue
    }

    SIPia_DeleteHeaderContainer(event_ptr->m_ptr, SIPIA_RESOURCE_PRIORITY);
    SIPia_addHeader_String(event_ptr->m_ptr, SIPIA_RESOURCE_PRIORITY, resource_priority,
                           strlen(resource_priority), LIST_END);

    return (IMS_SUCCESS);
}

/* Function:      IBCFb2b_iw_build_sip_invite
 *
 * Descriptions:  The function is used to build SIP message for SIPT to SIP Call
 *
 * Inputs:        p_SipConEvntPtr  - SipwLiConEvnt
 *                event_ptr       - output sip message
 *
 * Return:        IMS_SUCCESS/IMS_FAIL
 */
IMS_RETVAL
IBCFb2b_iw_build_sip_invite(B2BUAEvent *event_ptr, SipwLiConEvnt *p_SipConEvntPtr)
{
    B2BUALegData *          leg_ptr         = NULL_PTR;
    B2BUALegData *          mate_ptr        = NULL_PTR;
    IBCF_TRUNK_GROUP_DATA * tg_data_ptr     = NULL_PTR;
    IBCF_SIP_PROFILE_DATA * sip_profile_ptr = NULL_PTR;
    IBCF_VN_TG_ID           tg_info         = {0};
    SIPIA_PORT_IDX          sip_port_idx    = 0;
    NGSS_PORT_INDX          port_idx        = 0;

    if (IS_VALID_PTR(event_ptr) == FALSE || IS_VALID_PTR(event_ptr->m_ptr) == FALSE ||
        IS_VALID_PTR(p_SipConEvntPtr) == FALSE)
    {
        IMS_DLOG(IMS_IBCF_LOGHIGH, "%s: invalid input, event_ptr(%p), p_SipConEvntPtr(%p)",
                 __func__, event_ptr, p_SipConEvntPtr);
        return (IMS_FAIL);
    }

    B2BUAEventToLegs(event_ptr, &leg_ptr, &mate_ptr);
    if (IS_VALID_PTR(leg_ptr) == FALSE || IS_VALID_PTR(mate_ptr) == FALSE)
    {
        IMS_DLOG(IMS_IBCF_LOGHIGH, "%s: Invalid leg_ptr %p or mate_ptr %p", __func__, leg_ptr,
                 mate_ptr);
        return (IMS_FAIL);
    }

    tg_info = IBCF_get_tginfo_from_leg(leg_ptr);
    if (IBCF_is_valid_vn_tg(tg_info) == FALSE)
    {
        IMS_DLOG(IMS_IBCF_LOGHIGH, "%s: Invalide VN(%u) or TG(%u) from leg_ptr(%p)", __func__,
                 tg_info.vn_id, tg_info.tg_id, leg_ptr);
        return (IMS_FAIL);
    }

    tg_data_ptr = IBCF_get_tg_data_by_id(tg_info.vn_id, tg_info.tg_id);
    if (IS_VALID_PTR(tg_data_ptr) == FALSE)
    {
        IMS_DLOG(IMS_IBCF_LOGHIGH, "%s: could not found  tg_data_ptr with vn(%d)/tg(%d)", __func__,
                 tg_info.vn_id, tg_info.tg_id);
        return (IMS_FAIL);
    }

    sip_profile_ptr = IBCF_get_sip_profile_data_by_id(tg_data_ptr->sip_profile_id);
    if (IS_VALID_PTR(sip_profile_ptr) == FALSE)
    {
        IMS_DLOG(IMS_IBCF_LOGHIGH, "%s: could not found  sip_profile with vn(%d)/tg(%d)", __func__,
                 tg_info.vn_id, tg_info.tg_id);
        return (IMS_FAIL);
    }

    /* Get SIP stack port index. */
    sip_port_idx = B2BCutil_get_sipia_port_idx(event_ptr->leg_ptr);
    if (IS_VALID_SIPIA_PORT_IDX(sip_port_idx) == FALSE)
    {
        IMS_DLOG(IMS_IBCF_LOGHIGH, "%s: invalid sip_port_idx=[%u] from SIP "
                                   "message type (request or response)=[%d].",
                 __func__, sip_port_idx, SIPia_getMsgType(event_ptr->m_ptr));

        return (IMS_FAIL);
    }

    /* Get provisioned SIPia port index. */
    port_idx = SSgul_get_sipia_port_prov_key(sip_port_idx);
    if (port_idx == NGSS_DEFAULT_INDX)
    {
        IMS_DLOG(IMS_IBCF_LOGHIGH, "%s: invalid port_idx=[%d].", __func__, port_idx);

        return (IMS_FAIL);
    }

    // FCI M-bit       |     fciNpdiIwProc         |   "npdi" in R-URI
    /* update R-URI */
    // TNS             |                           |   "cic" in Request-URI
    // CdPN |(Append CC or CC+NDC if not Intl. E.164 num?) |Request-URI(userinfo) /To (addr-spec)
    // CSI             |   dai (Diale Around Info) |   "dai" in Request-URI(sip & tel)
    IBCFb2b_iw_build_uri(event_ptr, p_SipConEvntPtr);

    // NCI                                         |   procedure #1 #2
    /* Bits "DC" in Nature of Connection Indicator parameter should always be set to
     * "continuity check not required" (0x00) in the ISUP IAM encapsulated in egress SIP-I INVITE
     * in the first pass ppt, No actions shall be taken even bits "DC" are set inappropriately.
     */
    /* For ingress SIP-I INVITE with appropriate ISUP IAM encapsulated, SBC shall not check bit "E"
     * in Nature of Connection indicator parameter for determining if Echo Cancellation should be
     * activated for the call.
     */

    // CPC             |   cpcTag|rphEmergencyMap  |   "cpc"
    // in From/PAID/Accept-contact(language for oper call) | Resource-Priority Hdr (ets.x)
    // OLI             | oliTag                    |   "oli" or "isup-oli" in From/PAID
    // ATP (Calling Sub-address)   |   sipIsubMap  |   "isub"&"isub-encoding" in From/PAID URI
    // JIP             |   jipRnMap                |   "rn" in PAID
    // Generic Number/GAP| gnumIwProc              |   From
    // CgPN            |   contactIncludeCgpn      |   PAID/From/Privacy | Contact
    /* Contact Header will be updated in the point of IBCFb2b_updateContact()
     * IBCFb2b_populate_contact_buf()
     */
    switch (p_SipConEvntPtr->pAssertedId)
    {
    case SIP_PRIVACY_P_ASSERT:
        if (IBCFb2b_iw_build_p_asserted_id(event_ptr, p_SipConEvntPtr) == IMS_FAIL)
        {
            IMS_DLOG(IMS_IBCF_LOGMED, "%s(): buid P_Asserted_ID header failed", 
                     __func__);
            /* Move on to build other headers */
        }
        break;
    case SIP_PRIVACY_RMTPRTY_ID:
        if (IBCFb2b_iw_build_rpid(p_SipConEvntPtr, event_ptr) == IMS_FAIL)
        {    
            IMS_DLOG(IMS_IBCF_LOGMED, "%s(): build Single Remote-Party-ID header failed",
                     __func__);
            /* Move on to build other headers */
        }
        break;
    default:
        IMS_DLOG(IMS_IBCF_LOGMED, "%s(): Invalid pAssertedId parameter[%d] in ConEvnt",
                __func__, p_SipConEvntPtr->pAssertedId);
        /* Move on to build other headers */
        break;
    } /* End of switch(p_SipConEvntPtr->pAssertedId) */

    /* If multiple Remote-Party-ID header is enabled/supported,
     * build Remote-Party-ID header to carry Called & Redirect
     * party information.
     */
    if (IMSdc_get_ibcf_sip_sipi_interworking_profile_multirpidmap(tg_data_ptr->sipi_profile_id))
    {
        /* If Call Diversion data is available, build Remote-Party-ID
         * header from it.
         */
         if (IBCFb2b_iw_build_called_redirect_rpid(p_SipConEvntPtr, event_ptr) == IMS_FAIL)
         {
             IMS_DLOG(IMS_IBCF_LOGMED, "%s(): build Multiple Remote-Party-ID headers failed",
                      __func__);
             /* Move on to build other headers */
         }
    }

    IBCFb2b_iw_build_from(p_SipConEvntPtr, event_ptr);

    if (IBCFb2b_iw_build_AcceptContact(event_ptr->m_ptr, p_SipConEvntPtr) == IMS_FAIL)
    {
        IMS_DLOG(IMS_IBCF_LOGLOW, "%s: buid AcceptContact Header fails", __func__);
        return (IMS_FAIL);
    }

    // MLPP            |   mlppWpsMap              |   RPH(wps.x)
    /* should have been covered by IBCFb2b_iw_build_rsc_priority, mlppWpsMap
     * is process inside GCC
     */
    if (IBCFb2b_iw_build_rsc_priority(event_ptr, p_SipConEvntPtr) == IMS_FAIL)
    {
        IMS_DLOG(IMS_IBCF_LOGHIGH, "%s: build Resource-Priority Header fails", __func__);
        return (IMS_FAIL);
    }

    // TMR/USI         |                           |   SDP
    /* NOTE: for the first pass, TMR/USI's impact on SDP will not be done */

    // Hop Counter     |   maxFwdhopCntrRatio      |   MaxForwards (procedure #4)
    if (IBCFb2b_iw_build_maxforward(event_ptr, p_SipConEvntPtr) == IMS_FAIL)
    {
        IMS_DLOG(IMS_IBCF_LOGLOW, "%s: buid Max-Forwards Header fails", __func__);
        return (IMS_FAIL);
    }

    // Charge Number   |   pDcsBilling | pChargeInfoHdr | ogChrgNum |
    // P-DCS-Billing-Info, charge | P-Chg-Info, npi, noa
    if (IBCFb2b_iw_build_charging_hdr(event_ptr, p_SipConEvntPtr) == IMS_FAIL)
    {
        IMS_DLOG(IMS_IBCF_LOGLOW, "%s: buid P-Charge-Info/P-DCS-Billing-Info "
                                  "Header fails", __func__);
        return (IMS_FAIL);
    }

    // OFCI (CUG Call Ind.)| cugIwProc             |   Procedure #5
    /* first pass Procedure #5 seems to be impleted in GCC? */

    // User-to-User Ind./User-to-User Info|
    // uuiIwProc/sipUuiMap/sipUuiFormat|User-to-User Hdr (Procedure #6)
    if (IBCFb2b_iw_build_uui(p_SipConEvntPtr, event_ptr) == IMS_FAIL)
    {
        IMS_DLOG(IMS_IBCF_LOGLOW, "%s: buid USERTOUSER Header fails ", __func__);
        return (IMS_FAIL);
    }

    // Redirecting Num. Redirection Info|  divInfo/maxHistHdrCnt| History-Info,Diversion
    if (IBCFb2b_iw_build_diversion(p_SipConEvntPtr, event_ptr, FALSE) == IMS_FAIL)
    {
        IMS_DLOG(IMS_IBCF_LOGLOW, "%s: buid Diversion Header fails", __func__);
        return (IMS_FAIL);
    }

    /* Currently comment the history info logic. Will develope the logic in later release. */
    //if (IBCFb2b_iw_build_hist_info_for_rq(p_SipConEvntPtr, event_ptr) == IMS_FAIL)
    //{
    //   IMS_DLOG(IMS_IBCF_LOGLOW, "%s: buid History-Info Header fails ", __func__);
    //   return (IMS_FAIL);
    //}

    // Original Called Number | GAP|ocnIwProc | lnpToHdr   |   To
    if (IBCFb2b_iw_build_TO(p_SipConEvntPtr, event_ptr) == IMS_FAIL)
    {
        IMS_DLOG(IMS_IBCF_LOGLOW, "%s: buid TO Header fails", __func__);
        return (IMS_FAIL);
    }

    // Original Called Number|         pCdpId      |   P-Called-Party-ID
    if (IBCF_iw_build_called_party_id(event_ptr, p_SipConEvntPtr, port_idx,
                                      convertImsUri2IsupUri(sip_profile_ptr->uri_type)) == IMS_FAIL)
    {
        IMS_DLOG(IMS_IBCF_LOGLOW, "%s: buid P-Called-Party-Id Header fails", __func__);
        return (IMS_FAIL);
    }

    // CgPN used to include in Contact 
    IBCFb2b_iw_build_contact(event_ptr, p_SipConEvntPtr);

    IBCFb2b_iw_build_XOFCI(event_ptr, p_SipConEvntPtr, event_ptr->m_ptr);
    IBCFb2b_iw_build_XFCI(event_ptr, p_SipConEvntPtr, event_ptr->m_ptr);

    // CgPN, CN, OCN, GN, RN, PRESNUM | presRestrictList|   Delete or pass through
    /* PRESNUM-   Presentation Number */
    /* OCN    -   Original Called Number */
    /* RN     -   Redirecting Number */
    /* GN     -   Generic Number */
    /* CgPN   -   Calling Party Number */
    /* GAP    -   Generic Number, same configure value */
    /* GCC will cover this part and do the remove or pass through */

    return (IMS_SUCCESS);
}

/* Function:      IBCFb2b_iw_build_sipt_invite
 *
 * Descriptions:  The function is used to build SIP message for SIP to SIPT Call
 *
 * Inputs:        p_SipConEvntPtr   - SipwLiConEvnt
 *                event_ptr         - output sip message
 *
 * Return:        IMS_SUCCESS/IMS_FAIL
 */
IMS_RETVAL
IBCFb2b_iw_build_sipt_invite(B2BUAEvent *event_ptr, SipwLiConEvnt *p_SipConEvntPtr)
{
    B2BUALegData *          leg_ptr         = NULL_PTR;
    B2BUALegData *          mate_ptr        = NULL_PTR;
    IBCF_TRUNK_GROUP_DATA * tg_data_ptr     = NULL_PTR;
    IBCF_SIP_PROFILE_DATA * sip_profile_ptr = NULL_PTR;
    IBCF_VN_TG_ID           tg_info         = {0};
    SIPIA_PORT_IDX          sip_port_idx    = 0;
    NGSS_PORT_INDX          port_idx        = 0;
    const char *            call_type_str   = "sip2sipt\/sipt2sipt";

    if (IS_VALID_PTR(event_ptr) == FALSE || IS_VALID_PTR(event_ptr->m_ptr) == FALSE ||
        IS_VALID_PTR(p_SipConEvntPtr) == FALSE)
    {
        IMS_DLOG(IMS_IBCF_LOGHIGH, "%s: invalid input, event_ptr(%p), p_SipConEvntPtr(%p)",
                 __func__, event_ptr, p_SipConEvntPtr);
        return (IMS_FAIL);
    }

    B2BUAEventToLegs(event_ptr, &leg_ptr, &mate_ptr);
    if (IS_VALID_PTR(leg_ptr) == FALSE || IS_VALID_PTR(mate_ptr) == FALSE)
    {
        IMS_DLOG(IMS_IBCF_LOGHIGH, "%s: Invalid leg_ptr %p or mate_ptr %p", __func__, leg_ptr,
                 mate_ptr);
        return (IMS_FAIL);
    }

    tg_info = IBCF_get_tginfo_from_leg(leg_ptr);
    if (IBCF_is_valid_vn_tg(tg_info) == FALSE)
    {
        IMS_DLOG(IMS_IBCF_LOGHIGH, "%s: Invalide VN(%u) or TG(%u) from leg_ptr(%p)", __func__,
                 tg_info.vn_id, tg_info.tg_id, leg_ptr);
        return (IMS_FAIL);
    }

    tg_data_ptr = IBCF_get_tg_data_by_id(tg_info.vn_id, tg_info.tg_id);
    if (IS_VALID_PTR(tg_data_ptr) == FALSE)
    {
        IMS_DLOG(IMS_IBCF_LOGHIGH, "%s: could not found  tg_data_ptr with vn(%d)/tg(%d)", __func__,
                 tg_info.vn_id, tg_info.tg_id);
        return (IMS_FAIL);
    }

    sip_profile_ptr = IBCF_get_sip_profile_data_by_id(tg_data_ptr->sip_profile_id);
    if (IS_VALID_PTR(sip_profile_ptr) == FALSE)
    {
        IMS_DLOG(IMS_IBCF_LOGHIGH, "%s: could not found  sip_profile with vn(%d)/tg(%d)", __func__,
                 tg_info.vn_id, tg_info.tg_id);
        return (IMS_FAIL);
    }

    /* Get SIP stack port index. */
    sip_port_idx = B2BCutil_get_sipia_port_idx(event_ptr->leg_ptr);
    if (IS_VALID_SIPIA_PORT_IDX(sip_port_idx) == FALSE)
    {
        IMS_DLOG(IMS_IBCF_LOGHIGH, "%s: invalid sip_port_idx=[%u] from SIP "
                                   "message type (request or response)=[%d].",
                 __func__, sip_port_idx, SIPia_getMsgType(event_ptr->m_ptr));

        return (IMS_FAIL);
    }

    /* Get provisioned SIPia port index. */
    port_idx = SSgul_get_sipia_port_prov_key(sip_port_idx);
    if (port_idx == NGSS_DEFAULT_INDX)
    {
        IMS_DLOG(IMS_IBCF_LOGHIGH, "%s: invalid port_idx=[%d].", __func__, port_idx);

        return (IMS_FAIL);
    }

    /* the Indicator should has already generated/updated by GCC, only need to add ISUP body to SIP
     * message later somewhere
     */
    // SIP Hdrs/Procedure    -->    ISUP Param                      <--     MGC-8 Opts/settings
    // "npdi" in Request-URI -->    Forward Call Indicator | M bit	<--     sipFciIwProc
    //  procedure #1, #2, #3 -->    Nature of Connection Indicator  <--     nciIwProc
    // "dai"                 -->    Carrier Selection Information   <--

    //                       -->    TMR/USI                         <--
    /* when talking about TMR/USI, information could get from:
     * https://tools.ietf.org/id/draft-ietf-sip-isup-03.txt
     * Five mandatory parameters appear within the IAM message: the Called
     * Party Number (CPN), the Nature of Connection Indicator (NCI), the
     * Forward Call Indicators (FCI), the Calling Party's Category (CPC),
     * and finally a parameter that indicates the desired bearer
     * characteristics of the call - in some ISUP variants the Transmission
     * Medium Requirement (TMR) is required, in others the User Service
     * Information (USI) (or both)
     */

    //                                  -->    Optional FCI               <-- colpForSIP
    /* Optional FCI means -- Optional Forward Call Indicator. The Optional Forward Call Indicator
     * (Optional FCI) is used to request that the ISUP side send the Connected Number parameter in
     * its CON for SIP to SIPI Call
     */

    // "cpc" in From/PAID/Accept-Contact-->  CPC (PAID preferred if both) <--
    // Request-URI/"rn"                 -->  CdPN                         <--
    // "cic" in Request-URI             -->  CIP/TNS                      <--
    // From/Privacy | Request-URI       -->  Generic Number/GAP           <-- gnumIwProc
    // PAID/Privacy/Remote-Party-ID     -->  CgPN                         <--
    // "oli" or "isup-oli" in From/PAID -->  OLI                          <--
    // "isub" & "isub-encoding" in From/PAID --> ATP (Calling Subaddress) <-- sipIsubMap
    // "rn" in PAID                     -->             JIP               <--
    IBCFb2b_iw_build_uri(event_ptr, p_SipConEvntPtr);
    //IBCFb2b_iw_build_from(p_SipConEvntPtr, event_ptr);

    switch (p_SipConEvntPtr->pAssertedId)
    {
    case SIP_PRIVACY_P_ASSERT:
        if (IBCFb2b_iw_build_p_asserted_id(event_ptr, p_SipConEvntPtr) == IMS_FAIL)
        {
            IMS_DLOG(IMS_IBCF_LOGMED, "%s(): buid P_Asserted_ID header failed for %s call", 
                     __func__, call_type_str);
            /* Move on to build other headers */
        }
        break;
    case SIP_PRIVACY_RMTPRTY_ID:
        if (IBCFb2b_iw_build_rpid(p_SipConEvntPtr, event_ptr) == IMS_FAIL)
        {    
            IMS_DLOG(IMS_IBCF_LOGMED, "%s(): build single Remote-Party-ID header failed for %s call",
                     __func__, call_type_str);
            /* Move on to build other headers */
        }
        break;
    default:
        IMS_DLOG(IMS_IBCF_LOGMED, "%s(): Invalid pAssertedId parameter[%d] in ConEvnt",
                __func__, p_SipConEvntPtr->pAssertedId);
        /* Move on to build other headers */
        break;
    } /* End of switch(p_SipConEvntPtr->pAssertedId) */

    /* If multiple Remote-Party-ID header is enabled/supported,
     * build Remote-Party-ID header to carry Called & Redirect
     * party information.
     */
    if (IMSdc_get_ibcf_sip_sipi_interworking_profile_multirpidmap(tg_data_ptr->sipi_profile_id))
    {
        /* If Call Diversion data is available, build Remote-Party-ID
         * header from it.
         */
         if (IBCFb2b_iw_build_called_redirect_rpid(p_SipConEvntPtr, event_ptr) == IMS_FAIL)
         {
             IMS_DLOG(IMS_IBCF_LOGMED, "%s(): build Multiple Remote-Party-ID headers failed for %s call",
                      __func__, call_type_str);
             /* Move on to build other headers */
         }
    }

    if (IBCFb2b_iw_build_AcceptContact(event_ptr->m_ptr, p_SipConEvntPtr) == IMS_FAIL)
    {
        IMS_DLOG(IMS_IBCF_LOGLOW, "%s: buid AcceptContact Header fails for %s call", __func__,
                 call_type_str);
        return (IMS_FAIL);
    }

    // MaxForwards        -->          Hop Counter                  <-- maxFwdhopCntrRatio
    if (IBCFb2b_iw_build_maxforward(event_ptr, p_SipConEvntPtr) == IMS_FAIL)
    {
        IMS_DLOG(IMS_IBCF_LOGLOW, "%s: buid Max-Forwards Header fails for %s call", __func__,
                 call_type_str);
        return (IMS_FAIL);
    }

    // User-to-User hdr   -->         User-to-User Info             <-- sipUuiMap
    if (IBCFb2b_iw_build_uui(p_SipConEvntPtr, event_ptr) == IMS_FAIL)
    {
        IMS_DLOG(IMS_IBCF_LOGLOW, "%s: buid USERTOUSER Header fails for %s call", __func__,
                 call_type_str);
        return (IMS_FAIL);
    }

    // P-DCS-Billing-Info, charge; P-Chg-Info, -->   Charge Number  <--
    if (IBCFb2b_iw_build_charging_hdr(event_ptr, p_SipConEvntPtr) == IMS_FAIL)
    {
        IMS_DLOG(IMS_IBCF_LOGLOW,
                 "%s: buid P-Charge-Info/P-DCS-Billing-Info Header fails for %s call", __func__,
                 call_type_str);
        return (IMS_FAIL);
    }

    // To (Req-URI != To)/Bot-level Diversion/P-Called-Party-ID --> Original Called Number <--
    if (IBCFb2b_iw_build_TO(p_SipConEvntPtr, event_ptr) == IMS_FAIL)
    {
        IMS_DLOG(IMS_IBCF_LOGLOW, "%s: buid TO Header fails for  %s call", __func__, call_type_str);
        return (IMS_FAIL);
    }

    if (IBCF_iw_build_called_party_id(event_ptr, p_SipConEvntPtr, port_idx,
                                      convertImsUri2IsupUri(sip_profile_ptr->uri_type)) == IMS_FAIL)
    {
        IMS_DLOG(IMS_IBCF_LOGLOW, "%s: buid P-Called-Party-Id Header fails for %s call", __func__,
                 call_type_str);
        return (IMS_FAIL);
    }

    if (IBCFb2b_iw_build_diversion(p_SipConEvntPtr, event_ptr, FALSE) == IMS_FAIL)
    {
        IMS_DLOG(IMS_IBCF_LOGLOW, "%s: buid Diversion Header fails for %s call", __func__,
                 call_type_str);
        return (IMS_FAIL);
    }

    // History-Info, Top-level Diversion--> Redirecting Num. Redirection Info <--
    /* Currently comment the history info logic. Will develope the logic in later release. */
    //if (IBCFb2b_iw_build_hist_info_for_rq(p_SipConEvntPtr, event_ptr) == IMS_FAIL)
    //{
    //   IMS_DLOG(IMS_IBCF_LOGLOW, "%s: buid History-Info Header fails for %s call", __func__,
    //           call_type_str);
    //   return (IMS_FAIL);
    //}

    // Resource Priority Hdr (wps.x)  -->    MLPP              <--     MlppWpsMap
    if (IBCFb2b_iw_build_rsc_priority(event_ptr, p_SipConEvntPtr) == IMS_FAIL)
    {
        IMS_DLOG(IMS_IBCF_LOGHIGH, "%s: build Resource-Priority Header fails", __func__);
        return (IMS_FAIL);
    }

    return (IMS_SUCCESS);
}

IMS_RETVAL IBCFb2b_iw_build_bye(B2BUAEvent *event_ptr, SipwLiRelEvnt *sipwLiRelEvnt)
{
	IMS_RETVAL rc;

	// build reason header
	IBCFb2b_iw_build_reason(event_ptr, &(sipwLiRelEvnt->reason));

	// build UUI
	SipwLiConEvnt p_conEvntPtr;
	p_conEvntPtr.sipUuiData = sipwLiRelEvnt->sipUuiData;
	/* disconnect the memory ownership */
	sipwLiRelEvnt->sipUuiData.pres = NOTPRSNT;
	sipwLiRelEvnt->sipUuiData.len = 0;
	sipwLiRelEvnt->sipUuiData.val = NULL;
	rc = IBCFb2b_iw_build_uui(&p_conEvntPtr, event_ptr);
	

	return rc;
}

/*
 *  Function: IBCFb2b_iw_build_hi_entry
 *
 *  Description: This function will build a History-Info header entry
 *
 *  Output: Build History-Info header entry
 *
 *  Return: IMS_SUCCESS -- Successfully build History-Info header entry
 *  	    IMS_FAIL    -- Failed to build History-Info header entry
 */
IMS_RETVAL 
IBCFb2b_iw_build_hi_entry(SIPia_Msg *sip_msg_ptr, char *hist_ptr, long max_len, TknStrE *ruri_ptr, TknU16 *cause_ptr, TknU8 *privacy_ptr, TknStr *index_ptr, int *len_ptr)
{
	char parm[32];
	char *tmp_ptr;
	int  len = 0;
	bool ruri_hdr_pres = FALSE;
	BOOL header_add_ret = FALSE;

	/* check input parameters */
	if ((IS_VALID_PTR(hist_ptr) == FALSE) || (max_len == 0) 
		|| (IS_VALID_PTR(ruri_ptr) == FALSE) || (IS_VALID_PTR(index_ptr) == FALSE))
	{
		IMS_DLOG(IMS_IBCF_LOGHIGH, "%s(): Invalid input parameters", __func__);
		return IMS_FAIL;
	}

	if ((ruri_ptr->pres != PRSNT_NODEF) || (index_ptr->pres != PRSNT_NODEF)
		|| (ruri_ptr->len == 0) || (index_ptr->len == 0))
	{
		IMS_DLOG(IMS_IBCF_LOGHIGH, "%s(): Mandatory parameter missed for building Hist-Info entry", __func__);
		return IMS_FAIL;
	}

	/* clear the output string */
	memset(hist_ptr, 0, max_len);

	/* make sure buffer is not overflow 
	 * '64' is the max length of other characters 
	 */
	len = ruri_ptr->len + index_ptr->len + 64; 

	if (len > max_len)
	{
		IMS_DLOG(IMS_IBCF_LOGHIGH, "%s(): No enough memory for build the Hist-Info entry", __func__);
		return IMS_FAIL;
	}

	strcat(hist_ptr, "<");
	strcat(hist_ptr, reinterpret_cast<const char*>(ruri_ptr->val));

	tmp_ptr = strchr(reinterpret_cast<char *>(ruri_ptr->val), SIP_ASCII_QUESTION_MARK);
	if (IS_VALID_PTR(tmp_ptr) == FALSE)
	{
		ruri_hdr_pres = FALSE;
	}
	else
	{
		ruri_hdr_pres = TRUE;
	}

	if ((IS_VALID_PTR(cause_ptr) == TRUE) && (cause_ptr->pres == PRSNT_NODEF)) 
	{
		int resp_code = sipa_get_hist_info_cause(cause_ptr->val);
		if ((resp_code != SIP_E_FAIL) || (cause_ptr->val >= SIPCAUSE_301_MOVEDP))
		{
			if (ruri_hdr_pres)
			{
				strcat(hist_ptr, "&Reason=SIP");
			}
			else
			{
				strcat(hist_ptr, "?Reason=SIP");
				ruri_hdr_pres = TRUE;
			}
			snprintf(parm, sizeof(parm)-1, "%%3Bcause%%3D%d", 
				(resp_code != SIP_E_FAIL) ? resp_code : cause_ptr->val);
			strncat(hist_ptr, parm, strlen(parm));
		}
	} /* End if ((IS_VALID_PTR(cause_ptr) && (cause_ptr->pres == PRSNT_NODEF)) */

	if ((IS_VALID_PTR(privacy_ptr) == TRUE) && (privacy_ptr->pres == PRSNT_NODEF)
		&& (privacy_ptr->val != SIPDIV_PRIVACY_OFF))
	{
		if (ruri_hdr_pres)
		{
			strcat(hist_ptr, "&Privacy=history");
		}
		else
		{
			strcat(hist_ptr, "?Privacy=history");
		}
	}

	strcat(hist_ptr, ">;index=");
	strcat(hist_ptr, reinterpret_cast<const char*>(index_ptr->val));

	len = strlen(hist_ptr);
	IMS_DLOG(IMS_IBCF_LOGMED, "%s(): Successfully build Hist-Info entry: %s", __func__, hist_ptr);

	*len_ptr = len;

	header_add_ret = SIPia_addHeader_String(sip_msg_ptr,
			SIPIA_HISTORY_INFO, hist_ptr, len, LIST_END);
	
	if( TRUE == header_add_ret )
	{
		IMS_DLOG(IMS_IBCF_LOGLOW, "%s returns: "
					"add the hist_ptr [%s] success.", hist_ptr);
			
		IMS_DLOG_SIP_MSG(IMS_IBCF_LOGLOW, sip_msg_ptr, "%s(): new sip message is.",__func__);	
	}

	return IMS_SUCCESS;
} /* End of function IBCFb2b_iw_build_hi_entry */

/*
 * Function: IBCFb2b_iw_build_hist_info_for_rq
 *
 * Description: This function is called to build History-Info header in
 * 		SIP request message. History-Info header contents is under
 * 		control of privacy rule. This function will just build
 * 		History-Info header. The privacy rule will be enforced
 * 		by IBCF/SBC Privacy service later before sending the message
 * 		out. As to THIG, count on FED to do it. The re-route will 
 * 		be considered later.
 *
 * 		Function will be called for
 * 		 - building new INVITE when 301, 302, 305 or 422 response is received
 * 		 - process Conn. Req. Event from GCC
 *
 * Output:  Build History-Info header in SIP message
 *
 * Return:  IMS_SUCCESS   -- Successfully built History-Info header
 * 	    IMS_FAIL	  -- Failed to build History-Info header
 */
IMS_RETVAL 
IBCFb2b_iw_build_hist_info_for_rq(SipwLiConEvnt *p_SipConEvntPtr, B2BUAEvent *event_ptr)
{
	URI		*uri_ptr = NULL;
	SipHistInfo	*histInfo_ptr = NULL;
	SipDiversion	*div_ptr = NULL;
	Sip_URI		*tmp_uri_ptr = NULL;
	DString 	*temp_ds_ptr = NULL;
	TknStr		nh_index;
	TknStrE		nh_ruri;
	char		hi_entry[SIPA_MAX_HI_ENTRY_LEN];
	char		hi_str[SIPA_MAX_HIST_HDR_LEN];
	char		indexStr[140];  /* Klocwork issue 25323 of bug79550 */
	int		len = 0;
	int		hist_hdr_len = 0;
	int		tmp_len = 0;
	IMS_RETVAL	ret_val;

	/* check input parameters */
	if ((IS_VALID_PTR(p_SipConEvntPtr) == FALSE) || (IS_VALID_PTR(event_ptr) == FALSE))
	{
		IMS_DLOG(IMS_IBCF_LOGHIGH, "%s(): Invalid input parameters", __func__);
		return IMS_FAIL;
	}

	histInfo_ptr = &(p_SipConEvntPtr->histInfo);
	div_ptr = &(p_SipConEvntPtr->div);

	/* set the data for next hop hi_entry */
	nh_index.pres = PRSNT_NODEF;
	nh_index.len = 1;
	strcpy(reinterpret_cast<char *>(nh_index.val), "1");

	nh_ruri.pres = PRSNT_NODEF;

	SIP_TopLine *topline_ptr = SIPia_getTopLine(event_ptr->m_ptr);
	if (IS_VALID_PTR(topline_ptr))
	{
		uri_ptr = SIPia_TopLineReq_getURI(topline_ptr);
		if (IS_VALID_PTR(uri_ptr) == FALSE)
		{
			IMS_DLOG(IMS_IBCF_LOGHIGH, "%s(): Invalid input parameters", __func__);
			return IMS_FAIL;
		}
	}

	if ((uri_ptr->type == SIP_URI) ||
		(uri_ptr->type == SIPS_URI))
	{
		tmp_uri_ptr = URI_getSipUri(uri_ptr);
		temp_ds_ptr = Sip_URI_getRawTextURI(event_ptr->m_ptr, tmp_uri_ptr);
		CPsip_DString_to_String(temp_ds_ptr, reinterpret_cast<char *>(nh_ruri.val), sizeof(nh_ruri.val));

		nh_ruri.len = DString_getLen(temp_ds_ptr);
		
		IMS_DLOG(IMS_IBCF_LOGLOW, "%s(): nh_ruri.val is %s, len is %d.",
				__func__, reinterpret_cast<char *>(nh_ruri.val), nh_ruri.len);
	}
	else if (uri_ptr->type == TEL_URI)
	{
		IMS_DLOG(IMS_IBCF_LOGHIGH, "%s(): TEL URI is used. Don't know how to put it into History-Info?", __func__);
		return IMS_FAIL;
	}

	nh_ruri.val[sizeof(nh_index.val)-1] = 0;

	/* SIP Hist-Info to SIP Hist-Info mapping */
	if ((IS_VALID_PTR(histInfo_ptr) == TRUE) && (histInfo_ptr->eh.pres == PRSNT_NODEF))
	{
		if ((histInfo_ptr->index.pres != PRSNT_NODEF)
			&&(histInfo_ptr->index.len == 0))
		{
			IMS_DLOG(IMS_IBCF_LOGHIGH, "%s(): Mandatory Index value missed", __func__);
			return IMS_FAIL;
		}

		/* 78704 if the histInfo->index appears in cc_hist string, this must be a reroute*/
		memset(indexStr, 0 , sizeof(indexStr));
       
		/* Bug94830, to check ccReroute, the index should add one more .1 */
		if( strlen(reinterpret_cast<const char*>(histInfo_ptr->index.val)) + strlen("index=") + 2 < sizeof(indexStr))
		{
			sprintf(indexStr, "index=%s.1", histInfo_ptr->index.val); /* Bug94830 */
			strcat(reinterpret_cast<char *>(histInfo_ptr->index.val), ".1");
			histInfo_ptr->index.len += 2;
		}
		else
		{
			IMS_DLOG(IMS_IBCF_LOGHIGH, "%s(): length overflow of local indexStr", __func__);
			return IMS_FAIL;
		}

		ret_val = IBCFb2b_iw_build_hi_entries_by_cchistr(hi_str, sizeof(hi_str), histInfo_ptr, &len);
		if (ret_val == IMS_FAIL)
		{
			IMS_DLOG(IMS_IBCF_LOGHIGH, "%s(): IBCFb2b_iw_build_hi_entries_by_cchistr failed", __func__);
			return IMS_FAIL;
		}

		if ((histInfo_ptr->privacy.pres != PRSNT_NODEF) 
			|| (histInfo_ptr->privacy.val != SIPRPI_PRIVACY_FULL))
		{
			TknU16 cause;
			cause.pres = histInfo_ptr->cause.pres;
			cause.val = histInfo_ptr->cause.val;
			ret_val = IBCFb2b_iw_build_hi_entry(event_ptr->m_ptr, hi_entry, sizeof(hi_entry),
				&histInfo_ptr->ruri, &cause, &histInfo_ptr->privacy, 
				&histInfo_ptr->index, &tmp_len);

			if (ret_val == IMS_FAIL)
			{
				IMS_DLOG(IMS_IBCF_LOGHIGH, "%s(): IBCFb2b_iw_build_hi_entry failed", __func__);
				return IMS_FAIL;
			}

			if (len > 0)
			{
				strcat(hi_str, ",");
				len ++;
			}

			strncat(hi_str, hi_entry, strlen(hi_entry));
			len += tmp_len;
		}
		else
		{
			IMS_DLOG(IMS_IBCF_LOGHIGH, "%s(): Not add hi_entry due to privacy policy", __func__);
		}

		memcpy(&nh_index, &histInfo_ptr->index, sizeof(nh_index));
		strcat(reinterpret_cast<char *>(nh_index.val), ".1");
	}
	else if ((div_ptr) && (div_ptr->eh.pres == PRSNT_NODEF))
	{
		/* PSTN or SIP Div to SIP Hist-Info mapping. */
		char curr_index[SIPA_MAX_HIST_IDX_LEN];
		int   histInfoFmt = HISTINFOFMT_DEFAULT;

		memset(curr_index, 0, sizeof(curr_index));

		ret_val = IBCFb2b_iw_build_hi_entries_by_div(hi_str, SIPA_MAX_HIST_HDR_LEN, 
				div_ptr, curr_index, event_ptr, &len);

		if (ret_val == IMS_FAIL)
		{
			IMS_DLOG(IMS_IBCF_LOGHIGH, "%s(): IBCFb2b_iw_build_hi_entries_by_div() failed.", __func__);
			return IMS_FAIL;
		}

		strcpy(reinterpret_cast<char *>(nh_index.val), curr_index);
		if (strlen(reinterpret_cast<const char *>(nh_index.val)) > 0)
		{
			strcat(reinterpret_cast<char *>(nh_index.val), ".1");
		}
		else
		{
			strcat(reinterpret_cast<char *>(nh_index.val), "1");
		}
			nh_index.len = strlen(reinterpret_cast<char *>(nh_index.val));
			nh_index.pres = PRSNT_NODEF;
	}

	/* Add the hi_entry for the next hop */
	ret_val = IBCFb2b_iw_build_hi_entry(event_ptr->m_ptr, hi_entry,
			sizeof(hi_entry), &nh_ruri, NULL, NULL,
			&nh_index, &tmp_len);

	if (ret_val == IMS_FAIL)
	{
		IMS_DLOG(IMS_IBCF_LOGHIGH, "%s(): IBCFb2b_iw_build_hi_entry() failed.", __func__);
		return IMS_FAIL;
	}

	if (len + tmp_len + 1 >= sizeof(hi_str))
	{
		IMS_DLOG(IMS_IBCF_LOGHIGH, "%s(): No enough mem for History-Entry.", __func__);
		return IMS_FAIL;
	}

	if (len > 0)
	{
		strcat(hi_str, ",");
		len ++;
	}
	strcat(hi_str, hi_entry);

	return IMS_SUCCESS;
} /* End of function IBCFb2b_iw_build_hist_info_for_rq */

/*
 * Function: IBCFb2b_iw_build_hist_info_for_rsp
 *
 * Description: This function will build History-Info header for responses
 * 	mapped from GCC primitives. It is called upon receiving
 * 	 - Connection Status Request (CnStReq)
 * 	 - Conection Response (conrsp)
 * 	 - Release Request (relreq)
 *
 * Output: Build History-Info header in SIP message
 *
 * Return:  IMS_SUCCESS   -- Successfully built History-Info header
 * 	    IMS_FAIL	  -- Failed to build History-Info header
 */
IMS_RETVAL 
IBCFb2b_iw_build_hist_info_for_rsp (B2BUAEvent *event_ptr, SipHistInfo *histInfo_ptr, SipDiversion *div_ptr, short status_code)
{
	SIPia_Msg	*sipia_msg_ptr;
	char		hi_str[SIPA_MAX_HIST_HDR_LEN];
	int		len = 0;
	int		tmp_len = 0;
	int		histInfoFmt = HISTINFOFMT_DEFAULT;
	int		peerHistInfoFmt = HISTINFOFMT_DEFAULT;
	IMS_RETVAL	ret_val;

	/* check input parameters */
	if ((IS_VALID_PTR(histInfo_ptr) == FALSE) || (IS_VALID_PTR(event_ptr) == FALSE) ||
		(IS_VALID_PTR(div_ptr) == FALSE))
	{
		IMS_DLOG(IMS_IBCF_LOGHIGH, "%s(): Invalid input parameters.", __func__);
		return IMS_FAIL;
	}

	/* init the hdr string and length to be built */
	memset(hi_str, 0, sizeof(hi_str));

	/* FID 16661.0 get the histInfoFmt */
	if (status_code == SIPCNSTS_181_FORWDNG) 
	{
		const IBCF_TRUNK_GROUP_DATA *tg_data_ptr = IBCF_get_tg_data_by_sip_msg(event_ptr->m_ptr);

		if (IS_VALID_PTR(tg_data_ptr) == FALSE)
		{
			IMS_DLOG(IMS_IBCF_LOGHIGH, "%s(), Could not find trunk group data for sip_msg_ptr=[%p].",
			__func__, event_ptr->m_ptr);
			return IMS_FAIL;
		}

		histInfoFmt = IMSdc_get_ibcf_sip_sipi_interworking_profile_histinfofmt(tg_data_ptr->sipi_profile_id);  

		if ( (div_ptr) && (div_ptr->eh.pres == PRSNT_NODEF) &&
		(div_ptr->peerHistInfoFmt.pres == PRSNT_NODEF))
		{
			peerHistInfoFmt = div_ptr->peerHistInfoFmt.val;  
		}
	}

	/* FID 16661.0 if the format is different for the 181, build history-info by div */
	if ((IS_VALID_PTR(histInfo_ptr)) && (histInfo_ptr->eh.pres == PRSNT_NODEF) && 
		(histInfo_ptr->otherEntries.pres == PRSNT_NODEF) && (histInfo_ptr->otherEntries.len >0) &&
		((status_code != SIPCNSTS_181_FORWDNG) || (histInfoFmt == peerHistInfoFmt)))
	{
		ret_val = IBCFb2b_iw_build_hi_entries_by_cchistr(hi_str, sizeof(hi_str), histInfo_ptr, &len);

	if (ret_val == IMS_FAIL)
	{
		IMS_DLOG(IMS_IBCF_LOGHIGH, "%s(): IBCFb2b_iw_build_hi_entries_by_cchistr failed.", __func__);
		return IMS_FAIL;
	}
	}
	else
	{
		if (!((div_ptr) && (div_ptr->isFacTo302))) /* FID 16367.0 if not ISDN FACILITY to SIP 302*/
		{
			/* MGC-8 code will build the History-Info header from incoming SIP INVITE message
			 * if History-Info headers are included in the message based on histInfoFlag in
			 * the event structure. But the interworking code (GCC) will only set histInfoFlag
			 * to appropriate value when ccBothSupportCfx() returns true. With ccBothSupportCfx,
			 * setting of "cfIwProc" on both side trunk groups will be checked, and the trunk
			 * protocols need to be ISDN at one side and SIP at the another side. SBC doesn't
			 * support ISDN connection. So, ccBothSupportCfx() should always returns false.
			 * Then assume the histInfoFlag in event structure should never be SIP_HI_ADDPRIVACY
			 * or SIP_HI_RMVPRIVACY. SIPW code could set histInfoFlag in event structure to
			 * SIP_HI_FIRSTENTRY or SIP_HI_SECONDLSTENTRY when translating SipConInd to GCC
			 * format. And GCC will check the flag for History-Info header to Redirecting Number
			 * mapping.
			 * Don't think the MGC-8 code applies here. Leave a place holder for later reference.
			 */
		}

		if ((div_ptr) && (div_ptr->eh.pres == PRSNT_NODEF))
		{
			char hist_str[SIPA_MAX_HIST_HDR_LEN];
			char curr_index[SIPA_MAX_HIST_IDX_LEN];

			/* clear the string to store the results */            
			memset(hist_str, 0, SIPA_MAX_HIST_HDR_LEN);
			memset(curr_index, 0, SIPA_MAX_HIST_IDX_LEN);

			memcpy(curr_index, histInfo_ptr->index.val, SIPA_MAX_HIST_IDX_LEN);

			ret_val = IBCFb2b_iw_build_hi_entries_by_div(hist_str, sizeof(hist_str), 
					div_ptr, curr_index, event_ptr, &tmp_len);
			if (ret_val == IMS_FAIL)
			{
				IMS_DLOG(IMS_IBCF_LOGHIGH, "%s(): IBCFb2b_iw_build_hi_entries_by_div failed.", __func__);
				return IMS_FAIL;
			}

			if (tmp_len > 0)
			{
				if (len + tmp_len + 1 >= sizeof(hi_str))
				{
					IMS_DLOG(IMS_IBCF_LOGHIGH, "%s(): No enough mem for History-Info entry.", __func__);
					return IMS_FAIL;
				}

				if (len > 0)
				{
					strcat(hi_str, ",");
					len ++;
				}
				strcat(hi_str, hist_str);
				len += tmp_len;

				/* fid16206.0, add the new hisotry-info into the 
				 * original INVITE ips, the last offset of history-info  
				 */
				BString *tmp_hi_str = BString_new(hi_str, len, 0);

				/* Add History-Info header to the message */
				(void)SIPia_addHeader_String(event_ptr->m_ptr,
						SIPIA_HISTORY_INFO,
						BString_getStr(tmp_hi_str),
						BString_getLen(tmp_hi_str),
						LIST_END);
			}
		}
	}

	return IMS_SUCCESS;
}/* End of function IBCFb2b_build_hist_info_for_rsp */

/*
 *  Function: IBCFb2b_iw_build_hi_entries_by_div
 *
 *  Description: This function will build History-Info header entry
 *  		from Diversion header.
 *
 *  Output: Build History-Info header in SIP message
 *
 *  Return:	IMS_SUCCESS -- Successfully build History-Info header
 *  		IMS_FAIL    -- Failed to build History-Info header
 */
IMS_RETVAL 
IBCFb2b_iw_build_hi_entries_by_div(char *hist_str, long max_len, SipDiversion *div, char *curr_index, B2BUAEvent *event_ptr, int *len_ptr)
{
	const char 	*fed_ext_name_ptr;
	int		len = 0;
	int		host_len = 0;
	int		uriType;
	int		histInfoFmt;
	int		fed_ext_name_len = 0;
	SIPA_NUM_TYPE	numType = SIPA_NUMTYPE_UKNWN;
	char		i = 0;
	char		count_limit = 0;

	/* check parameters */
	if ((IS_VALID_PTR(curr_index) == FALSE) || (IS_VALID_PTR(hist_str) == FALSE) || 
		(max_len == 0))
	{
		IMS_DLOG(IMS_IBCF_LOGHIGH, "%s(): Invalid input parameters.", __func__);
		return IMS_FAIL;
	}

	const IBCF_TRUNK_GROUP_DATA *tg_data_ptr = IBCF_get_tg_data_by_sip_msg(event_ptr->m_ptr);

	if (IS_VALID_PTR(tg_data_ptr) == FALSE)
	{
		IMS_DLOG(IMS_IBCF_LOGHIGH, "%s(), Could not find trunk group data for sip_msg_ptr=[%p].",
		__func__, event_ptr->m_ptr);

		return IMS_FAIL;
	}

	const IBCF_SIP_PROFILE_DATA *sip_profile_ptr = IBCF_get_sip_profile_data_by_id(tg_data_ptr->sip_profile_id);
	if (IS_VALID_PTR(sip_profile_ptr) == FALSE)
	{
		IMS_DLOG(IMS_IBCF_LOGHIGH,
		"%s(), Could not find trunk group data for sip_msg_ptr=[%p], tg_id=[%u], vn_id=[%u], sip_profile_id=[%u].",
		__func__, event_ptr->m_ptr, tg_data_ptr->tg_id, tg_data_ptr->vn_id, tg_data_ptr->sip_profile_id);

		return IMS_FAIL;
	}

	/* Get SIP stack port index. */
	SIPIA_PORT_IDX sip_port_idx = B2BCutil_get_sipia_port_idx(event_ptr->leg_ptr);

	if (IS_VALID_SIPIA_PORT_IDX(sip_port_idx) == FALSE)
	{
		IMS_DLOG(IMS_IBCF_LOGHIGH, "%s(), invalid sip_port_idx=[%u] from SIP message type (request or response)=[%d].",
		__func__, sip_port_idx, SIPia_getMsgType(event_ptr->m_ptr));

		return IMS_FAIL;
	}

	/* Get provisioned SIPia port index. */
	NGSS_PORT_INDX port_idx = SSgul_get_sipia_port_prov_key(sip_port_idx);
	if (port_idx == NGSS_DEFAULT_INDX)
	{
		IMS_DLOG(IMS_IBCF_LOGHIGH, "%s(), invalid port_idx=[%d].", __func__, port_idx);

		return IMS_FAIL;
	}

	const IBCF_VIRTUAL_NETWORK_DATA *vn_data_ptr = IBCF_get_vn_data_by_sip_msg(event_ptr->m_ptr);
	if (IS_VALID_PTR(vn_data_ptr) == FALSE)
	{
		IMS_DLOG(IMS_IBCF_LOGHIGH, "%s(), Couldn't find VN data through SIP messsage.", __func__);
		return IMS_FAIL;
	}

	if (vn_data_ptr->vn_type == IBCF_VIRTUAL_NETWORK_CORE)
	{
		fed_ext_name_ptr = IMScfed_get_core_fed_port_ext_host_name(port_idx, &fed_ext_name_len, event_ptr->m_ptr);
	}
	else
	{
		fed_ext_name_ptr = IMSpfed_get_peer_fed_port_ext_host_name(port_idx, event_ptr->m_ptr);
	}

	switch (sip_profile_ptr->uri_type)
	{
	case IBCF_SIP_URI:
		uriType = SIP_TOKEN_SIP;
		break;
	case IBCF_SIPS_URI:
		uriType = SIP_TOKEN_SIPS;
		break;
	case IBCF_TEL_URI:
	default:
		IMS_DLOG(IMS_IBCF_LOGHIGH, "%s(), wrong URI Type[%d] in SIP profile[%u].",
		__func__, sip_profile_ptr->uri_type, tg_data_ptr->sip_profile_id);
		return IMS_FAIL;
	}

	host_len = strlen(fed_ext_name_ptr);

	/* make sure buffer is not overflow 
	 * TS29.163 7.4.6 require two entries,
	 * the first redirecting entry shall 
	 * not exceed 128 bytes. 
	 */
	len = ((div->botDivNum.pres == PRSNT_NODEF) ? div->botDivNum.len : 0) + div->topDivNum.len
		+ ((div->botDivNum.pres == PRSNT_NODEF) ? host_len : 0) + host_len
		+ SIPA_MAX_HIST_IDX_LEN + ((histInfoFmt == HISTINFOFMT_29163_746) ? 128 : 0)
		+ 128; /* max length of other characters */

	if (len > max_len)
	{
		IMS_DLOG(IMS_IBCF_LOGHIGH, "%s(): No enough buffer to build the Hist-Info entry.",
			__func__);
		return IMS_FAIL;
	}
  
	histInfoFmt = IMSdc_get_ibcf_sip_sipi_interworking_profile_histinfofmt(tg_data_ptr->sipi_profile_id);

	IMS_DLOG(IMS_IBCF_LOGMED, "%s(): Hist-Info format:[%d]", __func__, histInfoFmt);

	switch (histInfoFmt)
	{
	case HISTINFOFMT_29163_746:

		IMS_DLOG(IMS_IBCF_LOGMED, "%s(): TS29163 section 7.4.6 format.", __func__);

		/* There is no botDivNum for response */
		if ( (div->topPrivacy.val != SIPRPI_PRIVACY_FULL) )
		{
			char temp[8];
			int resp_code ;

			/* reason shall be present */
			if (div->topDivReason.pres != PRSNT_NODEF)
			{
				IMS_DLOG(IMS_IBCF_LOGHIGH, "%s(): Invalid No Reason present.", __func__);
				return IMS_FAIL;
			}
  
			/* 
			 * Format the redirecting entry, 
			 * will be placed with URI: unknown@unknown.invalid 
			 * If the topDivCause is present, use that value.  
			 */
			if ( (div->topDivCause.pres == PRSNT_NODEF) && (div->topDivCause.val >= 300) )
			{
				resp_code = div->topDivCause.val; 
				IMS_DLOG(IMS_IBCF_LOGHIGH, "%s():topDivCause:[%d]", __func__, resp_code);
			}
			else 
			{
				resp_code = sipa_hi_reason2cause(div->topDivReason.val);
			}

			IMS_DLOG(IMS_IBCF_LOGMED, "%s(): cause[%d], reason[%d]",
				__func__, resp_code, div->topDivReason.val);

			/* FID17364.0 SIP over TLS */
			strcat(hist_str, "<");
			if (uriType == SIP_TOKEN_SIPS)
			{
				strcat(hist_str, "sips:");
			}
			else
			{
				strcat(hist_str, "sip:");
			}

			strcat(hist_str, "unknown@unknown.invalid?Reason=SIP%3Bcause%3D" );
			sprintf(temp, "%d", resp_code);
			strcat(hist_str, temp); 

			/* bug78855 make sure count_limit > 0 */
			if(div->count.pres == PRSNT_NODEF && div->count.val > 0)
			{
				count_limit = div->count.val;
			}
			else
			{
				count_limit = 1;
			}
  
			if (strlen(curr_index) == 0)
			{
				strcpy(curr_index, "1");
				count_limit -= 1;
			}
			if (count_limit + strlen(curr_index)/2 > SIPA_MAX_HIST_HDR_CNT)
			{
				count_limit = SIPA_MAX_HIST_HDR_CNT - strlen(curr_index)/2;
			}
			for (i=0; i < count_limit; i++)
			{
				strcat(curr_index, ".1");
			}
  
			strcat(hist_str, ">;index=");
			strcat(hist_str, curr_index);
			IMS_DLOG(IMS_IBCF_LOGMED, "%s():Redirecting entry:%s", __func__, hist_str);

			/* Format the diverted entries */
			strcat(hist_str, ",<");

			if ( (div->topDivNum.pres == PRSNT_NODEF) && (div->topDivNum.len >0))
			{
				/* As placeholder is always sip uri, 
				 * use SIP uri all the way 
				 */
				char rc = 0;
				uriType = SIP_TOKEN_SIP;
				strcat(hist_str, "sip:");
				rc = IBCF_add_tel_num(hist_str, reinterpret_cast<char *>(div->topDivNum.val), div->topDivNum.len, &numType, uriType);
				if (rc == 0)
				{
					/* No number was added due to invalid tel-num */
					strcat(hist_str, "unavailable");
				}

				strcat(hist_str, "@");

				strcat(hist_str, fed_ext_name_ptr);

				/* Add user=phone in history info */
				if ( (div->topDivType != SIPA_USER_PART) && (rc != SIP_E_FAIL) ) 
				{
					strcat(hist_str, ";user=phone");
				}
			}
			else
			{
				IMS_DLOG(IMS_IBCF_LOGMED, "%s(): no redirection number present.", __func__);

				/* FID17364.0 SIP over TLS */
				if (uriType == SIP_TOKEN_SIPS)
				{
					strcat(hist_str, "sips:");
				}
				else
				{
					strcat(hist_str, "sip:");
				}

				strcat(hist_str, "unavailable@");
				/* FID 14341.0 */
				strcat(hist_str, fed_ext_name_ptr);
			}
 
			if ( ((div->notSubOptions.pres == PRSNT_NODEF) &&
			(div->notSubOptions.val == SIPDIV_NOTSUB_PRES_NOTALLOW) || 
			(div->notSubOptions.val == SIPDIV_NOTSUB_PRES_ALLOWWOREDNUM)) ||
			((div->topPrivacy.pres == PRSNT_NODEF) && 
			(div->topPrivacy.val != SIPDIV_PRIVACY_OFF)) )
			{
				strcat(hist_str, "?Privacy=history");
			}
  
			strcat(hist_str, ">;index=");
			strcat(curr_index, ".1");
			strcat(hist_str, curr_index);
		}
		break;

	case HISTINFOFMT_DEFAULT:
		/* This must be building Invite */
		/* BUG: 90227 */
		if ( (uriType == SIP_TOKEN_TEL) && 
			((div->topDivType == SIPA_USER_PART) || 
			(div->botDivType == SIPA_USER_PART)) ) 
		{
			IMS_DLOG(IMS_IBCF_LOGHIGH, "%s(): Ignore to build History-Info for invalid div Type.", __func__);
			return IMS_FAIL;
		}

		if ((div->botDivNum.pres == PRSNT_NODEF)
			&& (div->botPrivacy.val != SIPRPI_PRIVACY_FULL))
		{
			bool ruri_hdr_pres = FALSE;
			char rc;
  
			if (strlen(hist_str) > 0)
			{
				strcat(hist_str, ","); 
			}

			strcat(hist_str, "<");

			if (uriType == SIP_TOKEN_SIPS)
			{
				strcat(hist_str, "sips:");
			}
			else
			{
				strcat(hist_str, "sip:");
			}

			rc = IBCF_add_tel_num(hist_str, 
				reinterpret_cast<char *>(div->botDivNum.val), 
				div->botDivNum.len, &numType, uriType);

			if (rc == 0)
			{
				IMS_DLOG(IMS_IBCF_LOGHIGH, "%s(): No tel number added as user part",
					__func__);
			}

			/* FID17364.0 SIP over TLS */
			if ((uriType == SIP_TOKEN_SIP) || (uriType == SIP_TOKEN_SIPS))
			{
				strcat(hist_str, "@");
				/* FID 14341.0 */
				strcat(hist_str, fed_ext_name_ptr);
			}
			else if (uriType == SIP_TOKEN_TEL)
			{
				/* "phone-context" is not supported for TEL URI for now */
			}

			if (div->botDivReason.pres == PRSNT_NODEF)
			{
				char temp[8];
				int resp_code = sipa_get_hist_info_cause(div->botDivReason.val);
				if ((resp_code != SIP_E_FAIL) && (resp_code >= SIPCAUSE_301_MOVEDP))
				{
					strcat(hist_str, "?Reason=SIP%3Bcause%3D");
					sprintf(temp, "%d", resp_code);
					strcat(hist_str, temp);
					ruri_hdr_pres = TRUE;
				}
			}

			if ((div->botPrivacy.pres == PRSNT_NODEF)
				&& (div->botPrivacy.val != SIPDIV_PRIVACY_OFF))
			{
				if (ruri_hdr_pres)
				{
					strcat(hist_str, "&Privacy=history");
				}
				else
				{
					strcat(hist_str, "?Privacy=history");
				}
			}

			strcat(hist_str, ">;index=");

			/* FID 16367.0 edit index according to count.val to generate index for 2nd latest entry*/
			if((div->isFacTo302) &&
				(div->count.pres == PRSNT_NODEF) &&
				(div->count.val > SIPA_MAX_HIST_HDR_CNT))  
			{
				strcat(curr_index, "1");
				for (i=1; i < SIPA_MAX_HIST_HDR_CNT; i++)
				{    
					strcat(curr_index, ".1");
				} 
			}
			else
			{
				if(strlen(curr_index) == 0)
				{
					strcpy(curr_index, "1");
				}
				else
				{
					strcat(curr_index, ".1");
				}
			}

			len = len + strlen(curr_index);

			if (len > max_len)
			{
				IMS_DLOG(IMS_IBCF_LOGHIGH, "%s(): No enough mem to build the History-Info entry",
					__func__);
				return IMS_FAIL;
			}                                                                                       
			strcat(hist_str, curr_index);
		}

		if ((div->topDivNum.pres == PRSNT_NODEF)
			&& (div->topPrivacy.val != SIPRPI_PRIVACY_FULL))
		{
			bool ruri_hdr_pres = FALSE;
			char temp[8];
			char rc;

			if (strlen(hist_str) > 0)
			{
				strcat(hist_str, ",");
			}

			/* bug78855 make sure count_limit > 0 */
			if(div->count.pres == PRSNT_NODEF && div->count.val > 0)
			{
				count_limit = div->count.val;
			}
			else
			{
				count_limit = 1;
			}

			if (strlen(curr_index) == 0)
			{
				strcpy(curr_index, "1");
				count_limit -= 1;
			}

			if (count_limit + strlen(curr_index)/2 > SIPA_MAX_HIST_HDR_CNT)
			{
				count_limit = SIPA_MAX_HIST_HDR_CNT - strlen(curr_index)/2;
			}
			for (i=0;i<count_limit;i++)
			{
				strcat(curr_index, ".1");
			}
  
			strcat(hist_str, "<");
			if (uriType == SIP_TOKEN_SIPS)
			{
				strcat(hist_str, "sips:");
			}
			else
			{
				strcat(hist_str, "sip:");
			}

			rc = IBCF_add_tel_num(hist_str, reinterpret_cast<char *>(div->topDivNum.val), div->topDivNum.len, &numType, uriType);

			if (rc == 0)
			{
				IMS_DLOG(IMS_IBCF_LOGHIGH, "%s(): No tel number added as user part.",
					__func__);
			}

			/* FID17364.0 SIP over TLS */
			if ((uriType == SIP_TOKEN_SIP) || (uriType == SIP_TOKEN_SIPS))
			{
				strcat(hist_str, "@");
				/* FID 14341.0 */
				strcat(hist_str, fed_ext_name_ptr);
			}
			else if (uriType == SIP_TOKEN_TEL)
			{
				/* "phone-context" is not supported for now */
			}
  
			if (div->topDivCause.pres == PRSNT_NODEF)
			{
				strcat(hist_str, "?Reason=SIP%3Bcause%3D");
				sprintf(temp, "%d", div->topDivCause.val); 
				strcat(hist_str, temp);
				ruri_hdr_pres = TRUE;
			}
			else if (div->topDivReason.pres == PRSNT_NODEF)
			{
				int resp_code = sipa_get_hist_info_cause(div->topDivReason.val);
				if ((resp_code != SIP_E_FAIL) && (resp_code >= SIPCAUSE_301_MOVEDP))
				{
					strcat(hist_str, "?Reason=SIP%3Bcause%3D");
					sprintf(temp, "%d", resp_code);
					strcat(hist_str, temp);
					ruri_hdr_pres = TRUE;
				}
			}
  
			if ((div->topPrivacy.pres == PRSNT_NODEF)
				&& (div->topPrivacy.val != SIPDIV_PRIVACY_OFF))
			{
				if (ruri_hdr_pres)
				{
					strcat(hist_str, "&Privacy=history");
				}
				else
				{
					strcat(hist_str, "?Privacy=history");
				}
			}
  
			strcat(hist_str, ">;index=");
			strcat(hist_str, curr_index);
		}
		break;

	default:
		IMS_DLOG(IMS_IBCF_LOGHIGH, "%s(): Other format[%d]", __func__, histInfoFmt);
		break;
	} /* End switch(histInfoFmt) */ 

	len = strlen(hist_str);
	IMS_DLOG(IMS_IBCF_LOGMED, "%s(): History-Info entry:[%s]", __func__, hist_str);

	*len_ptr = len;
    
	return IMS_SUCCESS;
} /* End of function IBCFb2b_iw_build_hi_entries_by_div */

/*
 * Function: IBCFb2b_iw_build_hi_entries_by_ips
 *
 * Description: This function will build History-Info header entries from
 * 		incoming SIP request
 *
 * Output:
 *
 * Return:
 */
IMS_RETVAL 
IBCFb2b_iw_build_hi_entries_by_ips (B2BUAEvent *event_ptr, char histInfoFlag)
{
	SIPia_List	*hi_list_ptr;
	SIPia_List_Cursor	*hi_cursor_ptr;
	SIP_Header              *last_hist_ptr = NULL;
	SIP_HistoryInfo         *hist_info_ptr = NULL;

	if (histInfoFlag == SIP_HI_NOHISTHDR)
	{
		IMS_DLOG(IMS_IBCF_LOGHIGH, "%s(): No History-Info header should be built.", __func__);
		return IMS_SUCCESS;
	}

	/* check parameters */
	if (IS_VALID_PTR(event_ptr) == FALSE)
	{
		IMS_DLOG(IMS_IBCF_LOGHIGH, "%s(): Invalid input parameters.", __func__);
		return IMS_FAIL;
	}

	hi_list_ptr = SIPia_getHistoryInfoList(event_ptr->m_ptr);

	if (IS_VALID_PTR(hi_list_ptr) == FALSE)
	{
		IMS_DLOG(IMS_IBCF_LOGHIGH, "%s(): No History-Info header to process.", __func__);
		return IMS_FAIL;
	}

	(void) SIPia_List_cursor_init(hi_list_ptr, &hi_cursor_ptr);

	while (IS_VALID_PTR(hi_cursor_ptr))
	{
		last_hist_ptr = reinterpret_cast<SIP_Header *>(SIPia_List_traverse(&hi_cursor_ptr));

		if (IS_VALID_PTR(last_hist_ptr) == FALSE)
		{
		IMS_DLOG(IMS_IBCF_LOGHIGH, "%s(): No History-Info header to process.", __func__);
		return IMS_FAIL;
		}

		hist_info_ptr = reinterpret_cast<SIP_HistoryInfo *>(SIP_Header_getDecodedObject(event_ptr->m_ptr,
				SIPIA_HISTORY_INFO, last_hist_ptr));

		if (IS_VALID_PTR(hist_info_ptr) == FALSE)
		{
			IMS_DLOG(IMS_IBCF_LOGHIGH, "%s(): No valid History-Info header to process.", __func__);
			return IMS_FAIL;
		}

		if (histInfoFlag == SIP_HI_ADDPRIVACY)
		{
			DString	*d_privacy_ptr = NULL;

			d_privacy_ptr = SIPia_HistoryInfo_getPrivacyValue(event_ptr->m_ptr, hist_info_ptr);
			if (IS_VALID_PTR(d_privacy_ptr))
			{
				if (SIPstrcasecmp(DString_getStr(d_privacy_ptr), "history", 7) == TRUE)
				{
					IMS_DLOG(IMS_IBCF_LOGMED, "%s(): Privacy header already included.", __func__);
					return IMS_SUCCESS;
				}
				else
				{
					if (SIPia_HistoryInfo_setPrivacyValue(event_ptr->m_ptr, hist_info_ptr, HISTORY_STR, HISTORY_LEN) != SIPIA_SUCCESS)
					{
						IMS_DLOG(IMS_IBCF_LOGHIGH, "%s(): Failed to set Privacy.", __func__);
						return IMS_FAIL;
					}
				}
			}
			else
			{
				if (SIPia_HistoryInfo_setPrivacyValue(event_ptr->m_ptr, hist_info_ptr, HISTORY_STR, HISTORY_LEN) != SIPIA_SUCCESS)
				{
					IMS_DLOG(IMS_IBCF_LOGHIGH, "%s(): Failed to set Privacy.", __func__);
					return IMS_FAIL;
				}
			}
		}
		else if (histInfoFlag == SIP_HI_RMVPRIVACY)
		{
			SIPia_HistoryInfo_rmvPrivacyValue(event_ptr->m_ptr, hist_info_ptr);
		}
	}

	return IMS_SUCCESS;
}/* End of function IBCFb2b_iw_build_hi_entries_by_ips */

/*
 * Function: IBCFb2b_iw_build_hi_entries_by_cchistr
 *
 * Description: This function will build History-Info header entry by
 * 		History-Info saved before
 *
 * Output: History-Info header entry
 *
 * Return: Length of the string built
 */
IMS_RETVAL 
IBCFb2b_iw_build_hi_entries_by_cchistr (char *hist_str, long max_len, SipHistInfo *histInfo, int *len_ptr)
{
	char	*idx     = NULL;
	char	temp_str[SIPA_MAX_HI_ENTRY_LEN];
	char	indexStr[SIPA_MAX_HI_ENTRY_LEN];
	int	temp_str_len  = 0;
	int	src_len  = 0;
	int	delimiter_len = sizeof(char);
	int	len      = 0;

	/* check parameters */
	if ((IS_VALID_PTR(hist_str) == FALSE) || (IS_VALID_PTR(histInfo) == FALSE) || 
		(max_len == 0))
	{
		IMS_DLOG(IMS_IBCF_LOGHIGH, "%s(): Invalid input parameters.", __func__);
		return IMS_FAIL;
	}

	/* clear the string to store the results */
	memset(hist_str, 0, max_len);
    
	if ((histInfo->eh.pres == PRSNT_NODEF)
		&& (histInfo->otherEntries.pres == PRSNT_NODEF)
		&& (histInfo->otherEntries.len > 0))
	{
		/* 78704 */
		idx = (char *)histInfo->otherEntries.val;

		while ((src_len < histInfo->otherEntries.len)
			&& ((long)(*idx) > 0))
		{
			/* make sure the dest str is large enough for the entry and comma */
			if ((len + (long)(*idx) + 1) >= max_len)
			{
				IMS_DLOG(IMS_IBCF_LOGHIGH, "%s(): No enough mem for the History-Info Entry: %.*s",
					__func__, *idx, idx+delimiter_len);
				return IMS_FAIL;
			}

			if (strstr(idx+delimiter_len, "Privacy=history") == NULL)
			{
				IMS_DLOG(IMS_IBCF_LOGMED, "%s(): Adding History-Info entry: %.*s",
					__func__, *idx, idx+delimiter_len);

				/* bug78704 for reroute case, ignore all entries whose 
				 * index is greater than histInfo->index. update reason 
				 * for rerouted entry 
				 */
				if (len > 0)
				{
					strcat(hist_str, ",");
					len ++;
				}
				strncat(hist_str, idx + delimiter_len, (long)(*idx));
				len += (long)*idx;
			}
			else
			{
				IMS_DLOG(IMS_IBCF_LOGMED, "%s(): Not add History-Info entry[%.*s] due to privacy policy",
					__func__, *idx, idx+delimiter_len);
			}

			src_len += (long)*idx + delimiter_len;
			idx     =  (char *)histInfo->otherEntries.val + src_len;
		}
	}

	*len_ptr = len;

	return IMS_SUCCESS;
}/* End of function IBCFb2b_iw_build_hi_entries_by_cchistr */

IMS_RETVAL
IBCFb2b_iw_convert_cnStEvent2conEvent(SipwLiConEvnt *conEvent_ptr, SipwLiCnStEvnt *cnStEvent_ptr)
{

	if (IS_VALID_PTR(conEvent_ptr) == FALSE ||
	    IS_VALID_PTR(cnStEvent_ptr) == FALSE)
	{
		IMS_DLOG(IMS_IBCF_LOGHIGH, "%s: invalid input conEvent_ptr(%p) or cnStEvent_ptr(%p).",
			 __FUNCTION__, conEvent_ptr, cnStEvent_ptr);
		return IMS_FAIL;
	}

	memset(conEvent_ptr, 0, sizeof(SipwLiConEvnt));

	conEvent_ptr->div = cnStEvent_ptr->div;
	conEvent_ptr->alertInfo =  cnStEvent_ptr->cwtAlertInfo;
	conEvent_ptr->sipUuiData = cnStEvent_ptr->sipUuiData;
	conEvent_ptr->optBwdCallInd = cnStEvent_ptr->optBwdCallInd;
	conEvent_ptr->bkwdCallInd = cnStEvent_ptr->bkwdCallInd;

	return IMS_SUCCESS;
}

IMS_RETVAL
IBCFb2b_iw_build_18x(B2BUAEvent *event_ptr, SipwLiCnStEvnt *cnStEvent_ptr)
{
	SipwLiConEvnt 	*conEvent_ptr = NULL_PTR;
	SIP_TopLine	*topLine_ptr = NULL_PTR;
	SipwLiConEvnt	conEvent;
	SIPIA_RETURN	ret_val = SIPIA_SUCCESS;
	IMS_RETVAL 	rc = IMS_SUCCESS;
	short		status_code = 0;

	if (IS_VALID_PTR(event_ptr) == FALSE ||
	    IS_VALID_PTR(event_ptr->m_ptr) == FALSE)
	{
		IMS_DLOG(IMS_IBCF_LOGHIGH, "%s: invalid input event_ptr(%p) or msg_ptr.", __FUNCTION__, event_ptr);
		return IMS_FAIL;
	}

	if (IS_VALID_PTR(cnStEvent_ptr) == FALSE)
	{
		IMS_DLOG(IMS_IBCF_LOGHIGH, "%s: invalid input cnStEvent_ptr.", __FUNCTION__);
		return IMS_FAIL;
	}

	/* set status code */
	topLine_ptr = SIPia_getTopLine(event_ptr->m_ptr);
	if (IS_VALID_PTR(topLine_ptr) == FALSE)
	{
		IMS_DLOG(IMS_IBCF_LOGHIGH, "%s: invalid topline found in sip msg.", __FUNCTION__);
		return IMS_FAIL;
	}
        Sip_MsgType_Enum msg_type = SIPia_getMsgType(event_ptr->m_ptr);
        SIP_Method_Enum method    = SIPia_Msg_getMethod(event_ptr->m_ptr);
        status_code               = SIPia_Msg_getStatusCode(event_ptr->m_ptr);
        
        IMS_DLOG(IMS_IBCF_LOGLOWEST, "%s: event_ptr->m_ptr=[%p], msg_type=[%d], method=[%d], status=[%d]",\
            __func__, event_ptr->m_ptr, msg_type, method, status_code);

        if(cnStEvent_ptr->evntType.pres == PRSNT_NODEF)
        {
            if ((msg_type == SIP_RESPONSE) &&
        	(status_code != cnStEvent_ptr->evntType.val) &&
        	((cnStEvent_ptr->evntType.val == CCSIP_ET_180_RINGING) ||
        	 (cnStEvent_ptr->evntType.val == CCSIP_ET_181_FORWDNG) ||
        	 (cnStEvent_ptr->evntType.val == CCSIP_ET_182_QUEUED) ||
        	 (cnStEvent_ptr->evntType.val == CCSIP_ET_183_CALLPRG)))
            {
            	IMS_DLOG(IMS_IBCF_LOGLOW, "%s(): Update Response Code from [%d] to [%d]",
            		__func__, status_code, cnStEvent_ptr->evntType.val);

		SIPia_TopLineResp_setStatusCode(event_ptr->m_ptr, topLine_ptr, cnStEvent_ptr->evntType.val);
		status_code = cnStEvent_ptr->evntType.val;

		/* Set appropriate phrase for 18x response */
		char	*phrase_str;
		char	resbuf[50];
		DMUCHAR	phrase_len = 0;

		phrase_len = SIPia_getReasonPhrase((Sip_Response_Enum)(status_code), resbuf);
		phrase_str = resbuf;

		if (phrase_len > 0)
		{
			resbuf[phrase_len] = '\0';
			ret_val = SIPia_TopLineResp_setReasonPhrase(event_ptr->m_ptr, 
				topLine_ptr, reinterpret_cast<char*>(phrase_str), phrase_len);
			if (ret_val == SIPIA_SUCCESS)
			{
				SIPia_InvalidateRawText(event_ptr->m_ptr, SIPIA_TOP_LINE);
			}
			else
			{
				IMS_DLOG(IMS_IBCF_LOGHIGH, "%s(): Failed set response phrase for [%d] ret_val = %d:\n", 
					__func__, status_code, ret_val);
				/* Let the call contiue */
			}
		}
		/* Didn't get appropriate Reason Phrase, but
		 * let the call continue with the existing
		 * Reason Phrase
		 */
            }
            else if (msg_type == SIP_REQUEST)
            {
                IMS_DLOG(IMS_IBCF_LOGLOWEST, "%s: This is REQUEST", __func__);
            }
        }

	conEvent_ptr = &conEvent;
	rc = IBCFb2b_iw_convert_cnStEvent2conEvent(conEvent_ptr, cnStEvent_ptr);
	if (rc == IMS_FAIL)
	{
		IMS_DLOG(IMS_IBCF_LOGHIGH, "%s: IBCFb2b_iw_convert_cnStEvent2conEvent failed", __FUNCTION__);
		return IMS_FAIL;
	}

	/* build history info header*/
	/* Currently comment the history info logic. Will develope the logic in later release. */
	//IBCFb2b_iw_build_hist_info_for_rsp(event_ptr, &(cnStEvent_ptr->histInfo), &(cnStEvent_ptr->div), status_code);
	
	IBCFb2b_iw_build_diversion(conEvent_ptr,event_ptr, false);

	IBCFb2b_iw_build_reason(event_ptr, &(cnStEvent_ptr->reason));

	IBCFb2b_iw_build_alertinfo(event_ptr, conEvent_ptr);

	IBCFb2b_iw_build_uui(conEvent_ptr, event_ptr);

	IBCFb2b_iw_build_XBCI(conEvent_ptr, event_ptr);

	IBCFb2b_iw_build_XOBCI(event_ptr, conEvent_ptr);

	return IMS_SUCCESS;
}


/* Function:    IBCFb2b_iw_get_sipi_isup_ver
 * Description: Convert the sipi profile configure mimeverion int value to a const char* string
 *
 * Input:       mimeversion
 *
 * Output:
 * 	            failed - ""
 *	            success - version string
 *
 * NOTE:        the function should NEVER return a null pointer
 */
#define MIME_VERSION_MAX 9
const char *siptmimeversion[] = {
    "",          /* 0 */
    "ansi",      /* 1 */
    "ansi00",    /* 2 */
    "itu-t88",   /* 3 */
    "itu-t92+",  /* 4 */
    "CHN",       /* 5 */
    "X-NATISUP", /* 6 */
    "X-UKISUP",  /* 7 */
    ""           /* max */
};

const char *
IBCFb2b_iw_get_sipi_isup_ver(DMUCHAR mimeversion)
{
    if (mimeversion >= MIME_VERSION_MAX)
    {
        IMS_DLOG(IMS_IBCF_LOGHIGH, "%s: mimeversion(%d) must less than %d ", __func__, mimeversion,
                 MIME_VERSION_MAX);
        return ("");
    }

    return (siptmimeversion[mimeversion]);
}

/* Function:    IBCFb2b_iw_encode_ISUP_body
 * Description: add SDP/ISUP body to the sip message
 *
 * Input:       event_ptr
 *
 * Output:
 * 	            failed - ""
 *	            success - version string
 *
 * NOTE:        the function should NEVER return a null pointer
 */
IMS_RETVAL
IBCFb2b_iw_encode_ISUP_body(B2BUAEvent *event_ptr, TknStrVar *encIsupMsg_ptr)
{
    /* check to see if this is request or response */
    SIPia_Msg *             sip_msg_ptr        = NULL_PTR;
    B2BUACallData *         calldata_ptr       = NULL_PTR;
    CcConCb *               con                = NULL_PTR;
    char *                  buf_ptr            = NULL_PTR;
    IBCF_SIPI_PROFILE_DATA *sipi_prof_data_ptr = NULL_PTR;
    SIPDC_IBCF_MIME_INFO    mime_info;
    U8                      evntType = CC_ET_ACM;
    U8                      protType = CC_ET_ACM;
    S16                     ret;
    char                    isup_ver[MAX_ISUP_VER_LEN + 1];
    char                    msg_buffer[256];
    DMUSHORT                profile_id = 0;
    DMUSHORT                variant    = 0;

    if (IS_VALID_PTR(event_ptr) == FALSE)
    {
        IMS_DLOG(IMS_IBCF_LOGHIGH, "%s: invalid input event_ptr", __func__);
        return (IMS_FAIL);
    }

    if (IS_VALID_PTR(event_ptr->calldata_ptr) == FALSE)
    {
        IMS_DLOG(IMS_IBCF_LOGHIGH, "%s, Invalid calldata_ptr.", __func__);
        return (IMS_FAIL);
    }

    if (IS_VALID_PTR(event_ptr->m_ptr) == FALSE)
    {
        IMS_DLOG(IMS_IBCF_LOGHIGH, "%s: invalid m_ptr", __func__);
        return (IMS_FAIL);
    }

	if (IS_VALID_PTR(encIsupMsg_ptr) == FALSE)
	{
		IMS_DLOG(IMS_IBCF_LOGLOW, "%s: no isup body", __func__);
		return (IMS_SUCCESS);
	}
	if (!encIsupMsg_ptr->pres)
	{
		IMS_DLOG(IMS_IBCF_LOGHIGH, "%s, Error,pres%d, size %d",
		                __func__,encIsupMsg_ptr->pres, encIsupMsg_ptr->size);
		return (IMS_SUCCESS);
	}

    IBCFb2b_iw_get_sipt_profile_id(event_ptr, profile_id, variant);
    if (profile_id == 0)
    {
        IMS_DLOG(IMS_IBCF_LOGLOW, "%s, get SIPI profile failed", __func__);
        return (IMS_SUCCESS);
    }
	if (variant == 0)
	{
		IMS_DLOG(IMS_IBCF_LOGLOW, "%s, failed variant 0 ", __func__);
		return (IMS_SUCCESS);
	}

    sipi_prof_data_ptr = IBCF_get_sipi_profile_data_by_id(profile_id);
    if (IS_VALID_PTR(sipi_prof_data_ptr) == FALSE)
    {
        IMS_DLOG(IMS_IBCF_LOGHIGH, "%s, Error: sipi_prof_data_ptr is invalid.", __func__);
        return (IMS_FAIL);
    }

    calldata_ptr = event_ptr->calldata_ptr;
    sip_msg_ptr  = event_ptr->m_ptr;
    con          = IBCFb2b_iw_get_con(event_ptr->calldata_ptr);

    if (IS_VALID_PTR(con) == FALSE)
    {
        IMS_DLOG(IMS_IBCF_LOGHIGH, "%s: could not get con from call data", __func__);
        return (IMS_FAIL);
    }

    IMS_DLOG(IMS_IBCF_LOGLOW, "%s: ISUP body there will add it to outgoing message", __func__);

    /* Create ISUP message body */
    memset(&mime_info, 0, sizeof(SIPDC_IBCF_MIME_INFO));
    memset(&msg_buffer[0], 0, 256);
    mime_info.btype = SIP_ISUP_BODY;
    msg_buffer[0]   = '\0';
    buf_ptr         = &msg_buffer[0];
#if 0
    if ((variant  >= CC_SIPTDDI_BASE) && sipi_prof_data_ptr->db_data.siptmimeversion)
    {
    	IMS_DLOG(IMS_IBCF_LOGLOW, "DDI option %d, use sipi_prof_data_ptr->db_data.siptmimeversion %d",
    		variant, sipi_prof_data_ptr->db_data.siptmimeversion);
        strMcat(&buf_ptr, const_cast<char *>(IBCFb2b_iw_get_sipi_isup_ver(
                          sipi_prof_data_ptr->db_data.siptmimeversion)));
    mime_info.isup_version =
        DString_new(sip_msg_ptr, reinterpret_cast<const char *>(&msg_buffer[0]),
                    static_cast<int>(buf_ptr - &msg_buffer[0]));

    IMS_DLOG(IMS_IBCF_LOGLOW,"configured version %s",
    IBCFb2b_iw_get_sipi_isup_ver(sipi_prof_data_ptr->db_data.siptmimeversion));
    }
    else
#endif
        IBCFb2b_iw_generate_isup_version(&mime_info, sip_msg_ptr, variant);

    mime_info.isup_handling = CONTENT_DISP_HANDLING_REQUIRED;
    mime_info.msgbody_ptr   = reinterpret_cast<char *>(encIsupMsg_ptr->val);
    mime_info.msgbody_len   = (encIsupMsg_ptr->size-1);

    if (IBCFb2b_iw_addSDPorISUP_MsgBody(sip_msg_ptr, &mime_info) != eSTATUS_OK)
    {
        IMS_DLOG(IMS_IBCF_LOGHIGH, "%s:  - cannot add ISUP msg body", __func__);
        return (IMS_FAIL);
    }

    return (IMS_SUCCESS);
}

/* Function:    IBCFb2b_iw_convertSingleToMultipart
 * Description: Convert a SIPia message from a single message body to
 *		        a multipart message body.
 *
 * 		        The following headers are expected in the SIPia message so that
 *		        they can be converted to a multipart message body
 *		        Content-Type
 *		        Content-Length
 *		        Content-Disposition (optional header)
 *		        Content-Encoding (optional header)
 *		        Message-Body
 *
 * Input:
 *	            msg_ptr  - address pointing to SIPia message
 *	            first_mp_part - location containinig address pointing to
 *		        the first multipart
 *
 * Output:
 * 	            failed - IMS_FAIL
 *	            success - IMS_SUCCESS
 */
IMS_RETVAL
IBCFb2b_iw_convertSingleToMultipart(SIPia_Msg *msg_ptr, Multipart **first_mp_ptr)
{
    char *            buf_ptr         = NULL_PTR;
    SIP_ContentType * ct_ptr          = NULL_PTR;
    SIP_MessageBody * mb_ptr          = NULL_PTR;
    SIP_MessageBody * new_mb_ptr      = NULL_PTR;
    SIP_OtherHeader * other_hdr_ptr   = NULL_PTR;
    SIPia_List *      mb_list_ptr     = NULL_PTR;
    DString *         cd_dptr         = NULL_PTR;
    DString *         ce_dptr         = NULL_PTR;
    SIP_ContentLength ct_length       = 0;
    char              msg_buffer[256] = {0}; /*temp buffer to format msg */

    if (IS_VALID_PTR(msg_ptr) == FALSE || IS_VALID_PTR(first_mp_ptr) == FALSE)
    {
        ASRT_RPT(ASRTBADPARAM, 0, "Invalid SIPia msg ptr=%#x, or Multipart **ptr=%#x", msg_ptr,
                 first_mp_ptr);
        return (IMS_FAIL);
    }

    *first_mp_ptr = NULL_PTR;

    /* get content-type decoded object */
    ct_ptr = SIPia_getContentType(msg_ptr);
    if (IS_VALID_PTR(ct_ptr) == FALSE)
    {
        ASRT_RPT(ASUNEXP_RETURN, 0, "Missing content type");
        return (IMS_FAIL);
    }

    if (SIPia_ContentType_getContentType(ct_ptr) == SIP_CT_MULTIPART)
    {
        IMS_DLOG(IMS_IBCF_LOGMED, "%s - already Multipart, nothing to do ", __func__);
        return (IMS_SUCCESS);
    }

    /* get content-disposition raw text */
    cd_dptr = SIPia_Header_getRawText(msg_ptr, CONTENT_DISPOSITION);

    /* get content-encoding raw text */
    ce_dptr = SIPia_Header_getRawText(msg_ptr, CONTENT_ENCODING);

    /* get content-length */
    ct_length = SIPia_getContentLength(msg_ptr);

    /* get message body decoded object */
    mb_ptr = SIPia_getMessageBody(msg_ptr);

    /**************************/
    /* Building Multiplart    */
    /**************************/

    /* create content-type */
    msg_buffer[0] = '\0';
    buf_ptr       = &msg_buffer[0];
    strMcat(&buf_ptr, "multipart/mixed;boundary=\"" SIPDC_LUCENT_BOUNDARY_IBCF "\"",
            sizeof(msg_buffer));

    if (SIPia_addHeader_String(msg_ptr, CONTENT_TYPE, (const char *)&msg_buffer[0],
                               (int)(buf_ptr - &msg_buffer[0])) == FALSE)
    {
        *buf_ptr = '\0';
        ASRT_RPT(ASUNEXP_RETURN, 0, "Cannot add Multipart content-type=[%s]", msg_buffer);
        return (IMS_FAIL);
    }

    /* create message-body */
    new_mb_ptr = SIPia_MessageBody_new(msg_ptr, SIP_MULTIPART_LIST);
    if (new_mb_ptr == NULL)
    {
        IMS_DLOG(IMS_IBCF_LOGLOW, "%s: failed to create new SIP_MessageBody", __func__);
        return (IMS_FAIL);
    }

    if (SIPia_addHeader_Object(msg_ptr, MESSAGE_BODY, (void *)new_mb_ptr) == FALSE)
    {
        ASRT_RPT(ASUNEXP_RETURN, 0, "Cannot add new message body to SIPia message");
        return (IMS_FAIL);
    }

    mb_list_ptr = SIPia_MessageBody_getMultipartList(new_mb_ptr);
    if (IS_VALID_PTR(mb_list_ptr) == FALSE)
    {
        ASRT_RPT(ASUNEXP_RETURN, 0, "Cannot get new multipart message body multipart list ptr");
        return (IMS_FAIL);
    }

    *first_mp_ptr = reinterpret_cast<Multipart *>(SIPia_createObject(msg_ptr, sizeof(Multipart)));
    if (IS_VALID_PTR(*first_mp_ptr) == FALSE)
    {
        IMS_DLOG(IMS_IBCF_LOGHIGH, "%s: failed to create Multipart Object", __func__);
        return (IMS_FAIL);
    }

    if (DString_copy(msg_ptr, Multipart_getBorderBoundary(*first_mp_ptr),
                     (const char *)SIPDC_LUCENT_BOUNDARY_IBCF, SIPDC_LUCENT_BOUNDARY_LEN_IBCF) <= 0)
    {
        ASRT_RPT(ASUNEXP_RETURN, 0, "Cannot create boundary object in the new multipart");
        return (IMS_FAIL);
    }

    /* If content-disposition exists, add it into Multipart */
    if (IS_VALID_PTR(cd_dptr) == TRUE)
    {
        other_hdr_ptr = (SIP_OtherHeader *)SIPia_createObject(msg_ptr, sizeof(SIP_OtherHeader));
        if (IS_VALID_PTR(other_hdr_ptr) == FALSE)
        {
            IMS_DLOG(IMS_IBCF_LOGHIGH, "%s: failed to create SIP_OtherHeader Object", __func__);
            return (IMS_FAIL);
        }

        (void)DString_copy(msg_ptr, &other_hdr_ptr->headerName, "Content-Disposition",
                           sizeof("Content-Disposition") - 1);
        (void)DString_set(&other_hdr_ptr->token, cd_dptr);
        (void)SIPia_List_add(msg_ptr, &((*first_mp_ptr)->otherContentList), (void *)other_hdr_ptr,
                             LIST_END);

        /* This header has been copied to multipart,
         * need to remove it from the top-level in the SIPia message.
         */
        SIPia_DeleteHeaderContainer(msg_ptr, CONTENT_DISPOSITION);
    } /* if content-disposition exists */

    /* If content-encoding exists, add it into Multipart */
    if (IS_VALID_PTR(ce_dptr) == TRUE)
    {
        other_hdr_ptr = (SIP_OtherHeader *)SIPia_createObject(msg_ptr, sizeof(SIP_OtherHeader));
        if (other_hdr_ptr == NULL)
        {
            IMS_DLOG(IMS_IBCF_LOGHIGH, "%s: failed to create SIP_OtherHeader Object", __func__);
            return (IMS_FAIL);
        }

        (void)DString_copy(msg_ptr, &other_hdr_ptr->headerName, "Content-Encoding",
                           sizeof("Content-Encoding") - 1);
        (void)DString_set(&other_hdr_ptr->token, ce_dptr);
        (void)SIPia_List_add(msg_ptr, &((*first_mp_ptr)->otherContentList), (void *)other_hdr_ptr,
                             LIST_END);

        /* This header has been copied to multipart,
         * need to remove it from the top-level in the SIPia message.
         */
        SIPia_DeleteHeaderContainer(msg_ptr, CONTENT_ENCODING);
    } /* if content-encoding exists */

    Multipart_setContentType(*first_mp_ptr, ct_ptr);
    Multipart_setContentLength(*first_mp_ptr, ct_length);
    Multipart_setMultipartBody(*first_mp_ptr, mb_ptr);

    if (SIPia_List_add(msg_ptr, mb_list_ptr, (*first_mp_ptr), LIST_FRONT) != SIPIA_SUCCESS)
    {
        ASRT_RPT(ASUNEXP_RETURN, 0, "Cannot add multipart to the SIPia list;");
        return (IMS_FAIL);
    }

    return (IMS_SUCCESS);
}

/* Function:    IBCFb2b_iw_addSDPorISUP_MsgBody
 *
 * Description:
 *              Add new ISUP or SDP message body into a SIPia message.
 *              1. If SIPia message has single message body, convert the single message
 *                 body into multipart, and then add the new message body into
 *                 multipart.
 *              2. If SIPia message does not have content type object created,
 *                 add the new message body as single message body.
 *              3. If SIPia message has multipart content type, then add new
 *                 message body into multipart.
 * NOTE:
 *             This function currently only supports SDP and ISUP message body
 *             Input:
 *             msg_ptr - address pointing to the SIPia message
 *             mime_info_ptr- address pointing to SDP or ISUP message body information
 * Output:
 *             success    - IMS_SUCCESS
 *             failure    - IMS_FAIL, IMS_FAIL
 */
IMS_RETVAL
IBCFb2b_iw_addSDPorISUP_MsgBody(SIPia_Msg *msg_ptr, SIPDC_IBCF_MIME_INFO *mime_info_ptr)
{
    SIP_ContentType *  ct_ptr             = NULL_PTR;
    SIP_ContentType *  ct_new_ptr         = NULL_PTR;
    SIP_MessageBody *  mb_ptr             = NULL_PTR;
    Multipart *        mp_ptr             = NULL_PTR;
    Multipart *        first_mp_ptr       = NULL_PTR;
    SIPia_List *       mb_list_ptr        = NULL_PTR;
    SIPia_List_Cursor *cursor_p           = NULL_PTR;
    char *             buf_ptr            = NULL_PTR;
    BOOL               single_part_needed = FALSE;
    char               msgbuff[256]       = {0};
    Body_Type_Enum     btype;

    if (IS_VALID_PTR(msg_ptr) == FALSE || IS_VALID_PTR(mime_info_ptr) == FALSE)
    {
        ASRT_RPT(ASRTBADPARAM, 0, "Invalid SIPia msg ptr=%#x or MIME info ptr=%#x", msg_ptr,
                 mime_info_ptr);
        return (IMS_FAIL);
    }

    if (IS_VALID_PTR(mime_info_ptr->msgbody_ptr) == FALSE)
    {
        IMS_DLOG(IMS_IBCF_LOGLOW, "%s: nothing to append to the message", __func__);
        /* nothing to append to the message */
        return (IMS_SUCCESS);
    }

    btype = mime_info_ptr->btype;

    if (btype != SIP_SDP_BODY && btype != SIP_ISUP_BODY)
    {
        IMS_DLOG(IMS_IBCF_LOGHIGH, "%s: body type=%d is not supported", __func__, btype);
        return (IMS_FAIL);
    }

    ct_ptr = SIPia_getContentType(msg_ptr);
    if (IS_VALID_PTR(ct_ptr) == TRUE &&
        SIPia_ContentType_getContentType(ct_ptr) != SIP_CT_MULTIPART)
    {
        IMS_DLOG(IMS_IBCF_LOGMED,
                 "%s - This is not a multipart. Content-Type=%d - convert it to multipart",
                 __func__, SIPia_ContentType_getContentType(ct_ptr));

        if (SIPia_getContentLength(msg_ptr) <= 0)
        {
            IMS_DLOG(IMS_IBCF_LOGMED, "%s - content length is %d", __func__,
                     SIPia_getContentLength(msg_ptr));
            /* We will only one single Content */
            single_part_needed = TRUE;
        }
        else
        {
            /* Convert the current single message body to multipart */
            if (IBCFb2b_iw_convertSingleToMultipart(msg_ptr, &first_mp_ptr) != IMS_SUCCESS)
            {
                IMS_DLOG(IMS_IBCF_LOGHIGH, "%s - conver single message to multipart", __func__,
                         SIPia_getContentLength(msg_ptr));
                return (IMS_FAIL);
            }
        }

        /* get the new multipart content-type location */
        ct_ptr = SIPia_getContentType(msg_ptr);
    }

    /* create a new content-type object */
    ct_new_ptr = (SIP_ContentType *)SIPia_createObject(msg_ptr, sizeof(SIP_ContentType));
    if (IS_VALID_PTR(ct_new_ptr) == FALSE)
    {
        /* Failed in allocating the object */
        IMS_DLOG(IMS_IBCF_LOGLOW, "%s: failed to create Object", __func__);
        return (IMS_FAIL);
    }

    SIPia_ContentType_setContentType(ct_new_ptr, SIP_CT_APPLICATION);

    if (btype == SIP_SDP_BODY)
    {
        (void)DString_copy(msg_ptr, &ct_new_ptr->sub_type, SIPDC_SUBTYPE_SDP_STR_IBCF,
                           SIPDC_SUBTYPE_SDP_LEN_IBCF);
    }
    else if (btype == SIP_ISUP_BODY)
    {
        (void)DString_copy(msg_ptr, SIPia_ContentType_getSubType(ct_new_ptr),
                           SIPDC_SUBTYPE_ISUP_STR_IBCF, SIPDC_SUBTYPE_ISUP_LEN_IBCF);

        /* add isup version */
        if (SIPia_List_add(msg_ptr, SIPia_ContentType_getMParameters(ct_new_ptr),
                           (void *)mime_info_ptr->isup_version, LIST_FRONT) != SIPIA_SUCCESS)
        {
            IMS_DLOG(IMS_IBCF_LOGHIGH, "%s: failed to add ISUP version", __func__);
            return (IMS_FAIL);
        }
    }

    /* Check if the new message body can be added to SIPia message as a single
     * message body or a mulitpart message body.
     */
    if (IS_VALID_PTR(ct_ptr) == FALSE || single_part_needed == TRUE)
    {
        IMS_DLOG(IMS_IBCF_LOGMED, "%s - btype %d be put in single part%d", __func__, btype,
                 single_part_needed);
        /* Single message body */

        if (btype == SIP_SDP_BODY)
        {
            if (SIPia_Msg_setSDP_rawText(msg_ptr, mime_info_ptr->msgbody_ptr,
                                         mime_info_ptr->msgbody_len) != SIPIA_SUCCESS)
            {
                ASRT_RPT(ASUNEXP_RETURN, 0, "Cannot set single SDP message body");
                return (IMS_FAIL);
            }
        }
        else /* SIP_ISUP_BODY */
        {
            if (SIPia_addHeader_String(msg_ptr, MESSAGE_BODY, mime_info_ptr->msgbody_ptr,
                                       mime_info_ptr->msgbody_len) != TRUE)
            {
                ASRT_RPT(ASUNEXP_RETURN, 0, "Cannot add single ISUP message body");
                return (IMS_FAIL);
            }

            if (SIPia_addHeader_Object(msg_ptr, CONTENT_TYPE, (void *)ct_new_ptr) != TRUE)
            {
                ASRT_RPT(ASUNEXP_RETURN, 0, "Cannot add ISUP content-type");
                return (IMS_FAIL);
            }

            SIPia_InvalidateRawText(msg_ptr, CONTENT_TYPE);

            /* content-disposition */
            msgbuff[0] = '\0';
            buf_ptr    = &msgbuff[0];

            strMcat(&buf_ptr, SIPDC_ISUPDISPTYPE_STR_IBCF ";handling=", sizeof(msgbuff));
            strNcat(&buf_ptr,
                    (char *)SIP_ContentDisp_handling_tbl[mime_info_ptr->isup_handling].s_ptr,
                    SIP_ContentDisp_handling_tbl[mime_info_ptr->isup_handling].len);

            if (SIPia_addHeader_String(msg_ptr, CONTENT_DISPOSITION, (const char *)&msgbuff[0],
                                       (int)(buf_ptr - &msgbuff[0])) != TRUE)
            {
                *buf_ptr = '\0';
                IMS_DLOG(IMS_IBCF_LOGHIGH, "%s - cannot add content-disposition=[%s]", __func__,
                         msgbuff);
                return IMS_FAIL;
            }

            (void)SIPia_setContentLength(msg_ptr, mime_info_ptr->msgbody_len);
        }
#if 0
                /* I do not think we should add MIME version for single part */
                if (SIP_Header_setRawText(msg_ptr, MIME_VERSION, SIPDC_MIMEVERSION_STR_IBCF, SIPDC_MIMEVERSION_LEN_IBCF )!= SIPIA_SUCCESS)
                {
                        return IMS_FAIL;
                }

#endif
        return (IMS_SUCCESS);
    }

    /* multipart msg body */
    /* Always add the new message body to the front of the multipart list */
    IMS_DLOG(IMS_IBCF_LOGMED, "%s - start to add multipart msg body", __func__);

    mb_ptr = SIPia_getMessageBody(msg_ptr);
    if (IS_VALID_PTR(mb_ptr) == FALSE)
    {
        ASRT_RPT(ASUNEXP_RETURN, 0, "Error! Multipart message without message body");
        return (IMS_FAIL);
    }

    mb_list_ptr = SIPia_MessageBody_getMultipartList(mb_ptr);
    if (IS_VALID_PTR(mb_list_ptr) == FALSE)
    {
        ASRT_RPT(ASUNEXP_RETURN, 0, "Invalid mutilpart list ptr=%#x", mb_list_ptr);
        return (IMS_FAIL);
    }

    if (IS_VALID_PTR(first_mp_ptr) == FALSE)
    {
        cursor_p = NULL_PTR;
        (void *)SIPia_List_cursor_init(mb_list_ptr, &cursor_p);
        first_mp_ptr = reinterpret_cast<Multipart *>(SIPia_List_traverse(&cursor_p));
    }

    if (IS_VALID_PTR(first_mp_ptr) == FALSE)
    {
        ASRT_RPT(ASBAD_DATA, 0, "Invalid first message body multipart ptr=%#x", first_mp_ptr);
        return (IMS_FAIL);
    }

    /* add message body into multipart */
    mp_ptr = Multipart_new(msg_ptr);
    if (IS_VALID_PTR(mp_ptr) == FALSE)
    {
        IMS_DLOG(IMS_IBCF_LOGHIGH, "%s: Multipart_new fail", __func__);
        return (IMS_FAIL);
    }

    Multipart_setContentType(mp_ptr, ct_new_ptr);
    Multipart_setContentLength(mp_ptr, mime_info_ptr->msgbody_len);

    if (DString_copy(msg_ptr, &mp_ptr->borderBoundary, &first_mp_ptr->borderBoundary) <= 0)
    {
        ASRT_RPT(ASUNEXP_RETURN, 0, "Cannot copy boundary object");
        return (IMS_FAIL);
    }

    mb_ptr = SIPia_MessageBody_new(msg_ptr, mime_info_ptr->btype);
    if (IS_VALID_PTR(mb_ptr) == FALSE)
    {
        IMS_DLOG(IMS_IBCF_LOGHIGH, "%s: SIPia_MessageBody_new fail", __func__);
        return (IMS_FAIL);
    }

    if (DString_copy(msg_ptr, SIPia_MessageBody_getBodyText(mb_ptr),
                     reinterpret_cast<const char *>(mime_info_ptr->msgbody_ptr),
                     mime_info_ptr->msgbody_len) <= 0)
    {
        IMS_DLOG(IMS_IBCF_LOGHIGH, "%s:Cannot copy message body", __func__);
        return (IMS_FAIL);
    }

    if (btype == SIP_ISUP_BODY)
    {
        /* content-disposition */
        SIP_OtherHeader *hdr_ptr =
            (SIP_OtherHeader *)SIPia_createObject(msg_ptr, sizeof(SIP_OtherHeader));
        if (hdr_ptr == NULL)
        {
            return IMS_FAIL;
        }

        msgbuff[0] = '\0';
        buf_ptr    = &msgbuff[0];
        strMcat(&buf_ptr, SIPDC_ISUPDISPTYPE_STR_IBCF ";handling=", sizeof(msgbuff));
        strNcat(&buf_ptr, const_cast<char *>(
                              SIP_ContentDisp_handling_tbl[mime_info_ptr->isup_handling].s_ptr),
                SIP_ContentDisp_handling_tbl[mime_info_ptr->isup_handling].len);
        (void)DString_copy(msg_ptr, &hdr_ptr->headerName, "Content-Disposition",
                           sizeof("Content-Disposition") - 1);

        (void)DString_copy(msg_ptr, &hdr_ptr->token, (const char *)&msgbuff[0],
                           (int)(buf_ptr - &msgbuff[0]));

        (void)SIPia_List_add(msg_ptr, &mp_ptr->otherContentList, (void *)hdr_ptr, LIST_END);

        /* content-encoding */
        hdr_ptr = reinterpret_cast<SIP_OtherHeader *>(
            SIPia_createObject(msg_ptr, sizeof(SIP_OtherHeader)));
        if (IS_VALID_PTR(hdr_ptr) == FALSE)
        {
            IMS_DLOG(IMS_IBCF_LOGHIGH, "%s: SIPia_createObject fail", __func__);
            return (IMS_FAIL);
        }

        (void)DString_copy(msg_ptr, &hdr_ptr->headerName, "Content-Encoding",
                           sizeof("Content-Encoding") - 1);
        (void)DString_copy(msg_ptr, &hdr_ptr->token, "binary", sizeof("binary") - 1);
        (void)SIPia_List_add(msg_ptr, &mp_ptr->otherContentList, (void *)hdr_ptr, LIST_END);
    } /* if SIP_ISUP_BODY */

    Multipart_setMultipartBody(mp_ptr, mb_ptr);
    SIPia_List_add(msg_ptr, mb_list_ptr, mp_ptr, LIST_END);
    SIPia_InvalidateRawText(msg_ptr, MESSAGE_BODY);

    /*
     * Build raw text to update body's overall content-length.
     * SIPia_Header_getRawText() updates overall body
     * Content-Length automatically.
     */
    (void)SIPia_Header_getRawText(msg_ptr, SIPIA_MESSAGE_BODY);

    return (IMS_SUCCESS);
}
/*
 *  Function : IBCFb2b_iw_generate_isup_version
 *
 *  Input: SIPDC_MIME_INFO *mime_info
 *
 *  Output:
 *
 *
 */
IMS_RETVAL IBCFb2b_iw_generate_isup_version
(  
   SIPDC_IBCF_MIME_INFO *mime_info,
   SIPia_Msg *sip_msg_ptr,
   DMUSHORT   variant
)
{
    char isup_ver[256];
    char *buf_ptr = NULL;
    if (mime_info == NULL)
    {
        IMS_DLOG(IMS_IBCF_LOGHIGH, "%s (): mime_info NULL", __func__);
        return (IMS_FAIL);
    }
    if (sip_msg_ptr == NULL)
    {
        IMS_DLOG(IMS_IBCF_LOGHIGH, "%s (): sip_msg_ptr  NULL", __func__);
        return (IMS_FAIL);
    }

    isup_ver[0] = '\0';
    buf_ptr = &isup_ver[0];

    switch(variant)
    {
        case CC_SIPTANS92:
        case CC_SIPTANS95:
        case CC_SIPTATT:
                strMcat(&buf_ptr, " version=ansi00");
                break;

        case CC_SIPT767IT:
        case CC_SIPT76792:
        case CC_SIPTMEXICO:
        case CC_SIPTPOLAND:
        case CC_SIPTSPAIN:
        case CC_SIPTTHAILAND:
        case CC_SIPTITU88:
                strMcat(&buf_ptr, " version=itu-t88");
                break;

        case CC_SIPTITU92:
        case CC_SIPTETSI:
        case CC_SIPTFTZ:
        case CC_SIPTSIGTEL:
        case CC_SIPTITU97:
        case CC_SIPTETSIV3:
        case CC_SIPTTELSTRA:
        case CC_SIPTG500V2:
        case CC_SIPTG500V0:
        case CC_SIPTBRAZIL:
        case CC_SIPTINDIA:
        case CC_SIPTVENEZUELA:
        case CC_SIPTGERMANY:
        case CC_SIPTARGENTINA:
        case CC_SIPTKOREAN:
        case CC_SIPTCOSTARICA:
        case CC_SIPTVEMOVILNET:

        case CC_SIPTKPNNL2:
        case CC_SIPTKPNV2:
        case CC_SIPTRUSSIA:
        case CC_SIPTNICARAGUA:
        case CC_SIPTHK:
        case CC_SIPTPE:
        case CC_SIPTUA:
        case CC_SIPTVN:
        case CC_SIPTITU99:
                strMcat(&buf_ptr, " version=itu-t92+");
                break;
        case CC_SIPTUK:
                strMcat(&buf_ptr, " version=X-UKISUP");
                break;
        case CC_SIPTCHINA:
                strMcat(&buf_ptr, " version=CHN");
                break;
	case CC_SIPTFR:
		strMcat(&buf_ptr, " version=spirou");
		break;
        default:
                IMS_DLOG(IMS_SIPDC_LOGLOW, "%s: Unsupported isup variant value=%d - default to itu-t92+\n",
                        __func__, variant);
                strMcat(&buf_ptr, " version=itu-t92+");
                break;
    }
    mime_info->isup_version = DString_new(sip_msg_ptr,
                                    (const char *) &isup_ver[0],
                                    static_cast<int>(buf_ptr - &isup_ver[0]));

    mime_info->isup_handling = CONTENT_DISP_HANDLING_OPTION;

    IMS_DLOG(IMS_IBCF_LOGLOW, "%s, isupversion %s,isup_handling %d", 
                    __func__, 
                    DString_getStr(mime_info->isup_version),
                    mime_info->isup_handling);
    return ( IMS_SUCCESS );
}
/*
 * Function:    IBCFb2b_iw_add_privacy_value()
 *
 * Description: This function is called add privacy value to Privacy header.
 *      if there is no Privacy header in sip message, now, we need to add a
 *      fresh new Privacy header;
 *      BUT, if there is Privacy header, just append values present in the
 *      variable privacyHeader not present in SIPia_Msg Privacy header.
 *
 * Parameters:  msg_ptr     - Pointer to SIPia_Msg.
 *              privacyHeader   - std::string for Privacy header value.
 *
 * Return:      None
 */
void
IBCFb2b_iw_add_privacy_value(SIPia_Msg *msg_ptr, std::string privacyHeader)
{
    SIP_Privacy *privacy_ptr = reinterpret_cast<SIP_Privacy *>(SIP_Header_getDecodedObject(msg_ptr, SIPIA_PRIVACY));
    if (IS_VALID_PTR(privacy_ptr) == FALSE)
    {
        if (privacyHeader != "")
        {
            SIPia_addHeader_String(msg_ptr, SIPIA_PRIVACY, privacyHeader.c_str());
        }
        return;
    }

    SIPia_List *plist_ptr = SIPia_Privacy_getPrivacyTokenList(privacy_ptr);
    if (IS_VALID_PTR(plist_ptr) == FALSE)
    {
        IMS_DLOG(IMS_IBCF_LOGHIGH, "%s: no privacy token list", __func__);
        return;
    }

    while (!privacyHeader.empty())
    {
        std::string tmp_str = string_pop_with_separator(privacyHeader, ";");
        if (tmp_str.empty())
        {
            continue;
        }
        
        IMS_DLOG(IMS_IBCF_LOGLOW, "%s: pop it: %s", __func__, tmp_str.c_str());

        SIPia_List_Cursor *crsr_p = NULL_PTR;
        SIPia_List_cursor_init(plist_ptr, &crsr_p);

        if (strncmp(tmp_str.c_str(), "header", strlen("header")) == 0)
        {
            SIPia_setPrivacyHeaderPref(privacy_ptr, TRUE);
            continue;
        }
        else if (strncmp(tmp_str.c_str(), "session", strlen("session")) == 0)
        {
            SIPia_Privacy_setPrivacySession(privacy_ptr, TRUE);
            continue;
        }
        else if (strncmp(tmp_str.c_str(), "user", strlen("user")) == 0)
        {
            SIPia_Privacy_setPrivacyUser(privacy_ptr, TRUE);
            continue;
        }
        else if (strncmp(tmp_str.c_str(), "critical", strlen("critical")) == 0)
        {
            SIPia_Privacy_setPrivacyCritical(privacy_ptr, TRUE);
            continue;
        }
        else if (strncmp(tmp_str.c_str(), "none", strlen("none")) == 0)
        {
            SIPia_Privacy_setPrivacyNone(privacy_ptr, TRUE);
            continue;
        }

        /* for token list */
        BOOL findit = FALSE;
        while (IS_VALID_PTR(crsr_p) == TRUE)
        {
            DString *ds_ptr = reinterpret_cast<DString *>(SIPia_List_traverse(&crsr_p));
            IMS_DLOG(IMS_IBCF_LOGLOW, "%s: compare %s with %D", __func__, tmp_str.c_str(), ds_ptr);

            if (DString_getLen(ds_ptr) == tmp_str.length() &&
                strncmp(DString_getStr(ds_ptr), tmp_str.c_str(), tmp_str.length()) == 0)
            {
                findit = TRUE;
                IMS_DLOG(IMS_IBCF_LOGLOW, "%s: find %s, no need add", __func__, tmp_str.c_str());
                break;
            }
        }

        if (findit == FALSE)
        {
            DString *ds_str = DString_new(msg_ptr, tmp_str.c_str(), tmp_str.length(), 0);
            if (IS_VALID_PTR(ds_str) == FALSE)
            {
                IMS_DLOG(IMS_IBCF_LOGHIGH, "%s: allocate new DString fail", __func__);
                break;
            }

            SIPIA_RETURN ret_val = SIPia_List_add(msg_ptr, plist_ptr, ds_str, LIST_END);
            IMS_DLOG(IMS_IBCF_LOGLOW, "%s: SIPia_List_add (%s) return with(%d)", __func__,
                     tmp_str.c_str(), ret_val);
        }
    }
    
    SIPia_InvalidateRawText(msg_ptr, SIPIA_PRIVACY);
    IMS_DLOG(IMS_IBCF_LOGLOW, "%s: success", __func__);
    return;
}

IMS_RETVAL IBCFb2b_iw_build_XOFCI(B2BUAEvent *event_ptr, SipwLiConEvnt *p_SipConEvntPtr, SIPia_Msg *sip_msg_ptr)
{
	DString 			*header_dst = NULL_PTR;
	SIPia_List_Cursor 		*cursor = NULL_PTR;
	CcConCb 			*con = NULL_PTR;
	char				szToken[SIP_XOFCI_MAX_LEN];

	if(IS_VALID_PTR(event_ptr) == FALSE || IS_VALID_PTR(p_SipConEvntPtr) == FALSE || IS_VALID_PTR(sip_msg_ptr) == FALSE)
	{
		ASRT_RPT(ASRTBADPARAM, 0, "event_ptr: %p, p_SipConEvntPtr: %p, sip_msg_ptr: %p",event_ptr, p_SipConEvntPtr, sip_msg_ptr);
		return (IMS_SUCCESS);
	}

	//xofci is only added to initial invite FID16521.1-0130
	if(CPcscf_is_init_invite(sip_msg_ptr) == FALSE)
	{
		IMS_DLOG(IMS_IBCF_LOGLOW, "%s: not a initial invite. no need to build X-Optforward-Call-Indicator.", __func__);
		return (IMS_SUCCESS);
	}

	//check xofci flag
	con = IBCFb2b_iw_get_con(event_ptr->calldata_ptr);
	if((!con) || (!(con->ogIntfcCb)) || (!(con->ogIntfcCb->ccTgCb)) || (!(con->ogIntfcCb->ccTgCb->tgAtt.sipPrflPtr)))
	{
	        IMS_DLOG(IMS_IBCF_LOGLOW, "%s: NULL pointer found. con: %p, ogIntfcCb: %p, ccTgCb: %p, sipPrflPtr: %p", __func__, con, con->ogIntfcCb, con->ogIntfcCb->ccTgCb, con->ogIntfcCb->ccTgCb->tgAtt.sipPrflPtr);
	        return (IMS_SUCCESS);
	}
	
	if (con->ogIntfcCb->ccTgCb->tgAtt.sipPrflPtr->xOfci == 0)
	{
		IMS_DLOG(IMS_IBCF_LOGLOW, "%s: xOfci is not TRUE no need to build X-Optforward-Call-Indicator.", __func__);
		return (IMS_SUCCESS);
	}

	if(p_SipConEvntPtr->optFwdCallInd.eh.pres != PRSNT_NODEF)
	{
		IMS_DLOG(IMS_IBCF_LOGLOW, "%s: optFwdCallInd not present. no need to build X-Optforward-Call-Indicator.", __func__);
		return (IMS_SUCCESS);
	}
	
	/* When X interworking is TRUE, It is a SIPT2SIP call. 
	 * The X header will be replaced by new one for SIPT2SIP. 
	 * Remove the existing header
	 */
	header_dst = SIP_getOtherHeader(sip_msg_ptr, &cursor, X_OFCI_HEADER);
	if (IS_VALID_PTR(header_dst) == TRUE && DString_getLen(header_dst) != 0)
	{	
		SIPia_deleteOtherHeader(event_ptr->m_ptr, X_OFCI_HEADER, X_OFCI_HEADER_LEN);	
	}
	
	if (IBCFb2b_need_build_X_header(event_ptr) == FALSE)
	{
		IMS_DLOG(IMS_IBCF_LOGLOW, "%s: no need to build XOFCI.", __func__);
		return (IMS_SUCCESS);
	}
	
	memset(szToken, 0, SIP_XOFCI_MAX_LEN);
	snprintf(szToken, SIP_XOFCI_MAX_LEN, "cug-ind=%u;ss-ind=%u;clir-ind=%u", p_SipConEvntPtr->optFwdCallInd.clsdUGrpCaInd.val, p_SipConEvntPtr->optFwdCallInd.simpleSegmInd.val, p_SipConEvntPtr->optFwdCallInd.conLineIdReqInd.val);

	if(SIPia_AddOtherHeader(sip_msg_ptr,X_OFCI_HEADER,szToken,X_OFCI_HEADER_LEN,strlen(szToken)) != SIPIA_SUCCESS)
	{
		IMS_DLOG(IMS_IBCF_LOGHIGH, "%s(), failed to add X-Optforward-Call-Indicator.", __func__);
		return (IMS_SUCCESS);
	}

	IMS_DLOG(IMS_IBCF_LOGLOW, "%s(): %s added.", __func__, szToken);

	return (IMS_SUCCESS);
}

IMS_RETVAL IBCFb2b_iw_build_XOBCI(B2BUAEvent *event_ptr, SipwLiConEvnt *p_SipConEvntPtr)
{
	char szToken[128];
	const char *szHeaderName = "X-Optbackward-Call-Indicator";
	SIPia_Msg *sip_msg_ptr = event_ptr->m_ptr;
	
	if(IS_VALID_PTR(event_ptr) == FALSE || IS_VALID_PTR(p_SipConEvntPtr) == FALSE || IS_VALID_PTR(sip_msg_ptr) == FALSE)
	{
		ASRT_RPT(ASRTBADPARAM, 0, "event_ptr: %p, p_SipConEvntPtr: %p, sip_msg_ptr: %p",event_ptr, p_SipConEvntPtr, sip_msg_ptr);
		return IMS_SUCCESS;
	}
	
	SIP_Method_Enum method = SIPia_Msg_getMethod(sip_msg_ptr);
	if(method != SIP_METHOD_INVITE)
	{
		IMS_DLOG(IMS_IBCF_LOGLOWEST, "%s: don't build xobci as the sip method is NOT invite", __func__);
		return IMS_SUCCESS;
	}
	
	int statusCode = SIPia_Msg_getStatusCode(sip_msg_ptr);
	if(statusCode/10 != 18 && statusCode != 200)
	{
		IMS_DLOG(IMS_IBCF_LOGLOWEST, "%s: don't build xobci as the invite response code is %d", __func__, statusCode);
		return IMS_SUCCESS;
	}

	DMUSHORT sipi_profile_id = 0;
	DMUSHORT variant = 0;
	IBCF_SIPI_PROFILE_DATA *sipi_profile_ptr;
	IBCFb2b_iw_get_sipt_profile_id(event_ptr, sipi_profile_id, variant);

	if(variant == TG_SIPI_VARIANT_IT && statusCode == 200)
	{
		IMS_DLOG(IMS_IBCF_LOGLOWEST, "%s: don't build xobci for variant %u, statusCode %d", __func__, variant, statusCode);
		return IMS_SUCCESS;
	}

	sipi_profile_ptr = IBCF_get_sipi_profile_data_by_id(sipi_profile_id);
	if(IS_VALID_PTR(sipi_profile_ptr) == FALSE)
	{
		ASRT_RPT(ASRTBADPARAM, 0, "sipi_profile_ptr: %p", sipi_profile_ptr);
		return IMS_SUCCESS;
	}
	if(sipi_profile_ptr->db_data.xobci == 0)
	{
		IMS_DLOG(IMS_IBCF_LOGLOWEST, "%s: don't build xobci as xobci is not enabled", __func__);
		return IMS_SUCCESS;
	}
	
	if(IBCFb2b_need_build_X_header(event_ptr) == FALSE)
	{
		IMS_DLOG(IMS_IBCF_LOGLOWEST, "%s: no need to build XOBCI.", __func__);
		return IMS_SUCCESS;
	}
	
	SipOptBwdCallInd *pOptBkwdCallInd = &p_SipConEvntPtr->optBwdCallInd;
	if(pOptBkwdCallInd->eh.pres != PRSNT_NODEF)
	{
		IMS_DLOG(IMS_IBCF_LOGLOWEST, "%s: optBwdCallInd not present.", __func__);
		return IMS_SUCCESS;
	}
	

	snprintf(szToken, sizeof(szToken),
                        "%s=%d;%s=%d;%s=%d;%s=%d",
                        "ibi-ind", pOptBkwdCallInd->inbndInfoInd.val,
                        "calldiv-ind", pOptBkwdCallInd->caFwdMayOcc.val,
                        "ssind", pOptBkwdCallInd->simpleSegmInd.val,
                        "mlppuser-ind", pOptBkwdCallInd->mlppUserInd.val);

	SIPia_deleteOtherHeader(sip_msg_ptr, szHeaderName);
	if(SIPia_AddOtherHeader(sip_msg_ptr,szHeaderName,szToken,strlen(szHeaderName),strlen(szToken)) != SIPIA_SUCCESS)
	{
		IMS_DLOG(IMS_IBCF_LOGHIGH, "%s(), failed to add %s.", __func__, szHeaderName);
		return IMS_SUCCESS;
	}

	IMS_DLOG(IMS_IBCF_LOGLOWEST, "%s(): %s added.", __func__, szToken);
	
    return IMS_SUCCESS;
}

IMS_RETVAL IBCFb2b_iw_build_XBCI(SipwLiConEvnt *p_conEvntPtr, B2BUAEvent *event_ptr)
{
	DString 			*header_dst = NULL_PTR;
	SIPia_List_Cursor 		*cursor = NULL_PTR;
	int	count = 0;
	char			x_bci_str[IBCF_XBCI_MAX_LEN+1] = {0};
	DMUSHORT		sipi_profile_id = 0;
	DMUSHORT		variant = TG_SIPI_VARIANT_NONE;
	IBCF_SIPI_PROFILE_DATA *sipi_profile_ptr = NULL_PTR;
	
	if (IS_VALID_PTR(p_conEvntPtr) == FALSE || IS_VALID_PTR(event_ptr) == FALSE)
	{
		IMS_DLOG(IMS_IBCF_LOGLOW, "%s: invalid input parameter", __func__);
		return IMS_FAIL;
	}

	//build X-BCI for a SIP to SIPT call
	IBCF_IW_INFO *iw_info = IBCFb2b_iw_get_iw_info(event_ptr->calldata_ptr);
	if(IS_VALID_PTR(iw_info) == FALSE)
	{
		IMS_DLOG(IMS_IBCF_LOGHIGH, "%s: interwork service data not found. won't build X-Backward-Call-Indicator.", __func__);
		return IMS_FAIL;
	}
	if(iw_info->call_type != CALL_TYPE_SIP2SIPT)
	{
		IMS_DLOG(IMS_IBCF_LOGLOW, "%s: not a sip2sipt call. no need to build X-Backward-Call-Indicator.", __func__);
		return IMS_FAIL;
	}

	IBCFb2b_iw_get_sipt_profile_id(event_ptr, sipi_profile_id, variant);
	sipi_profile_ptr = IBCF_get_sipi_profile_data_by_id(sipi_profile_id);
	if(IS_VALID_PTR(sipi_profile_ptr) == FALSE)
	{
		ASRT_RPT(ASRTBADPARAM, 0, "invalid sipi_profile_ptr: %p", sipi_profile_ptr);
		return IMS_SUCCESS;
	}
	if(sipi_profile_ptr->db_data.xbci == 0)
	{
		IMS_DLOG(IMS_IBCF_LOGLOW, "%s: xbci is No, no need to build X-Backward-Call-Indicator", __func__);
		return IMS_SUCCESS;
	}

	header_dst = SIP_getOtherHeader(event_ptr->m_ptr, &cursor, X_BCI_HEADER);
	if (IS_VALID_PTR(header_dst) == TRUE && DString_getLen(header_dst) != 0)
	{	
		SIPia_deleteOtherHeader(event_ptr->m_ptr, X_BCI_HEADER, X_BCI_HEADER_LEN);	
	}
	
	if (IBCFb2b_need_build_X_header(event_ptr) == FALSE)
	{
		IMS_DLOG(IMS_IBCF_LOGLOW, "%s: no need to build XBCI hdr.", __func__);
		return IMS_FAIL;
	}

	if(p_conEvntPtr->bkwdCallInd.eh.pres != PRSNT_NODEF)
	{
		IMS_DLOG(IMS_IBCF_LOGLOWEST, "%s: bkwdCallInd not present. no need to build X-Backward-Call-Indicator", __func__);
		return IMS_FAIL;
	}

	count = snprintf(x_bci_str, IBCF_XBCI_MAX_LEN,
                        "charge-ind=%d;cdptyst-ind=%d;cdptycat-ind=%d;e2emeth-ind=%d;iw-ind=%d;e2einfo-ind=%d;isdnup-ind=%d;hldg-ind=%d;isdna-ind=%d;echoctrld-ind=%d;sccpmeth-ind=%d",
                         p_conEvntPtr->bkwdCallInd.chrgInd.val,
                         p_conEvntPtr->bkwdCallInd.cadPtyStatInd.val,
                         p_conEvntPtr->bkwdCallInd.cadPtyCatInd.val,

                         p_conEvntPtr->bkwdCallInd.end2EndMethInd.val,
                         p_conEvntPtr->bkwdCallInd.intInd.val,
                         p_conEvntPtr->bkwdCallInd.end2EndInfoInd.val,

                         p_conEvntPtr->bkwdCallInd.isdnUsrPrtInd.val,
                         p_conEvntPtr->bkwdCallInd.holdInd.val,
                         p_conEvntPtr->bkwdCallInd.isdnAccInd.val,

                         p_conEvntPtr->bkwdCallInd.echoCtrlDevInd.val,
                         p_conEvntPtr->bkwdCallInd.sccpMethInd.val);
	
	if (count < 0 || count >= IBCF_XBCI_MAX_LEN + 1)
	{
		IMS_DLOG(IMS_IBCF_LOGLOW, "%s: failed to construct X-Backward-Call-Indicator hdr. count=%d",
			__func__, count);
		return IMS_FAIL;
	}

	if(SIPia_AddOtherHeader(event_ptr->m_ptr ,X_BCI_HEADER, x_bci_str, X_BCI_HEADER_LEN, strlen(x_bci_str)) != SIPIA_SUCCESS)
	{
		IMS_DLOG(IMS_IBCF_LOGHIGH, "%s: failed to add X-Backward-Call-Indicator.", __func__);
		return IMS_FAIL;
	}

	IMS_DLOG(IMS_IBCF_LOGLOW, "%s: X-Backward-Call-Indicator: %s is added.", __func__, x_bci_str);

	return IMS_SUCCESS;
}


/* + FID 16521.0 + */
/**PROC+**********************************************************************/
/* Name:		sipa_build_XFCI 											 */
/*																			 */
/* Purpose: 	Build proprietary X-Forward-Call-Indicator header			 */
/*				KPN Format: 												 */
/*				X-Forward-Call-Indicator: 0 (national)						 */
/*				X-Forward-Call-Indicator: 1 (international) 				 */
/*																			 */
/*				ITALIAN Format: 											 */
/*				X-Forward-Call-Indicator: natintcall-ind=<natintcall-ind>;	 */
/*										  e2emeth-ind=<e2emeth-ind>;		 */
/*										  iw-ind=<iw-ind>;					 */
/*										  e2einfo-ind=<e2einfo-ind>;		 */
/*										  isdnup-ind=<isdnup-ind>;			 */
/*										  isdnupp-ind=<isdnupp-ind>;		 */
/*										  isdna-ind=<isdna-ind>;			 */
/*										  sccpmeth-ind=<sccpmeth-ind>;		 */
/*										  bitL=<bitL>; bitM=<bitM>; 		 */
/*										  bitN=<bitN>; bitO=<bitO>; 		 */
/*										  bitP=<bitP>						 */
/*																			 */
/* Returns: 	void														 */
/*																			 */
/* Params:																	 */
/*																			 */
/* Operation:  Build this header first use KPN format, if failed, use		 */
/*			   Italian format												 */
/*																			 */
/**PROC-**********************************************************************/
IMS_RETVAL IBCFb2b_iw_build_XFCI(B2BUAEvent *event_ptr, SipwLiConEvnt *p_SipConEvntPtr, SIPia_Msg *sip_msg_ptr)
{
	int					count = 0;
	DString 			*header_dst = NULL_PTR;
	SIPia_List_Cursor	*cursor = NULL_PTR;
	CcConCb 			*con = NULL_PTR;
	char				szToken[SIP_XFCI_MAX_LEN];

	if(IS_VALID_PTR(event_ptr) == FALSE || IS_VALID_PTR(p_SipConEvntPtr) == FALSE || IS_VALID_PTR(sip_msg_ptr) == FALSE)
	{
		ASRT_RPT(ASRTBADPARAM, 0, "event_ptr: %p, p_SipConEvntPtr: %p, sip_msg_ptr: %p",event_ptr, p_SipConEvntPtr, sip_msg_ptr);
		return (IMS_SUCCESS);
	}

	//xfci is only added to initial invite FID16521.1-0130
	if(CPcscf_is_init_invite(sip_msg_ptr) == FALSE)
	{
		IMS_DLOG(IMS_IBCF_LOGLOW, "%s: not a initial invite. no need to build X-Optforward-Call-Indicator.", __func__);
		return (IMS_SUCCESS);
	}

	//check xfci flag
	con = IBCFb2b_iw_get_con(event_ptr->calldata_ptr);
	if((!con) || (!(con->ogIntfcCb)) || (!(con->ogIntfcCb->ccTgCb)) || (!(con->ogIntfcCb->ccTgCb->tgAtt.sipPrflPtr)))
	{
			IMS_DLOG(IMS_IBCF_LOGLOW, "%s: NULL pointer found. con: %p, ogIntfcCb: %p, ccTgCb: %p, sipPrflPtr: %p", __func__, con, con->ogIntfcCb, con->ogIntfcCb->ccTgCb, con->ogIntfcCb->ccTgCb->tgAtt.sipPrflPtr);
			return (IMS_SUCCESS);
	}
	
	if (con->ogIntfcCb->ccTgCb->tgAtt.sipPrflPtr->xFwdCallInd == 0)
	{		
		IMS_DLOG(IMS_IBCF_LOGLOW, "%s: xfci is not TRUE no need to build X-Forward-Call-Indicator.", __func__);
		return (IMS_SUCCESS);
	}
	
	/* When X interworking is TRUE, It is a SIPT2SIP call. 
	 * The X header will be replaced by new one for SIPT2SIP. 
	 * Remove the existing header
	 */
	header_dst = SIP_getOtherHeader(sip_msg_ptr, &cursor, X_FCI_HEADER);
	if (IS_VALID_PTR(header_dst) == TRUE && DString_getLen(header_dst) != 0)
	{	
		SIPia_deleteOtherHeader(event_ptr->m_ptr, X_FCI_HEADER, X_FCI_HEADER_LEN);	
	}
		
	if (IBCFb2b_need_build_X_header(event_ptr) == FALSE)
	{
		IMS_DLOG(IMS_IBCF_LOGLOW, "%s: no need to build XFCI.", __func__);
		return (IMS_SUCCESS);
	}
	
	memset(szToken, 0, SIP_XFCI_MAX_LEN);

	if(p_SipConEvntPtr->XFCIValue.pres == PRSNT_NODEF)
	{
		IMS_DLOG(IMS_IBCF_LOGLOW, "%s: Need to add XFCI to outgoing Invite in KPN format(val: %d).",
									 __func__, p_SipConEvntPtr->XFCIValue);
	}
	else if(p_SipConEvntPtr->fwdCallInd.eh.pres == PRSNT_NODEF)
	{
		IMS_DLOG(IMS_IBCF_LOGLOW, "%s: natintcall-ind=%d;e2emeth-ind=%d;iw-ind=%d;e2einfo-ind=%d;isdnup-ind=%d;isdnupp-ind=%d;isdna-ind=%d;sccpmeth-ind=%d;bitM=%d",
									 __func__, 
									 p_SipConEvntPtr->fwdCallInd.natIntCallInd.val,
									 p_SipConEvntPtr->fwdCallInd.end2EndMethInd.val,
									 p_SipConEvntPtr->fwdCallInd.intInd.val,
									 p_SipConEvntPtr->fwdCallInd.end2EndInfoInd.val,
									 p_SipConEvntPtr->fwdCallInd.isdnUsrPrtInd.val,
									 p_SipConEvntPtr->fwdCallInd.isdnUsrPrtPrfInd.val,
									 p_SipConEvntPtr->fwdCallInd.isdnAccInd.val,
									 p_SipConEvntPtr->fwdCallInd.sccpMethInd.val,
									 p_SipConEvntPtr->fwdCallInd.bitM.val);
									 
									 
		count = snprintf(szToken, SIP_XFCI_MAX_LEN,
				 "%s=%d;%s=%d;%s=%d;%s=%d;%s=%d;%s=%d;%s=%d;%s=%d;%s=%d",
						 "natintcall-ind", p_SipConEvntPtr->fwdCallInd.natIntCallInd.val,
						 "e2emeth-ind", p_SipConEvntPtr->fwdCallInd.end2EndMethInd.val,
						 "iw-ind",p_SipConEvntPtr->fwdCallInd.intInd.val,
						 "e2einfo-ind", p_SipConEvntPtr->fwdCallInd.end2EndInfoInd.val,
						 "isdnup-ind", p_SipConEvntPtr->fwdCallInd.isdnUsrPrtInd.val,
						 "isdnupp-ind", p_SipConEvntPtr->fwdCallInd.isdnUsrPrtPrfInd.val,
						 "isdna-ind", p_SipConEvntPtr->fwdCallInd.isdnAccInd.val,
						 "sccpmeth-ind", p_SipConEvntPtr->fwdCallInd.sccpMethInd.val,
						 "bitM", p_SipConEvntPtr->fwdCallInd.bitM.val);
		if(count < 0)
		{
			IMS_DLOG(IMS_IBCF_LOGLOW, "%s: Failed to construct XFCI for Italian format.", __func__);
			return (IMS_FAIL);
		}
		else if(count >= SIP_XFCI_MAX_LEN) /* truncated case */
		{			
			IMS_DLOG(IMS_IBCF_LOGLOW, "%s: XFCI is truncated, need to enlarge buffer for it.", __func__);
			return (IMS_FAIL);
		}
	}
	else
	{
		IMS_DLOG(IMS_IBCF_LOGLOW, "%s: XFCI is not present in either format, ignore it.", __func__);
		return (IMS_FAIL);
	}
	
	if(SIPia_AddOtherHeader(sip_msg_ptr,X_FCI_HEADER,szToken,X_FCI_HEADER_LEN,strlen(szToken)) != SIPIA_SUCCESS)
	{
		IMS_DLOG(IMS_IBCF_LOGHIGH, "%s(), failed to add X-Forward-Call-Indicator.", __func__);
	}

	IMS_DLOG(IMS_IBCF_LOGLOW, "%s(), %s add X-Forward-Call-Indicator.", __func__, szToken);
	
	return (IMS_SUCCESS);
}

/* Function:      IBCFb2b_iw_build_p_asserted_id_for_200ok
 *
 * Descriptions:  The function is used to build P-Asserted-Identity and Privacy Header for 200OK
 *
 * Inputs:        p_SipConEvntPtr  - SipwLiConEvnt
 *                event_ptr        - output sip message
 *
 * Return:        IMS_SUCCESS/IMS_FAIL
 */
IMS_RETVAL
IBCFb2b_iw_build_p_asserted_id_for_200ok(B2BUAEvent *event_ptr, SipwLiConEvnt *p_SipConEvntPtr)
{
    SIPA_NUM_TYPE              numType          = SIPA_NUMTYPE_UKNWN;
    unsigned long              uriType          = 0;
    IBCF_URI_TYPE              uri_type         = IBCF_MAX_URI;
    std::string                p_asserted_id_string;
    std::string                srcAddrStr;
    B2BUALegData *             leg_ptr        = NULL_PTR;
    B2BUALegData *             mate_ptr       = NULL_PTR;
    const char *               host_str       = NULL_PTR;
    int                        host_len       = 0;
    NGSS_PORT_INDX             ngss_port_idx  = 0;
    SIPIA_PORT_IDX             sipia_port_idx = 0;
    IBCF_VN_TG_ID              tg_info;
    IBCF_VIRTUAL_NETWORK_DATA *vn_data_ptr = NULL_PTR;

    if (!IS_VALID_PTR(p_SipConEvntPtr) || !IS_VALID_PTR(event_ptr) ||
        !IS_VALID_PTR(event_ptr->m_ptr))
    {
        IMS_DLOG(IMS_IBCF_LOGHIGH, "%s: Invalid pointer: msg_ptr(%p), event_ptr(%p), p_SipConEvntPtr(%p)", __func__,
                 event_ptr->m_ptr, event_ptr, p_SipConEvntPtr);
        return (IMS_FAIL);
    }

    B2BUAEventToLegs(event_ptr, &leg_ptr, &mate_ptr);
    if (IS_VALID_PTR(leg_ptr) == FALSE || IS_VALID_PTR(mate_ptr) == FALSE)
    {
        IMS_DLOG(IMS_IBCF_LOGHIGH, "%s: Invalid leg_ptr %p or mate_ptr %p", __func__, leg_ptr, mate_ptr);
        return (IMS_FAIL);
    }

    if (IS_VALID_PTR(event_ptr->calldata_ptr) == FALSE)
    {
        IMS_DLOG(IMS_IBCF_LOGHIGH, "%s: invalid calldata in event_ptr", __func__);
        return (IMS_FAIL);
    }

    sipia_port_idx = B2BCutil_get_sipia_port_idx(event_ptr->calldata_ptr);
    if (IS_VALID_SIPIA_PORT_IDX(sipia_port_idx) == FALSE)
    {
        IMS_DLOG(IMS_IBCF_LOGHIGH, "%s: Can't get valid sipia_port_idx:%d", __func__,
                 sipia_port_idx);
        return (IMS_FAIL);
    }

    ngss_port_idx = SSgul_get_sipia_port_prov_key(sipia_port_idx);

    SIPia_Msg *msg_ptr = event_ptr->m_ptr;
    IBCF_SIP_PROFILE_DATA *sip_profile_ptr = IBCF_get_sip_prof_data_by_sipmsg(msg_ptr);
    if (!IS_VALID_PTR(sip_profile_ptr))
    {
        IMS_DLOG(IMS_IBCF_LOGLOW, "%s(), Can't get sip profile ", __func__);
        return (IMS_FAIL);
    }

    DMUSHORT id = 0;
    DMUSHORT variant = 0;
    IBCFb2b_iw_get_sipt_profile_id(event_ptr, id, variant);

    IBCF_SIPI_PROFILE_DATA *sipi_prof_data_ptr = IBCF_get_sipi_profile_data_by_id(id);
    if (!IS_VALID_PTR(sipi_prof_data_ptr))
    {
        IMS_DLOG(IMS_IBCF_LOGHIGH, "%s: could not found  sipi_prof_data_ptr", __func__);
        return (IMS_FAIL);
    }

    tg_info     = IBCF_get_tginfo_from_leg(leg_ptr);
    vn_data_ptr = IBCF_get_vn_data_by_id(tg_info.vn_id);
    if (IS_VALID_PTR(vn_data_ptr) == FALSE)
    {
        IMS_DLOG(IMS_IBCF_LOGHIGH, "%s: invalid vn_data_ptr with vn_id(%d)", __func__,
                 tg_info.vn_id);
        return (IMS_FAIL);
    }

    SIPia_List *list_ptr = reinterpret_cast<SIPia_List *>(SIPia_getHeaderContainer(event_ptr->m_ptr, SIPIA_P_ASSERTED_IDENTITY));
	if((IS_VALID_PTR(list_ptr) == TRUE) && (SIPia_List_num_items(list_ptr) > 0))
    {
        // don't build paid if there is already one in sip message
        IMS_DLOG(IMS_IBCF_LOGLOWEST, "%s(): don't build paid as there is already one", __func__);
        return (IMS_SUCCESS);
    }

	if(p_SipConEvntPtr->rpid.eh.pres == NOTPRSNT)
    {
        /* Do not build a P-Asserted-Identity nor Privacy if no rpid */
        IMS_DLOG(IMS_IBCF_LOGLOWEST, "%s(): No p_asserted-Identity to build\n", __func__);
        return (IMS_SUCCESS);
    }

	if((p_SipConEvntPtr->sipPrivacy.eh.pres == PRSNT_NODEF) &&
        (p_SipConEvntPtr->sipPrivacy.privacy.pres == PRSNT_NODEF) &&
        (p_SipConEvntPtr->sipPrivacy.privacy.len != 0))
    {
		SIPia_DeleteHeaderContainer(msg_ptr, SIPIA_PRIVACY);
		IBCFb2b_iw_add_privacy_value(msg_ptr, string(reinterpret_cast<const char *>(p_SipConEvntPtr->sipPrivacy.privacy.val), p_SipConEvntPtr->sipPrivacy.privacy.len));
    }
    else
    {
        SIP_Privacy *privacy_ptr = reinterpret_cast<SIP_Privacy *>(SIP_Header_getDecodedObject(msg_ptr, SIPIA_PRIVACY));
        SIPia_List *plist_ptr = NULL_PTR; /* list pointer for traverse */
        SIPia_List_Cursor *cursor_ptr = NULL_PTR; /* Cursor for list traverse */
        DString *object_ptr = NULL_PTR; /* pointer to current token */
        bool changed = false;

        /* check the token list to look for token 'id' and remove it*/
        if (privacy_ptr != NULL_PTR)
        {
            plist_ptr = SIPia_Privacy_getPrivacyTokenList(privacy_ptr);
        }
        else
        {
            IMS_DLOG(IMS_IBCF_LOGLOWEST, "%s(): No privacy header found.", __func__); 
        }
        
        if (plist_ptr != NULL_PTR)
        {
            SIPia_List_cursor_init(plist_ptr,&cursor_ptr);
        }
        else
        {
            cursor_ptr = NULL_PTR;
        }    

        while (cursor_ptr != NULL_PTR)
        {
            object_ptr = reinterpret_cast<DString *>(SIPia_List_traverse(&cursor_ptr));
            if (object_ptr == NULL_PTR)
            {
                break;
            }
            
            if (DString_ci_compare(object_ptr, "id", 2) == TRUE)
            {
                /* We found 'id' in the token list. remove it */
                SIPia_List_remove(plist_ptr, (void *) object_ptr);
                changed = true;
            }
        }
        
        // remove 'header'
        if(privacy_ptr != NULL_PTR)
        {
            SIPia_setPrivacyHeaderPref(privacy_ptr, FALSE);
            SIPia_Privacy_setPrivacyUser(privacy_ptr, FALSE);
            changed = true;
        }
        
	if(changed)
	{
            SIPia_InvalidateRawText(msg_ptr, SIPIA_PRIVACY);
            if(SIPia_List_num_items(plist_ptr) == 0 &&
                SIPia_Privacy_isPrivacyHeader(privacy_ptr) == FALSE &&
                SIPia_Privacy_isPrivacySession(privacy_ptr) == FALSE &&
                SIPia_Privacy_isPrivacyUser(privacy_ptr) == FALSE &&
                SIPia_Privacy_isPrivacyCritical(privacy_ptr) == FALSE &&
                SIPia_Privacy_isPrivacyNone(privacy_ptr) == FALSE)
            {
                SIPia_DeleteHeaderContainer(msg_ptr, SIPIA_PRIVACY);
            }
	}
    }

    /* Build pAssertedId and Privacy headers from data in rpid */

    uri_type = sip_profile_ptr->uri_type;

    if(p_SipConEvntPtr->rpid.addrSpec.pres == PRSNT_NODEF)
    {
        /* P-Asserted_identity: name-addr */
        if (p_SipConEvntPtr->rpid.displayName.pres == PRSNT_NODEF)
        {
            /*BUG65259 Chage PAID display name according to privacy */
            /* P-Asserted-Identity should include all info (ie no Anonymous etc)
             */
            /* As with From Hdr set p_asserted name to anonymous (not clear in
             * AT&T
             * NGBE reqs)
             */
            if ((p_SipConEvntPtr->rpid.rpiPrivacy.pres == PRSNT_NODEF) &&
                ((p_SipConEvntPtr->rpid.rpiPrivacy.val == SIPRPI_PRIVACY_FULL) ||
                 (p_SipConEvntPtr->rpid.rpiPrivacy.val == SIPRPI_PRIVACY_NAME)))
            {
                p_asserted_id_string.append("\"Anonymous\"");
            }
            else
            {
                p_asserted_id_string.append("\"");
                p_asserted_id_string.append(
                    reinterpret_cast<char *>(p_SipConEvntPtr->rpid.displayName.val),
                    p_SipConEvntPtr->rpid.displayName.len);
                p_asserted_id_string.append("\"");
            }
        }

        /* P-Asserted-Identity: addr-spec */
        p_asserted_id_string.append("<");

        char uri_type_string[16] = {0};
        IBCF_iw_add_uri_scheme(uri_type_string, convertImsUri2IsupUri(uri_type));
        p_asserted_id_string.append(uri_type_string);

        char num_string[64] = {0};
        IBCF_add_tel_num(num_string, reinterpret_cast<char *>(p_SipConEvntPtr->rpid.addrSpec.val),
                         p_SipConEvntPtr->rpid.addrSpec.len, &numType,
                         convertImsUri2IsupUri(uri_type));
        p_asserted_id_string.append(num_string);

        IMS_DLOG(IMS_IBCF_LOGLOWEST, "%s: Now source address: %s", __func__, p_asserted_id_string.c_str());


        /*
         * there is complex logic to descide how to make source address in original
         * mgc-8, to simple the functionality, we get the original address from sip
         * message(IBCF sip stack.
         */

        switch (vn_data_ptr->vn_type)
        {
        case IBCF_VIRTUAL_NETWORK_PEER:
            host_str = IMSpfed_get_peer_fed_port_ext_host_name(ngss_port_idx, msg_ptr);
            break;
        case IBCF_VIRTUAL_NETWORK_CORE:
            host_str = IMScfed_get_core_fed_port_ext_host_name(ngss_port_idx, &host_len, msg_ptr);
            break;
        default:
            IMS_DLOG(IMS_IBCF_LOGHIGH, "%s: Invalid network type peer/core(%d)", __func__,
                     vn_data_ptr->vn_type);
            return (IMS_FAIL);
        }

        if (IS_VALID_PTR(host_str) == FALSE)
        {
            IMS_DLOG(IMS_IBCF_LOGHIGH, "%s: can not get host string with fed", __func__);
            return (IMS_FAIL);
        }

        srcAddrStr.assign(host_str);


        /* FID17364.0 SIP over TLS */
        if ((convertImsUri2IsupUri(uri_type) == SIP_TOKEN_SIP) ||
            (convertImsUri2IsupUri(uri_type) == SIP_TOKEN_SIPS))
        {
            p_asserted_id_string.append("@");
            p_asserted_id_string.append(srcAddrStr);
        }
        else if (convertImsUri2IsupUri(uri_type) == SIP_TOKEN_TEL)
        {
            char buff[128] = {0};
            IBCF_iw_add_phone_context(buff, const_cast<char *>(srcAddrStr.c_str()), NULL, numType);
            p_asserted_id_string.append(buff);
        }

        if (convertImsUri2IsupUri(uri_type) != SIP_TOKEN_TEL &&
            p_SipConEvntPtr->rpid.type == SIPA_TELE_SUB_PART)
        {
            p_asserted_id_string.append(";user=phone");
        }

        /* FID17364.0 SIP over TLS */
        if (IBCF_get_tg_transport_type(msg_ptr) == IBCF_TRANSPT_TCP)
        {
            p_asserted_id_string.append(";transport=tcp");
        }

        p_asserted_id_string.append(">");

        SIPia_DeleteHeaderContainer(event_ptr->m_ptr, SIPIA_P_ASSERTED_IDENTITY);
        (void)SIPia_addHeader_String(event_ptr->m_ptr, 
                                     SIPIA_P_ASSERTED_IDENTITY,
                                     p_asserted_id_string.c_str(),
                                     p_asserted_id_string.length()); 
    }

    return (IMS_SUCCESS);
}
