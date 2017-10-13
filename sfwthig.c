/**MOD+***********************************************************************/
/* Module:    sfwthig.c                                                       */
/*                                                                           */
/* Purpose:   SIP Firewall thig function.                                     */
/*                                                                           */
/*                                                                           */
/* $Revision: 1.25 $ $Modtime::   Nov 02 2001 09:58:08   $                */
/*                                                                           */
/**MOD-***********************************************************************/

#ifdef SFW
#include <envdep.h>
#include <gen.h>
#include <gen.x>
#include <nbbos.h>
#include <sxaincl.h>
#include <siphif.h>
#include <sippbnch.h>
#include <sippvia.h>
#include <siphue.h>
#include <envdep.h>
#include <sipm.h>
#include <siptincl.h>
#include "sipfdef.h"
#include "sipfunc.h"
#include "sfwstrc.h"
#include "sfwdef.h"
#include "sfwfunc.h"
#include "nbbxproc.h"
#include "ntlinet.h"
#include "sfwthig.h"

#include "sipt_cmds.h"
#include <sip0def.h>
#include <sip0prod.h>
extern NBB_THREAD_GLOBAL_DATA *nbb_thread_global_data; 
#define NBB_CCXT , nbb_thread_global_data

/* -------- Thig definition -------- */
NBB_CHAR sfw_dfa_thig_chars[64] = {
    '4','y','V','r',  'O','k','H','d',
    'Y','u','R','n',  'K','g','D','-',
    'U','q','N','j',  'G','c','.','3',
    'Q','m','J','f',  'C','9','2','x',
    'M','i','F','b',  '8','1','X','t',
    'I','e','B','7',  '0','w','T','p',
    'E','a','6','z',  'W','s','P','l',
    'A','5','Z','v',  'S','o','L','h'
};
NBB_CHAR sfw_dfa_thig_digits[256];
NBB_CHAR* sfw_dfa_thig_markHead = "Kevrineg";
NBB_INT    sfw_dfa_thig_markHeadLen;
NBB_CHAR*  sfw_dfa_thig_markTail = "Kat.ell";
NBB_INT    sfw_dfa_thig_markTailLen;
NBB_CHAR* sfw_dfa_thig_tokenHead = ";tokenized=";
NBB_INT    sfw_dfa_thig_tokenHeadLen;
NBB_CHAR  sfw_dfa_thig_tokenByHead[SFW_THIG_HEAD_LEN];
NBB_INT    sfw_dfa_thig_tokenByHeadLen;

INLINE NBB_USHORT sfw_encipher_via(SIP_IPS **ips NBB_CCXT_T NBB_CXT);
INLINE NBB_USHORT sfw_encipher_record_route(SIP_IPS **ips ,NBB_BOOL request, sfw_poc_cb_t *lpocIp NBB_CCXT_T NBB_CXT);
INLINE NBB_USHORT sfw_encipher_hdr(SIP_IPS **ips,NBB_ULONG method, NBB_ULONG token  NBB_CCXT_T NBB_CXT);
INLINE NBB_USHORT sfw_decipher_header(SIP_IPS **ips,SFW_USER_DATA *userDataPtr, NBB_ULONG token  NBB_CCXT_T NBB_CXT);
INLINE NBB_USHORT sfw_decipher_r_uri(SIP_IPS **ips,SFW_USER_DATA *userDataPtr NBB_CCXT_T NBB_CXT);
INLINE NBB_USHORT sfw_encipher_p_asserted_id(SIP_IPS **ips,NBB_ULONG method NBB_CCXT_T NBB_CXT);
INLINE NBB_USHORT sfw_encipher_remote_party_id(SIP_IPS **ips,NBB_ULONG method NBB_CCXT_T NBB_CXT);
INLINE NBB_USHORT sfw_encipher_from(SIP_IPS **ips,NBB_ULONG method NBB_CCXT_T NBB_CXT);
INLINE NBB_USHORT sfw_encipher_to(SIP_IPS **ips,NBB_ULONG method NBB_CCXT_T NBB_CXT);
INLINE NBB_USHORT sfw_encipher_contact(SIP_IPS **ips,NBB_ULONG method NBB_CCXT_T NBB_CXT);
INLINE NBB_USHORT sfw_encipher_service_route(SIP_IPS **ips , sfw_poc_cb_t *lpocip NBB_CCXT_T NBB_CXT);
INLINE NBB_USHORT sfw_encipher_path(SIP_IPS **ips , sfw_poc_cb_t *lpocip NBB_CCXT_T NBB_CXT);
INLINE NBB_USHORT sfw_encipher_referred_by(SIP_IPS **ips,NBB_ULONG method NBB_CCXT_T NBB_CXT);
INLINE NBB_USHORT sfw_encipher_history_info(SIP_IPS **ips,NBB_ULONG method NBB_CCXT_T NBB_CXT);
INLINE NBB_USHORT sfw_encipher_diversion(SIP_IPS **ips,NBB_ULONG method  NBB_CCXT_T NBB_CXT);
INLINE NBB_USHORT sfw_thig_compare_ip(sfw_poc_cb_t *host1, SIPP_HOST *host2 NBB_CCXT_T NBB_CXT);
INLINE NBB_VOID sfw_dfa_thig_init();
INLINE NBB_INT  sfw_dfa_thig_encipher(NBB_CHAR *inpStr,NBB_INT inpLen,NBB_CHAR* res);
INLINE NBB_INT  sfw_dfa_thig_decipher(NBB_CHAR* inpStr,NBB_CHAR* resStr,NBB_INT* resLen);
NBB_INT sfw_dfa_thig_replace_tokenized(SIP_IPS **ips,SFW_USER_DATA *ud_ptr, NBB_CHAR* uriStr,NBB_INT *uriLen  NBB_CCXT_T NBB_CXT);
INLINE NBB_USHORT get_ipversion(SIP_IPS *ips, NBB_ULONG token,NBB_INT *ipversion NBB_CCXT_T NBB_CXT);
INLINE NBB_INT sfw_dfa_thig_encipherURI(
        NBB_CHAR*      uriStr,
        NBB_INT         uriLen,
        NBB_CHAR*       resStr,
        NBB_INT*         resLen,
        NBB_INT         ifNotTel);

NBB_INT sfw_dfa_thig_decipherURI(
        NBB_CHAR*      uriStr,
        NBB_INT         uriLen,
        NBB_CHAR*     resStr,
        NBB_INT  *     resLen);
NBB_USHORT sfw_thig_add_sfw_address(SIP_IPS **ips,NBB_CHAR    *uriStr,
                               NBB_INT     *uriLen,NBB_CHAR *localAddress,
                               NBB_INT     ifNotTel NBB_CCXT_T NBB_CXT);


INLINE NBB_USHORT get_ipversion(SIP_IPS *ips, NBB_ULONG token,NBB_INT *ipversion NBB_CCXT_T NBB_CXT)
{
    NBB_USHORT rc = SIP_E_OK;
    NBB_BOOL   isSip = FALSE;
    NBB_BUF_SIZE offset =0;

    SIPP_NAME_ADDR nameAddrHdr;
    SIPP_URI uri;
    SIPP_SIP_URI sipUri;

    SFED_TRACE (NBB_DETAIL_TRC, ("%s: Entering \n", __FUNCTION__));
    *ipversion = AF_UNSPEC;
    SIPP_NAME_ADDR_INIT(&nameAddrHdr);
    SIPP_URI_INIT(&uri);
    offset = SIPM_HEADER_FIND(ips, SIPM_HEADERS(ips), token);
    if(offset == 0)
    {
        SFED_TRACE(NBB_DETAIL_TRC, ("%s:There is no token:%lx header to parse\n", __FUNCTION__,token));
        goto EXIT_LABEL;
    }
    if((rc = SIPM_NAME_ADDR_COPY_OUT(&nameAddrHdr, ips, SIPM_HEADER_VALUE(&ips, offset))) != SIP_E_OK)
    {
        SFED_TRACE (NBB_DETAIL_TRC, ("%s: Could not retrieve NAME_ADDR from From header. rc:%d\n", __FUNCTION__, rc));
        goto EXIT_LABEL;
    }
    if((rc = SIPP_URI_PARSE_TEXT(&uri, SIPP_URI_GET_TEXT(&nameAddrHdr.uri),
                    SIPP_URI_GET_LENGTH(&nameAddrHdr.uri))) != SIP_E_OK)
    {
        SIPA_TRACE(NBB_DETAIL_TRC, ("%s: Could not retrieve uri from From header, rc:%d\n", __FUNCTION__, rc));
        goto EXIT_LABEL;
    }
    if((SIPP_PARM_GET_TOKEN(&(uri.scheme)) == SIP_TOKEN_SIP) ||
       (SIPP_PARM_GET_TOKEN(&(uri.scheme)) == SIP_TOKEN_SIPS)) /* FID17364.0 SIP over TLS */
    {
        SIPP_SIP_URI_INIT(&sipUri);
        isSip = TRUE;
        if((rc = SIPP_SIP_URI_PARSE_TEXT(&sipUri, SIPP_URI_GET_TEXT(&uri),
                        SIPP_URI_GET_LENGTH(&uri))) != SIP_E_OK)
        {
            SIPA_TRACE(NBB_ERROR_TRC, ("%s:failed for From: header:%d\n", __FUNCTION__, rc));
            goto EXIT_LABEL;
        }
        if (sipUri.host_port.object.length)
        {
            if ( sipUri.host_port.inet_address.length != 0)
            {

                if ( sipUri.host_port.inet_address.type == AMB_INETWK_ADDR_TYPE_IPV6 )
                {
                    *ipversion = AF_INET6;
                }
                else if ( sipUri.host_port.inet_address.type == AMB_INETWK_ADDR_TYPE_IPV4 )
                {
                    *ipversion = AF_INET;
                }
            }
        }
    }
EXIT_LABEL:
    SIPP_NAME_ADDR_TERM(&nameAddrHdr);
    SIPP_URI_TERM(&uri);
    if(isSip)
        SIPP_SIP_URI_TERM(&sipUri);
    return(rc);
}

INLINE NBB_USHORT sfw_thig_compare_ip(sfw_poc_cb_t *host1, SIPP_HOST *host2 NBB_CCXT_T NBB_CXT)
{
    NBB_USHORT rc = SIP_E_OK;

    SFED_TRACE (NBB_DETAIL_TRC, ("%s: Entering \n", __FUNCTION__));
    if(host1->ip.length !=0)
    {
        rc = host1->ip.length - host2->inet_address.length;
        if(rc ==0)
        {
            rc = host1->ip.type - host2->inet_address.type;

            if(rc ==0)
            {
                rc = NBB_MEMCMP(host1->ip.address,host2->inet_address.address,host1->ip.length);
            }
        }
    }
    if(rc == 0)
    {
        rc = host1->port - host2->port_id; 
    }
    return(rc);
}

NBB_USHORT sfw_thig_encipher_msg(SIP_IPS **ips, NBB_USHORT *rsp_code NBB_CCXT_T NBB_CXT)
{
    NBB_USHORT rc = SIP_E_OK;
    NBB_BOOL request = IS_REQUEST(*ips);
    NBB_BOOL isSrcFqdn = FALSE;
    NBB_ULONG token=SIP_TOKEN_NULL;
    SFW_USER_DATA *ud_ptr = NULL;
    NBB_UINT flags = 0;
    NBB_SHORT status_code = 0;
    /* NBB_CHAR ood_request = FALSE; */
    NBB_CHAR proxy = FALSE;
    NBB_SHORT lpocIdx = 0;
    NBB_BUF_SIZE hdrOffset = 0;

    NBB_TRC_ENTRY(__FUNCTION__);

    SFED_TRACE (NBB_DETAIL_TRC, ("%s: Entering \n", __FUNCTION__));
    if(*ips == NULL)
    {
        SFED_TRACE(NBB_DETAIL_TRC,
                ("%s: wrong parameter ips:0x%p rsp_code:0x%p\n",
                 __FUNCTION__, *ips, rsp_code));
        rc = SIP_E_FAIL;
        goto THIG_ENCIPHER_MSG;
    }

    /* it better to init thig in common place, not for each message.*/
    //sfw_dfa_thig_init();
    status_code = *SIPM_REASON_CODE(*ips);
    ud_ptr = (SFW_USER_DATA*)SIPM_USER_DATA_PTR(*ips);

    if(ud_ptr == NULL )
    {
        SFED_TRACE (NBB_DETAIL_TRC, ("%s: ud_ptr == NULL \n", __FUNCTION__));
        rc = SIP_E_OK;
        goto THIG_ENCIPHER_MSG;
    }

    /*From trust to untrust use lpoc port only*/
    token = SIPM_PARM_GET_TOKEN(*ips, SIPM_METHOD(*ips));
    if (token == 0)
    {
        NBB_USHORT rc;
        SIPP_CSEQ cseq;
        SIPP_CSEQ_INIT(&cseq);
        NBB_BUF_SIZE offset =0;
        offset = SIPM_HEADER_FIND(*ips,
                SIPM_HEADERS(*ips),
                SIP_TOKEN_CSEQ);
        if(offset != 0)
        {
            rc = SIPM_CSEQ_COPY_OUT(&cseq, *ips, SIPM_HEADER_VALUE(*ips, offset));
            if(rc == SIP_E_OK)
            {
                token = cseq.method.token;
            }
            else
            {
                SFED_TRACE(NBB_MAJOR_TRC,
                        ("sipf_filter_set: copy cseq  error exiting ... \n" ));
                rc = SIP_E_OK;
                SIPP_CSEQ_TERM(&cseq);
                goto THIG_ENCIPHER_MSG;
            }
        }
        else
        {
            SFED_TRACE(NBB_MAJOR_TRC,
                    ("sipf_filter_set: no cseq  exiting ... \n" ));
            rc = SIP_E_OK;
            SIPP_CSEQ_TERM(&cseq);
            goto THIG_ENCIPHER_MSG;
        }
        SIPP_CSEQ_TERM(&cseq);
    }

    SFED_TRACE(NBB_DETAIL_TRC, ("%s:msg token:0x%x, request:%d status_code:%d\n",__FUNCTION__,token,request,status_code));

    SFED_TRACE(NBB_DETAIL_TRC, ("%s:ud_ptr->thigFlags:0x%x\n",__FUNCTION__,ud_ptr->thigFlags));


    if (!(ud_ptr->element&SFW_USER_DATA_PEERNET) ||
        !(ud_ptr->element&SFW_USER_DATA_FUNC_FLAG)
        ||(ud_ptr->peernetId == 0))
    {
        rc = SIP_E_OK;
        goto THIG_ENCIPHER_MSG;
    }

    /* From untrust to trust */

    if(ud_ptr->lpocId & SFW_LPOC_UNTRUSTED_MASK)
    {
        rc = SIP_E_OK;
        goto THIG_ENCIPHER_MSG;
    }

    lpocIdx = ud_ptr->lpocId & SFW_LPOC_ID_MASK;
    /* Update lpocIdx when msg comes from trusted side,
     * since ud_ptr->lpocId is unavailable */
    if(!(ud_ptr->lpocId & SFW_LPOC_UNTRUSTED_MASK))
    {
        if(ud_ptr->element & SFW_USER_DATA_PEERNET) 
        {
            if(SFW_CB.peernet[ud_ptr->peernetId - 1] != NULL) 
            {
                lpocIdx = SFW_CB.peernet[ud_ptr->peernetId - 1]->lpoc_id - 1;
            }
        }
    }

    if(!(ud_ptr->funcFlags& SFW_FUNC_THIG))
    {
        rc = SIP_E_OK; 
        goto THIG_ENCIPHER_MSG;
    }

    hdrOffset = SIPM_HEADER_FIND(*ips, SIPM_HEADERS(*ips), SIP_TOKEN_VIA);
    if (hdrOffset)
    {
        SFED_TRACE(NBB_DETAIL_TRC, ("%s: has One Via header, this is proxy, otherwise the VIA has been removed in up level\n",__FUNCTION__));
        proxy = TRUE;
    }
    else
    {
        SFED_TRACE(NBB_DETAIL_TRC, ("%s: non-proxy mode \n",__FUNCTION__));
        proxy = FALSE;
    }
#if 0
    rc = sipa_ccl_ood_requst_flag(&ood_request, *ips NBB_CCXT);
    NBB_ASSERT(rc == SIP_E_OK);
#endif

    if(token == SIP_TOKEN_INVITE)
    {
#if 0
        sfw_encipher_diversion(ips,token NBB_CCXT);
#endif
        sfw_encipher_referred_by(ips,token NBB_CCXT);
        sfw_encipher_history_info(ips,token NBB_CCXT);
    }
    else
    {
        if(proxy)
        {
            sfw_encipher_contact(ips,token NBB_CCXT);
        }
    }

    if(token == SIP_TOKEN_REFER)
    {
        sfw_encipher_referred_by(ips,token NBB_CCXT);
    }

    if(token == SIP_TOKEN_REGISTER)
    {
        sfw_encipher_path(ips,&SFW_CB.lpoc[lpocIdx] NBB_CCXT);
        if(status_code == 200)
        {
            sfw_encipher_service_route(ips,&SFW_CB.lpoc[lpocIdx] NBB_CCXT);
        }
    }


    if(request)
    {
        if (proxy)
        {
            sfw_encipher_via(ips NBB_CCXT);
            sfw_encipher_from(ips, token NBB_CCXT);
            sfw_encipher_remote_party_id(ips, token  NBB_CCXT);
            sfw_encipher_p_asserted_id(ips, token NBB_CCXT);
        }
    }
    else
    {

        if(ud_ptr->thigFlags == SIP_TOKEN_TO)
        {
            sfw_encipher_to(ips,token NBB_CCXT);
        }
    }

    if (sxaGetCcsIdFromContact(*ips NBB_CCXT) == TELICA_INVALID_CCS_ID)
    {
        sfw_encipher_record_route(ips ,request,&SFW_CB.lpoc[lpocIdx] NBB_CCXT);
    }

THIG_ENCIPHER_MSG:
    return rc;
}

INLINE NBB_USHORT sfw_encipher_p_asserted_id(SIP_IPS **ips, NBB_ULONG method NBB_CCXT_T NBB_CXT)
{
    NBB_USHORT rc = SIP_E_OK;
    SFED_TRACE(NBB_DETAIL_TRC, ("%s: enter\n",__FUNCTION__));
    rc = sfw_encipher_hdr(ips,method, SIP_TOKEN_P_ASSERTED_IDENTITY  NBB_CCXT);
    return rc;
}

INLINE NBB_USHORT sfw_encipher_remote_party_id(SIP_IPS **ips, NBB_ULONG method NBB_CCXT_T NBB_CXT)
{
    NBB_USHORT rc = SIP_E_OK;
    SFED_TRACE(NBB_DETAIL_TRC, ("%s: enter\n",__FUNCTION__));
    rc = sfw_encipher_hdr(ips,method,SIP_TOKEN_REMOTE_PARTY_ID NBB_CCXT);
    return rc;
}

INLINE NBB_USHORT sfw_encipher_from(SIP_IPS **ips, NBB_ULONG method  NBB_CCXT_T NBB_CXT)
{
    NBB_USHORT rc = SIP_E_OK;
    SFED_TRACE(NBB_DETAIL_TRC, ("%s: enter\n",__FUNCTION__));
    rc = sfw_encipher_hdr(ips,method,SIP_TOKEN_FROM NBB_CCXT);
    return rc;
}

INLINE NBB_USHORT sfw_encipher_to(SIP_IPS **ips,NBB_ULONG method  NBB_CCXT_T NBB_CXT)
{
    NBB_USHORT rc = SIP_E_OK;
    SFED_TRACE(NBB_DETAIL_TRC, ("%s: enter\n",__FUNCTION__));
    rc = sfw_encipher_hdr(ips,method,SIP_TOKEN_TO NBB_CCXT);
    return rc;
}

INLINE NBB_USHORT sfw_encipher_contact(SIP_IPS **ips,NBB_ULONG method NBB_CCXT_T NBB_CXT)
{
    NBB_USHORT rc = SIP_E_OK;
    SFED_TRACE(NBB_DETAIL_TRC, ("%s: enter\n",__FUNCTION__));
    rc = sfw_encipher_hdr(ips,method,SIP_TOKEN_CONTACT NBB_CCXT);
    return rc;
}

INLINE NBB_USHORT sfw_encipher_referred_by(SIP_IPS **ips,NBB_ULONG method NBB_CCXT_T NBB_CXT)
{
    NBB_USHORT rc = SIP_E_OK;
    SFED_TRACE(NBB_DETAIL_TRC, ("%s: enter\n",__FUNCTION__));
    rc = sfw_encipher_hdr(ips,method,SIP_TOKEN_REFERRED_BY NBB_CCXT);
    return rc;
}

INLINE NBB_USHORT sfw_encipher_history_info(SIP_IPS **ips,NBB_ULONG method NBB_CCXT_T NBB_CXT)
{
    NBB_USHORT rc = SIP_E_OK;
    SFED_TRACE(NBB_DETAIL_TRC, ("%s: enter\n",__FUNCTION__));
    rc = sfw_encipher_hdr(ips,method,SIP_TOKEN_HISTORY_INFO NBB_CCXT);
    return rc;
}

INLINE NBB_USHORT sfw_encipher_diversion(SIP_IPS **ips,NBB_ULONG method NBB_CCXT_T NBB_CXT)
{
    NBB_USHORT rc = SIP_E_OK;
    SFED_TRACE(NBB_DETAIL_TRC, ("%s: enter\n",__FUNCTION__));
    rc = sfw_encipher_hdr(ips,method,SIP_TOKEN_DIVERSION NBB_CCXT);
    return rc;
}

INLINE NBB_USHORT sfw_encipher_via(SIP_IPS **ips NBB_CCXT_T NBB_CXT)
{
    NBB_USHORT rc = SIP_E_OK;
    NBB_BUF_SIZE offset =0;
    NBB_BUF_SIZE length = 0, len = 0,tailLen=0;
    NBB_BYTE text[MAX_THIG_HEADER_LEN],encipher_text[MAX_THIG_HEADER_LEN] ;
    NBB_BYTE *start,*end,*str;

    SFED_TRACE(NBB_DETAIL_TRC, ("%s: enter\n",__FUNCTION__));
    for(offset = SIPM_HEADER_FIND(*ips, SIPM_HEADERS(*ips), SIP_TOKEN_VIA);
        offset != 0;
        offset = SIPM_HEADER_FIND(*ips, offset, SIP_TOKEN_VIA))
    {
        memset(text,0,sizeof(text));
        memset(encipher_text,0,sizeof(encipher_text));
        length = SIPM_PARM_COPY_TEXT_OUT(*ips, SIPM_HEADER_VALUE(*ips, offset),
                text, sizeof(text) - 1);
        SFED_TRACE(NBB_DETAIL_TRC, ("%s: text:%s,len:%d\n",
                    __FUNCTION__,text,length));
        /* Not sure we need to check top_most via here?*/
        len = length;
        start = text;
        str = encipher_text;

        while(len--)
        {
            if(*start == ' '){start++;break;}
            start++;
        }

        if(len<0)
        {
            rc = SIP_E_FAIL; 
            return rc;
        }
        /*copy first part SIP/2.0/UDP*/
        OS_STRNCPY(str,text,start-text);
        str += (start - text);

        end = start;
        while(len--)
        {
            if(*end == ';' || *end==SIP_ASCII_NULL){break;}
            end++;
        }

        if(end<=start)
        {
            rc = SIP_E_FAIL; 
            return rc;
        }

        tailLen = length - (end - text);
        if(len<=0)
        {
            end = &text[length-1];
            tailLen = 0;
        }

        OS_STRNCPY(str,sfw_dfa_thig_markHead,sfw_dfa_thig_markHeadLen);
        str += sfw_dfa_thig_markHeadLen;
        str += sfw_dfa_thig_encipher(start,end-start,str);
        OS_STRNCPY(str,sfw_dfa_thig_markTail,sfw_dfa_thig_markTailLen);
        str += sfw_dfa_thig_markTailLen;
        if(OS_STRSTR(encipher_text,sfw_dfa_thig_tokenByHead)==NULL)
        {
            memcpy(str,sfw_dfa_thig_tokenByHead,sfw_dfa_thig_tokenByHeadLen);
            str += sfw_dfa_thig_tokenByHeadLen;
        }

        if(tailLen>0)
        {
            OS_STRNCPY(str,end,tailLen);
            str += tailLen;
        }
        len = (NBB_INT)(str - encipher_text); 

        len = len > MAX_THIG_HEADER_LEN?MAX_THIG_HEADER_LEN:len;
        SFED_TRACE(NBB_DETAIL_TRC, ("%s: enciphered text:%s, length:%d\n",
                    __FUNCTION__,encipher_text, len,tailLen));

        length = SIPM_PARM_COPY_TEXT_IN (ips,SIPM_HEADER_VALUE(*ips, offset),encipher_text,len);
        if( len != length )
        {
            continue;
        }

    }
    return rc;
}

INLINE NBB_USHORT sfw_encipher_record_route(SIP_IPS **ips ,NBB_BOOL request, sfw_poc_cb_t *lpocip NBB_CCXT_T NBB_CXT)
{
    NBB_USHORT rc = SIP_E_OK;
    NBB_BUF_SIZE offset =0;
    NBB_BUF_SIZE length = 0, len = 0;
    NBB_BOOL top_most = FALSE;
    NBB_BYTE text[MAX_THIG_HEADER_LEN],encipher_text[MAX_THIG_HEADER_LEN];
    NBB_BYTE *start = NULL;
    SIPP_URI uri;
    SIPP_SIP_URI sipUri;
    SIPP_NAME_ADDR nameAddrHdr;


    SFED_TRACE(NBB_DETAIL_TRC, ("%s: enter\n",__FUNCTION__));
    for(offset = SIPM_HEADER_FIND(*ips, SIPM_HEADERS(*ips), SIP_TOKEN_RECORD_ROUTE);
        offset != 0;
        offset = SIPM_HEADER_FIND(*ips, offset, SIP_TOKEN_RECORD_ROUTE))
    {
        len =0;
        SIPP_NAME_ADDR_INIT(&nameAddrHdr);
        memset(text,0,sizeof(text));
        memset(encipher_text,0,sizeof(encipher_text));

        if((rc = SIPM_NAME_ADDR_COPY_OUT(&nameAddrHdr, *ips, SIPM_HEADER_VALUE(ips, offset))) != SIP_E_OK)
        {
            SIPA_TRACE(NBB_DETAIL_TRC, ("%s: Could not retrieve NAME_ADDR from To: header. rc:%d\n",
                        __FUNCTION__, rc));
            SIPP_NAME_ADDR_TERM(&nameAddrHdr);
            goto EXIT_LABEL;
        }
        SIPA_TRACE (NBB_DETAIL_TRC, ("%s: Route:(%ld)\t\t\t%.*s\n",
                    __FUNCTION__,
                    nameAddrHdr.object.length,
                    (NBB_INT)nameAddrHdr.object.length, nameAddrHdr.object.text));

        SIPP_URI_INIT(&uri);
        if((rc = SIPP_URI_PARSE_TEXT(&uri, SIPP_URI_GET_TEXT(&nameAddrHdr.uri),
                  SIPP_URI_GET_LENGTH(&nameAddrHdr.uri))) != SIP_E_OK)
        {
            SIPA_TRACE(NBB_DETAIL_TRC, ("%s: Could not retrieve uri from To: header, rc:%d\n",
                        __FUNCTION__, rc));
            SIPP_URI_TERM(&uri);
            SIPP_NAME_ADDR_TERM(&nameAddrHdr);
            goto EXIT_LABEL;
        }

        SIPP_SIP_URI_INIT(&sipUri);
        if((rc = SIPP_SIP_URI_PARSE_TEXT(&sipUri, SIPP_URI_GET_TEXT(&uri),
                         SIPP_URI_GET_LENGTH(&uri))) != SIP_E_OK)
        {
            SIPA_TRACE(NBB_MAJOR_TRC, ("%s:failed for sip URI:%d\n", __FUNCTION__, rc));

            SIPP_SIP_URI_TERM(&sipUri);
            SIPP_URI_TERM(&uri);
            SIPP_NAME_ADDR_TERM(&nameAddrHdr);
            goto EXIT_LABEL;
        }

        if(request == TRUE)
        {
            /*do we need to check top_most?*/
            if(!sfw_thig_compare_ip(lpocip,&(sipUri.host_port) NBB_CCXT))
            {
                SIPA_TRACE(NBB_DETAIL_TRC, ("%s: get sfw ip\n",__FUNCTION__));
                SIPP_SIP_URI_TERM(&sipUri);
                SIPP_URI_TERM(&uri);
                SIPP_NAME_ADDR_TERM(&nameAddrHdr);
                continue;
            }
        }
        else
        {
            /*for response from top most to the one contiaining sfw lpoc ip*/
            /*the rest should be encrypt*/
            if(!sfw_thig_compare_ip(lpocip,&(sipUri.host_port) NBB_CCXT))
            {
                SIPA_TRACE(NBB_DETAIL_TRC, ("%s: get sfw ip, try next record-route\n",__FUNCTION__));
                SIPP_SIP_URI_TERM(&sipUri);
                SIPP_URI_TERM(&uri);
                SIPP_NAME_ADDR_TERM(&nameAddrHdr);
                rc = SIP_E_OK;
                return rc;
            }

        }
        length = SIPM_PARM_COPY_TEXT_OUT(*ips, SIPM_HEADER_VALUE(*ips, offset),
                text, sizeof(text) - 1);

        SFED_TRACE(NBB_DETAIL_TRC, ("%s: raw_text:%s, length:%d\n",__FUNCTION__,text, length));
        if(sfw_dfa_thig_encipherURI(text,length,encipher_text,&len,0) != SIP_E_OK)
        {
            SIPP_SIP_URI_TERM(&sipUri);
            SIPP_URI_TERM(&uri);
            SIPP_NAME_ADDR_TERM(&nameAddrHdr);
            continue;
        }

        len = len > MAX_THIG_HEADER_LEN?MAX_THIG_HEADER_LEN:len;
        SFED_TRACE(NBB_DETAIL_TRC, ("%s: encipher_text:%s, length:%d\n",__FUNCTION__,encipher_text, len));

        length = SIPM_PARM_COPY_TEXT_IN(ips,SIPM_HEADER_VALUE(*ips, offset),encipher_text,len);
        if( len != length )
        {
            SIPP_SIP_URI_TERM(&sipUri);
            SIPP_URI_TERM(&uri);
            SIPP_NAME_ADDR_TERM(&nameAddrHdr);
            continue;
        }
        SIPP_SIP_URI_TERM(&sipUri);
        SIPP_URI_TERM(&uri);
        SIPP_NAME_ADDR_TERM(&nameAddrHdr);
    }
EXIT_LABEL:
    return rc;
}

INLINE NBB_USHORT sfw_encipher_service_route(SIP_IPS **ips , sfw_poc_cb_t *lpocip NBB_CCXT_T NBB_CXT)
{
    NBB_USHORT rc = SIP_E_OK;
    NBB_BUF_SIZE offset =0;
    NBB_BUF_SIZE length = 0, len = 0;
    NBB_BOOL top_most = FALSE;
    NBB_BYTE text[MAX_THIG_HEADER_LEN],encipher_text[MAX_THIG_HEADER_LEN];
    NBB_BYTE *start = NULL;
    SIPP_URI uri;
    SIPP_SIP_URI sipUri;
    SIPP_NAME_ADDR nameAddrHdr;

    SFED_TRACE (NBB_DETAIL_TRC, ("%s: Entering \n", __FUNCTION__));

    for(offset = SIPM_HEADER_FIND(*ips, SIPM_HEADERS(*ips), SIP_TOKEN_SERVICE_ROUTE);
        offset != 0;
        offset = SIPM_HEADER_FIND(*ips, offset, SIP_TOKEN_SERVICE_ROUTE))
    {
        len =0;
        SIPP_NAME_ADDR_INIT(&nameAddrHdr);
        memset(text,0,sizeof(text));
        memset(encipher_text,0,sizeof(encipher_text));

        if((rc = SIPM_NAME_ADDR_COPY_OUT(&nameAddrHdr, *ips, SIPM_HEADER_VALUE(ips, offset))) != SIP_E_OK)
        {
            SIPA_TRACE(NBB_DETAIL_TRC, ("%s: Could not retrieve NAME_ADDR from To: header. rc:%d\n",
                        __FUNCTION__, rc));
            SIPP_NAME_ADDR_TERM(&nameAddrHdr);
            goto EXIT_LABEL;
        }
        SIPA_TRACE (NBB_DETAIL_TRC, ("%s: Route:(%ld)\t\t\t%.*s\n",
                    __FUNCTION__,
                    nameAddrHdr.object.length,
                    (NBB_INT)nameAddrHdr.object.length, nameAddrHdr.object.text));

        SIPP_URI_INIT(&uri);
        if((rc = SIPP_URI_PARSE_TEXT(&uri, SIPP_URI_GET_TEXT(&nameAddrHdr.uri),
                  SIPP_URI_GET_LENGTH(&nameAddrHdr.uri))) != SIP_E_OK)
        {
            SIPA_TRACE(NBB_DETAIL_TRC, ("%s: Could not retrieve uri from To: header, rc:%d\n",
                        __FUNCTION__, rc));
            SIPP_URI_TERM(&uri);
            SIPP_NAME_ADDR_TERM(&nameAddrHdr);
            goto EXIT_LABEL;
        }

        SIPP_SIP_URI_INIT(&sipUri);
        if((rc = SIPP_SIP_URI_PARSE_TEXT(&sipUri, SIPP_URI_GET_TEXT(&uri),
                         SIPP_URI_GET_LENGTH(&uri))) != SIP_E_OK)
        {
            SIPA_TRACE(NBB_MAJOR_TRC, ("%s:failed for sip URI:%d\n", __FUNCTION__, rc));

            SIPP_SIP_URI_TERM(&sipUri);
            SIPP_URI_TERM(&uri);
            SIPP_NAME_ADDR_TERM(&nameAddrHdr);
            goto EXIT_LABEL;
        }

            /*do we need to check top_most?*/
        if(!sfw_thig_compare_ip(lpocip,&(sipUri.host_port) NBB_CCXT))
        {
            SIPA_TRACE(NBB_DETAIL_TRC, ("%s: get sfw ip try next service route\n",__FUNCTION__));
            SIPP_SIP_URI_TERM(&sipUri);
            SIPP_URI_TERM(&uri);
            SIPP_NAME_ADDR_TERM(&nameAddrHdr);
            continue;
        }
        length = SIPM_PARM_COPY_TEXT_OUT(*ips, SIPM_HEADER_VALUE(*ips, offset),
                text, sizeof(text) - 1);
        SFED_TRACE(NBB_DETAIL_TRC, ("%s: raw_test:%s, length:%d\n", __FUNCTION__,encipher_text, length));
        if(sfw_dfa_thig_encipherURI(text,length,encipher_text,&len,0) != SIP_E_OK)
        {
            SIPP_SIP_URI_TERM(&sipUri);
            SIPP_URI_TERM(&uri);
            SIPP_NAME_ADDR_TERM(&nameAddrHdr);
            continue;
        }

        len = len > MAX_THIG_HEADER_LEN?MAX_THIG_HEADER_LEN:len;
        SFED_TRACE(NBB_DETAIL_TRC, ("%s: encipher_text:%s, length:%d\n", __FUNCTION__,encipher_text, len));

        length = SIPM_PARM_COPY_TEXT_IN(ips,SIPM_HEADER_VALUE(*ips, offset),encipher_text,len);
        if( len != length )
        {
            SIPP_SIP_URI_TERM(&sipUri);
            SIPP_URI_TERM(&uri);
            SIPP_NAME_ADDR_TERM(&nameAddrHdr);
            continue;
        }
        SIPP_SIP_URI_TERM(&sipUri);
        SIPP_URI_TERM(&uri);
        SIPP_NAME_ADDR_TERM(&nameAddrHdr);
    }
EXIT_LABEL:
    return rc;
}

INLINE NBB_USHORT sfw_encipher_path(SIP_IPS **ips , sfw_poc_cb_t *lpocip NBB_CCXT_T NBB_CXT)
{
    NBB_USHORT rc = SIP_E_OK;
    NBB_BUF_SIZE offset =0;
    NBB_BUF_SIZE length = 0, len = 0;
    NBB_BOOL top_most = FALSE;
    NBB_BYTE text[MAX_THIG_HEADER_LEN],encipher_text[MAX_THIG_HEADER_LEN];
    NBB_BYTE *start = NULL;
    SIPP_URI uri;
    SIPP_SIP_URI sipUri;
    SIPP_NAME_ADDR nameAddrHdr;


    SFED_TRACE (NBB_DETAIL_TRC, ("%s: Entering \n", __FUNCTION__));
    for(offset = SIPM_HEADER_FIND(*ips, SIPM_HEADERS(*ips), SIP_TOKEN_PATH);
        offset != 0;
        offset = SIPM_HEADER_FIND(*ips, offset, SIP_TOKEN_PATH))
    {
        len =0;
        SIPP_NAME_ADDR_INIT(&nameAddrHdr);
        memset(text,0,sizeof(text));
        memset(encipher_text,0,sizeof(encipher_text));

        if((rc = SIPM_NAME_ADDR_COPY_OUT(&nameAddrHdr, *ips, SIPM_HEADER_VALUE(ips, offset))) != SIP_E_OK)
        {
            SIPA_TRACE(NBB_DETAIL_TRC, ("%s: Could not retrieve NAME_ADDR from To: header. rc:%d\n",
                        __FUNCTION__, rc));
            SIPP_NAME_ADDR_TERM(&nameAddrHdr);
            goto EXIT_LABEL;
        }
        SIPA_TRACE (NBB_DETAIL_TRC, ("%s: Route:(%ld)\t\t\t%.*s\n",
                    __FUNCTION__,
                    nameAddrHdr.object.length,
                    (NBB_INT)nameAddrHdr.object.length, nameAddrHdr.object.text));

        SIPP_URI_INIT(&uri);
        if((rc = SIPP_URI_PARSE_TEXT(&uri, SIPP_URI_GET_TEXT(&nameAddrHdr.uri),
                  SIPP_URI_GET_LENGTH(&nameAddrHdr.uri))) != SIP_E_OK)
        {
            SIPA_TRACE(NBB_DETAIL_TRC, ("%s: Could not retrieve uri from To: header, rc:%d\n",
                        __FUNCTION__, rc));
            SIPP_URI_TERM(&uri);
            SIPP_NAME_ADDR_TERM(&nameAddrHdr);
            goto EXIT_LABEL;
        }

        SIPP_SIP_URI_INIT(&sipUri);
        if((rc = SIPP_SIP_URI_PARSE_TEXT(&sipUri, SIPP_URI_GET_TEXT(&uri),
                         SIPP_URI_GET_LENGTH(&uri))) != SIP_E_OK)
        {
            SIPA_TRACE(NBB_MAJOR_TRC, ("%s:failed for sip URI:%d\n", __FUNCTION__, rc));

            SIPP_SIP_URI_TERM(&sipUri);
            SIPP_URI_TERM(&uri);
            SIPP_NAME_ADDR_TERM(&nameAddrHdr);
            goto EXIT_LABEL;
        }

            /*do we need to check top_most?*/
        if(!sfw_thig_compare_ip(lpocip,&(sipUri.host_port) NBB_CCXT))
        {
            SIPA_TRACE(NBB_DETAIL_TRC, ("%s: get sfw ip, try next path\n",__FUNCTION__));
            SIPP_SIP_URI_TERM(&sipUri);
            SIPP_URI_TERM(&uri);
            SIPP_NAME_ADDR_TERM(&nameAddrHdr);
            continue;
        }
        length = SIPM_PARM_COPY_TEXT_OUT(*ips, SIPM_HEADER_VALUE(*ips, offset),
                text, sizeof(text) - 1);
        if(sfw_dfa_thig_encipherURI(text,length,encipher_text,&len,0) != SIP_E_OK)
        {
            SIPP_SIP_URI_TERM(&sipUri);
            SIPP_URI_TERM(&uri);
            SIPP_NAME_ADDR_TERM(&nameAddrHdr);
            continue;
        }

        len = len > MAX_THIG_HEADER_LEN?MAX_THIG_HEADER_LEN:len;
        SFED_TRACE(NBB_DETAIL_TRC, ("%s: encipher_text:%s, length:%d\n",__FUNCTION__,encipher_text, len));

        length = SIPM_PARM_COPY_TEXT_IN(ips,SIPM_HEADER_VALUE(*ips, offset),encipher_text,len);
        if( len != length )
        {
            SIPP_SIP_URI_TERM(&sipUri);
            SIPP_URI_TERM(&uri);
            SIPP_NAME_ADDR_TERM(&nameAddrHdr);
            continue;
        }
        SIPP_SIP_URI_TERM(&sipUri);
        SIPP_URI_TERM(&uri);
        SIPP_NAME_ADDR_TERM(&nameAddrHdr);
    }
EXIT_LABEL:
    return rc;
}

INLINE NBB_USHORT sfw_encipher_hdr(SIP_IPS **ips, NBB_ULONG method, NBB_ULONG token NBB_CCXT_T NBB_CXT)
{
    NBB_BYTE *start;
    SFW_USER_DATA *ud_ptr;
    NBB_BUF_SIZE offset =0;
    NBB_USHORT rc = SIP_E_OK;
    NBB_INT ipversion = AF_UNSPEC;
    NBB_BUF_SIZE length = 0, len = 0;
    SIPA_IP2TRNK *ip2TrkPtr = NULL;
    NBB_BYTE text[MAX_THIG_HEADER_LEN],encipher_text[MAX_THIG_HEADER_LEN],tmpaddress[INET6_ADDRSTRLEN];
    NBB_BYTE localAddress[64]={0},paramStr[8]={0};
    NBB_UINT  port = 0;  /* FID17364.0 */

    SFED_TRACE (NBB_DETAIL_TRC, ("%s: Entering \n", __FUNCTION__));

    if(SIPM_USER_DATA_PRESENT(*ips))
    {
        length = 128;
        ud_ptr = (SFW_USER_DATA*)SIPM_USER_DATA_PTR(*ips);
        NBB_BYTE lpocIdx = ud_ptr->lpocId & SFW_LPOC_ID_MASK;

        /* Update lpocIdx when msg comes from trusted side,
         * since ud_ptr->lpocId is unavailable */
        if(!(ud_ptr->lpocId & SFW_LPOC_UNTRUSTED_MASK))
        {
            if(ud_ptr->element & SFW_USER_DATA_PEERNET) 
            {
                if(SFW_CB.peernet[ud_ptr->peernetId - 1] != NULL) 
                {
                    lpocIdx = SFW_CB.peernet[ud_ptr->peernetId - 1]->lpoc_id - 1;
                }
            }
        }
        NTL_INET_ADDR_TO_STRING(&SFW_CB.lpoc[lpocIdx].ip, 0, (NBB_BYTE*)tmpaddress, &length);
        if(ud_ptr->trkGrp != 0)
        {
            ip2TrkPtr = (SIPA_IP2TRNK *)sipa_find_trnk_by_trnk(ud_ptr->trkGrp NBB_CCXT);
        }

        if ((ip2TrkPtr) && (ip2TrkPtr->cfgInfo.dstFqdnTransport.pres) &&
            (ip2TrkPtr->cfgInfo.dstFqdnTransport.val == TGP_SIPDSTFQDNTRANSPORT_TLS))
        {
          port = SFW_CB.lpoc[lpocIdx].tlsPort;
        }
        else
          port = SFW_CB.lpoc[lpocIdx].port;

        if(SFW_CB.lpoc[lpocIdx].ip.type == AMB_INETWK_ADDR_TYPE_IPV6)
        {
            OS_SPRINTF (localAddress, "[%s]:%d", tmpaddress, port);
        }
        else
        {
            OS_SPRINTF (localAddress, "%s:%d", tmpaddress, port);
        }
    }
    else
    {
        return rc;
    }
    for(offset = SIPM_HEADER_FIND(*ips, SIPM_HEADERS(*ips), token);
        offset != 0;
        offset = SIPM_HEADER_FIND(*ips, offset, token))
    {
        len =0;
        memset(text,0,sizeof(text));
        memset(encipher_text,0,sizeof(encipher_text));
        length = SIPM_PARM_COPY_TEXT_OUT(*ips, SIPM_HEADER_VALUE(*ips, offset),
                text, sizeof(text) - 1);
        if((token == SIP_TOKEN_FROM)||
           (token == SIP_TOKEN_REMOTE_PARTY_ID)||
           (token == SIP_TOKEN_P_ASSERTED_IDENTITY))
        {
            /*When THIG is ON, if OOD non-INVITE request From host contains FQDN, do not  tokenize it */
            /*When THIG is ON, if OOD non-INVITE request From host contains IP, always tokenize it*/
            /*the From tokenization of OOD non-INVITE is not related to sipSrcFqdn*/
            /*For INVITE sent to untrusted, From/PAID/Remote-Party-Id host contains sipSrcFQDN, VSFW won't encrypt them when THIG is ON*/
            if(method == SIP_TOKEN_INVITE||
               method == SIP_TOKEN_BYE||
               method == SIP_TOKEN_ACK||
               method == SIP_TOKEN_PRACK||
               method == SIP_TOKEN_UPDATE||
               method == SIP_TOKEN_CANCEL)
            {
                if((ip2TrkPtr)&&(ip2TrkPtr->cfgInfo.srcFqdn.pres==TRUE))
                    continue;
            }
            else 
            {
                get_ipversion(*ips,token,&ipversion NBB_CCXT);
                if(ipversion == AF_UNSPEC)
                    continue;
            }
        }

        if(token == SIP_TOKEN_HISTORY_INFO)
        {
            if(sfw_dfa_thig_encipherURI(text,length,encipher_text,&len,0) != SIP_E_OK)
            {
                continue;
            }
        }
        else
        {
            if(sfw_dfa_thig_encipherURI(text,length,encipher_text,&len,1) != SIP_E_OK)
            {
                continue;
            }
        }

        len = len>MAX_THIG_HEADER_LEN?MAX_THIG_HEADER_LEN:len;
        encipher_text[len]=0;

        if((token == SIP_TOKEN_FROM)||
           (token == SIP_TOKEN_TO)||
           (token == SIP_TOKEN_CONTACT)||
           (token == SIP_TOKEN_REMOTE_PARTY_ID)||
           (token == SIP_TOKEN_P_ASSERTED_IDENTITY))
        {
            if(sfw_thig_add_sfw_address(ips,encipher_text,&len,localAddress,0  NBB_CCXT) != SIP_E_OK)
            {
                continue;
            }
        }
        len = len > MAX_THIG_HEADER_LEN?MAX_THIG_HEADER_LEN:len;
        encipher_text[len] = 0;
        SFED_TRACE(NBB_DETAIL_TRC, ("%s: encipher_text:%s, length:%d\n",__FUNCTION__,encipher_text, len));

        length = SIPM_PARM_COPY_TEXT_IN (ips,SIPM_HEADER_VALUE(*ips, offset),encipher_text,len);
        if( len != length )
        {
            continue;
        }
    }
    return rc;
}

NBB_USHORT sfw_thig_add_sfw_address(SIP_IPS **ips,NBB_CHAR    *uriStr,
                               NBB_INT     *uriLen,NBB_CHAR *localAddress,
                               NBB_INT     ifNotTel NBB_CCXT_T NBB_CXT)
{

    NBB_SHORT   lpocId = 0;
    SFW_USER_DATA *ud_ptr = NULL;
    NBB_CHAR *start, *end,*userName;
    NBB_USHORT rc = SIP_E_OK;
    NBB_BUF_SIZE length = 0,userNameLen=0;
    NBB_CHAR text[MAX_THIG_HEADER_LEN]={0},paramStr[128]={0};

    memset(text,0,sizeof(text));
    memset(paramStr,0,sizeof(paramStr));

    SFED_TRACE (NBB_DETAIL_TRC, ("%s: Entering \n", __FUNCTION__));

    start = OS_STRSTR(uriStr,sfw_dfa_thig_markHead);
    if(start == NULL)
    {
        rc = SIP_E_FAIL;
        goto ADD_SFW_ADDRESS;
    }

    end = OS_STRSTR(start,sfw_dfa_thig_markTail);
    if(end == NULL)
    {
        rc = SIP_E_FAIL;
        goto ADD_SFW_ADDRESS;
    }

    if(((NBB_INT)(end-start)<=sfw_dfa_thig_markHeadLen)||((NBB_INT)(end-start)>128))
    {
        rc = SIP_E_FAIL;
        goto ADD_SFW_ADDRESS;
    }
    OS_STRNCPY(paramStr,start,end-start+sfw_dfa_thig_markTailLen);

    OS_STRNCPY(text,uriStr,start-uriStr);
    OS_STRCAT(text,localAddress);
    OS_STRCAT(text,sfw_dfa_thig_tokenHead);
    OS_STRCAT(text,paramStr);
    OS_STRCAT(text,end+sfw_dfa_thig_markTailLen);

    *uriLen = OS_STRLEN(text);
    OS_STRNCPY(uriStr,text,*uriLen);
ADD_SFW_ADDRESS:
    return rc;
}

NBB_USHORT sfw_thig_decipher_msg(SIP_IPS **ips NBB_CCXT_T NBB_CXT)
{
    NBB_USHORT rc = SIP_E_OK;
    NBB_BOOL request = IS_REQUEST(*ips);
    NBB_ULONG token = SIP_TOKEN_NULL;
    SFW_USER_DATA *ud_ptr = NULL;
    NBB_UINT flags = 0;
    NBB_SHORT status_code;
    NBB_CHAR ood_request = FALSE;

    NBB_TRC_ENTRY(__FUNCTION__);

    SFED_TRACE (NBB_DETAIL_TRC, ("%s: Entering \n", __FUNCTION__));
    if(*ips == NULL)
    {
        SFED_TRACE(NBB_DETAIL_TRC, ("%s: wrong parameter ips:0x%p \n",
                 __FUNCTION__, *ips ));
        rc = SIP_E_FAIL;
        goto THIG_DECIPHER_MSG;
    }

    status_code = *SIPM_REASON_CODE(*ips);

    ud_ptr = (SFW_USER_DATA*)SIPM_USER_DATA_PTR(*ips);
    if(ud_ptr == NULL || 
            !(ud_ptr->element & SFW_USER_DATA_PEERNET) ||
            (ud_ptr->peernetId == 0))
    {
        SFED_TRACE (NBB_DETAIL_TRC, ("%s: invalid user data \n", __FUNCTION__));
        rc = SIP_E_OK;
        goto THIG_DECIPHER_MSG;
    }

    token = SIPM_PARM_GET_TOKEN(*ips, SIPM_METHOD(*ips));
    if(!(ud_ptr->funcFlags & SFW_FUNC_THIG))
    {
        /* in case the flag was turned off recently, we may need to decipher also */
        NBB_UINT allowedDuration = 0;
        if(request)
        {
            if(ud_ptr->element & SFW_USER_DATA_TIMESTAMP && 
                    ud_ptr->maxCallDuration > 0)
            {
                if(token == SIP_TOKEN_BYE)
                {
                    allowedDuration = ud_ptr->maxCallDuration * 3600 * 2;
                }
                else
                {
                    allowedDuration = ud_ptr->maxCallDuration * 3600;
                }

            }
        }
        else
        {
            allowedDuration = 180;
        }

        if(allowedDuration > 0 && 
                (ud_ptr->msgRecvTime > (ud_ptr->thigOnOffTime + allowedDuration)))
        {
            SFW_DEBUG(NBB_DETAIL_TRC, 
                      ("%s: do not decipher, recvTime:%d, onOffTime %d, allowDuration: %d\n", 
                       __FUNCTION__, ud_ptr->msgRecvTime, ud_ptr->thigOnOffTime, allowedDuration));
            rc = SIP_E_OK;
            goto THIG_DECIPHER_MSG;
        }
    }

    /* it better to init thig in common place, not for each message.*/
    //sfw_dfa_thig_init();

    /*From Trust to Untrust*/
    if(!(ud_ptr->lpocId & SFW_LPOC_UNTRUSTED_MASK))
    {
        rc = SIP_E_OK;
        goto THIG_DECIPHER_MSG;
    }
    if(!(ud_ptr->funcFlags& SFW_FUNC_THIG))
    {
        rc = SIP_E_OK; 
        goto THIG_DECIPHER_MSG;
    }

    status_code = *SIPM_REASON_CODE(*ips);

    SFED_TRACE(NBB_DETAIL_TRC, ("%s:token:0x%x, request:%d status_code:%d\n",__FUNCTION__,token,request,status_code));

    if(request)
    {
        sfw_decipher_r_uri(ips,ud_ptr NBB_CCXT);
        sfw_decipher_header(ips,ud_ptr,SIP_TOKEN_TO NBB_CCXT);
        sfw_decipher_header(ips,ud_ptr,SIP_TOKEN_ROUTE NBB_CCXT);
    }
    else
    {
        sfw_decipher_header(ips,ud_ptr,SIP_TOKEN_VIA NBB_CCXT);
        sfw_decipher_header(ips,ud_ptr,SIP_TOKEN_RECORD_ROUTE NBB_CCXT);
        sfw_decipher_header(ips,ud_ptr,SIP_TOKEN_FROM NBB_CCXT);
    }
    sfw_decipher_header(ips,ud_ptr,SIP_TOKEN_HISTORY_INFO NBB_CCXT);
    sfw_decipher_header(ips,ud_ptr,SIP_TOKEN_DIVERSION NBB_CCXT);

    if(token == SIP_TOKEN_REFER)
    {
        sfw_decipher_header(ips,ud_ptr,SIP_TOKEN_REFERRED_BY NBB_CCXT);
        sfw_decipher_header(ips,ud_ptr,SIP_TOKEN_REFER_TO NBB_CCXT);
    }

    if(token == SIP_TOKEN_INVITE)
    {
        sfw_decipher_header(ips,ud_ptr,SIP_TOKEN_REFERRED_BY NBB_CCXT);
    }

THIG_DECIPHER_MSG:
    return rc;
}
INLINE NBB_USHORT sfw_decipher_r_uri(SIP_IPS **ips,SFW_USER_DATA *userDataPtr NBB_CCXT_T NBB_CXT)
{
    NBB_USHORT rc = SIP_E_OK;
    NBB_BUF_SIZE offset =0;
    NBB_BUF_SIZE length = 0, len = 0;
    NBB_BYTE raw_text[MAX_THIG_HEADER_LEN],decipher_text[MAX_THIG_HEADER_LEN] ;
    NBB_BYTE *start=NULL;
    SFED_TRACE(NBB_DETAIL_TRC, ("%s: Enter\n",__FUNCTION__));
    memset(raw_text,0,sizeof(raw_text));
    memset(decipher_text,0,sizeof(decipher_text));

    length = SIPM_PARM_COPY_TEXT_OUT(*ips, SIPM_REQUEST_URI(*ips ),raw_text, MAX_THIG_HEADER_LEN - 1);
    raw_text[length] = 0;
    sfw_dfa_thig_replace_tokenized(ips,userDataPtr,raw_text,&length NBB_CCXT);
    if(sfw_dfa_thig_decipherURI(raw_text,length,decipher_text,&len) == SIP_E_OK)
    {
        if(len > length)
        {
            SFED_TRACE(NBB_DETAIL_TRC,("%s: deciphered msg lenght:%d, enciphered msg length:%d\n", __FUNCTION__,len, length));
            return rc;
        }
        length = SIPM_PARM_COPY_TEXT_IN(ips,SIPM_REQUEST_URI(*ips),decipher_text,len);
    }
    return rc;
}

INLINE NBB_USHORT sfw_decipher_header(SIP_IPS **ips, SFW_USER_DATA *userDataPtr,NBB_ULONG token  NBB_CCXT_T NBB_CXT)
{
    NBB_USHORT rc = SIP_E_OK;
    NBB_BUF_SIZE offset =0;
    NBB_BUF_SIZE length = 0, len = 0;
    NBB_BYTE raw_text[MAX_THIG_HEADER_LEN],decipher_text[MAX_THIG_HEADER_LEN] ;
    NBB_BYTE *start=NULL;

    SFED_TRACE (NBB_DETAIL_TRC, ("%s: Entering \n", __FUNCTION__));

    for(offset = SIPM_HEADER_FIND(*ips, SIPM_HEADERS(*ips), token);
        offset != 0;
        offset = SIPM_HEADER_FIND(*ips, offset, token))
    {
        len =0;
        memset(raw_text,0,sizeof(raw_text));
        memset(decipher_text,0,sizeof(decipher_text));
        length = SIPM_PARM_COPY_TEXT_OUT(*ips, SIPM_HEADER_VALUE(*ips, offset),raw_text, MAX_THIG_HEADER_LEN - 1);
        raw_text[length]=0;
        sfw_dfa_thig_replace_tokenized(ips,userDataPtr,raw_text,&length NBB_CCXT);
        if(sfw_dfa_thig_decipherURI(raw_text,length,decipher_text,&len) == SIP_E_OK)
        {
            if(token == SIP_TOKEN_TO)
            {
                userDataPtr->thigFlags = token;
                SFED_TRACE(NBB_DETAIL_TRC, ("%s: userDataPtr->thigFlags:0x%x\n",__FUNCTION__,userDataPtr->thigFlags));
            }
            if(len > length)
            {
                SFED_TRACE(NBB_DETAIL_TRC,("%s: deciphered msg lenght:%d, enciphered msg length:%d\n", __FUNCTION__,len, length));
                continue;
            }
            length = SIPM_PARM_COPY_TEXT_IN (ips,SIPM_HEADER_VALUE(*ips, offset),decipher_text,len);
            if( len != length )
            {
                continue;
            }
        }
    }
    return rc;
}

INLINE NBB_VOID sfw_dfa_thig_init()
{
    NBB_INT i;

    for(i=256; i-- > 0; ) { sfw_dfa_thig_digits[i] = 0; }
    for(i=64; i-- > 0; ) {
        sfw_dfa_thig_digits[(NBB_INT) sfw_dfa_thig_chars[i]] = i;
    }
    sfw_dfa_thig_tokenHeadLen = strlen(sfw_dfa_thig_tokenHead);
    sfw_dfa_thig_markHeadLen = strlen(sfw_dfa_thig_markHead);
    sfw_dfa_thig_markTailLen = strlen(sfw_dfa_thig_markTail);
}

INLINE NBB_INT sfw_dfa_thig_encipher(
        NBB_CHAR      *inpStr,
        NBB_INT       inpLen,
        NBB_CHAR      *res)
{
    NBB_INT    len, chnb;
    NBB_CHAR   key, nibble;

    SFED_TRACE (NBB_DETAIL_TRC, ("%s: Entering \n", __FUNCTION__));

    /* use the input length of the string to create a key for the enciphter - Jerry */
    key = inpLen & 0xfc;

    /* Add the line length */
    *res ++ = sfw_dfa_thig_chars[((inpLen & 0xfc0) >> 6) & 0x3f];
    *res ++ = sfw_dfa_thig_chars[inpLen & 0x3f];

    len = 2;
    chnb = 0;

    while(inpLen > 0) {
        /*
         *          ** 3 consecutive bytes are encoded into 4 chars. It means that
         *                   ** there are trailing chars that are not of interest and
         *                   that
         *                            ** we shall have to drop while deciphering.
         *                                     */
        nibble = ((*inpStr) & 0xfc) >> 2;
        *res ++ = sfw_dfa_thig_chars[(nibble+(key++))& 0x3f];

        nibble = ((*inpStr++) & 0x03) << 4;
        nibble |= ((*inpStr) & 0xf0) >> 4;
        *res ++ = sfw_dfa_thig_chars[(nibble+(key++))& 0x3f];

        nibble = ((*inpStr++) & 0x0f) << 2;
        nibble |= ((*inpStr) & 0xc0) >> 6;
        *res ++ = sfw_dfa_thig_chars[(nibble+(key++))& 0x3f];

        nibble = ((*inpStr++) & 0x3f);
        *res ++ = sfw_dfa_thig_chars[(nibble+(key++))& 0x3f];

        len += 4;
        chnb += 4;
        inpLen -= 3;

        if(chnb == 40)
        {
            *res ++ = 'l';
            *res ++ = '.';
            *res ++ = 'c';
            len += 3;
            chnb = 0;
        }
    }

    return len;
}

NBB_INT sfw_dfa_thig_replace_tokenized(
        SIP_IPS **ips,
        SFW_USER_DATA *ud_ptr,
        NBB_CHAR*      uriStr,
        NBB_INT*       uriLen  NBB_CCXT_T NBB_CXT)
{
    NBB_CHAR *tokenized = NULL,*lpocip = NULL,*thigTail=NULL,*str=NULL;
    NBB_INT len =0 ,addrLen = 0;
    NBB_BUF_SIZE length = 0;
    NBB_USHORT rc = SIP_E_OK;
    NBB_BYTE tmpaddress[INET6_ADDRSTRLEN],localAddress[64]={0};
    NBB_CHAR text[MAX_THIG_HEADER_LEN]={0};
    NBB_SHORT lpocIdx = ud_ptr->lpocId & SFW_LPOC_ID_MASK;

    SFED_TRACE (NBB_DETAIL_TRC, ("%s: Entering \n", __FUNCTION__));

    tokenized = OS_STRSTR(uriStr,sfw_dfa_thig_tokenHead);
    if(tokenized == NULL)
    {
        rc = SIP_E_FAIL;
        goto THIG_REPLACE_TOKENIZED;
    }

    length = 128;

    /* Update lpocIdx when msg comes from trusted side,
     * since ud_ptr->lpocId is unavailable */
    if(!(ud_ptr->lpocId & SFW_LPOC_UNTRUSTED_MASK))
    {
        if(ud_ptr->element & SFW_USER_DATA_PEERNET) 
        {
            if(SFW_CB.peernet[ud_ptr->peernetId - 1] != NULL) 
            {
                lpocIdx = SFW_CB.peernet[ud_ptr->peernetId - 1]->lpoc_id - 1;
            }
        }
    }

    NTL_INET_ADDR_TO_STRING(&SFW_CB.lpoc[lpocIdx].ip, 0, (NBB_BYTE*)tmpaddress, &length);
    if(SFW_CB.lpoc[lpocIdx].ip.type == AMB_INETWK_ADDR_TYPE_IPV6)
    {
        OS_SPRINTF (localAddress, "[%s]:%d", tmpaddress,SFW_CB.lpoc[lpocIdx].port);
    }
    else
    {
        OS_SPRINTF (localAddress, "%s:%d", tmpaddress,SFW_CB.lpoc[lpocIdx].port);
    }

    addrLen = OS_STRLEN(localAddress);
    lpocip = OS_STRSTR(uriStr,localAddress);

    if(lpocip == NULL)
    {
        rc = SIP_E_FAIL;
        goto THIG_REPLACE_TOKENIZED;
    }
    str = text;
    OS_STRNCPY(text,uriStr,lpocip-uriStr);
    str += lpocip-uriStr;
    thigTail = OS_STRSTR(uriStr,sfw_dfa_thig_markTail);

    if((thigTail == NULL)||(thigTail-tokenized<=sfw_dfa_thig_tokenHeadLen))
    {
        rc = SIP_E_FAIL;
        goto THIG_REPLACE_TOKENIZED;
    }

    OS_STRNCPY(str,tokenized+sfw_dfa_thig_tokenHeadLen,thigTail-tokenized-sfw_dfa_thig_tokenHeadLen+sfw_dfa_thig_markTailLen);
    str += thigTail-tokenized-sfw_dfa_thig_tokenHeadLen+sfw_dfa_thig_markTailLen;

    if(tokenized-lpocip>addrLen)
    {
        OS_STRNCPY(str,lpocip+addrLen,tokenized-lpocip-addrLen);
    }

    OS_STRCAT(text,thigTail+sfw_dfa_thig_markTailLen);

    memset(uriStr,0,*uriLen);
    *uriLen = *uriLen - sfw_dfa_thig_tokenHeadLen - addrLen;
    OS_STRNCPY(uriStr,text,*uriLen);

THIG_REPLACE_TOKENIZED:
    SFED_TRACE(NBB_DETAIL_TRC, ("%s:text:%s uriLen:%d\n",__FUNCTION__,uriStr,*uriLen));
    return rc;
}

NBB_INT sfw_dfa_thig_decipherURI(
        NBB_CHAR *      uriStr,
        NBB_INT         uriLen,
        NBB_CHAR *     resStr,
        NBB_INT  *     resLen)
{
    NBB_USHORT rc = SIP_E_OK;
    NBB_CHAR *start=NULL, *end=NULL, *tokenized_by=NULL;
    NBB_CHAR *str=NULL;
    NBB_INT len=0,length=0;

    SFED_TRACE (NBB_DETAIL_TRC, ("%s: Entering \n", __FUNCTION__));

    str = resStr;
    tokenized_by = OS_STRSTR(uriStr,sfw_dfa_thig_tokenByHead);

    if(tokenized_by != NULL)
    {
        if(tokenized_by + sfw_dfa_thig_tokenByHeadLen - uriStr == uriLen)
        {
            *tokenized_by='\0';
        }
        else
        {
            memcpy(tokenized_by,tokenized_by+sfw_dfa_thig_tokenByHeadLen,(uriLen-(tokenized_by+sfw_dfa_thig_tokenByHeadLen-uriStr)));
            uriStr[uriLen-sfw_dfa_thig_tokenByHeadLen]='\0';
        }
        uriLen -= sfw_dfa_thig_tokenByHeadLen;
    }

    start = OS_STRSTR(uriStr,sfw_dfa_thig_markHead);
    if(start == NULL)
    {
        rc = SIP_E_FAIL;
        goto THIG_DECIPHERURI;
    }

    end = OS_STRSTR(start,sfw_dfa_thig_markTail);
    if(end == NULL)
    {
        rc = SIP_E_FAIL;
        goto THIG_DECIPHERURI;
    }

    if((NBB_INT)(end-start)<sfw_dfa_thig_markHeadLen) 
    {
        rc = SIP_E_FAIL;
        goto THIG_DECIPHERURI;
    }

    /*copy first part*/
    if(start != uriStr)
    {
        OS_STRNCPY(str,uriStr,start-uriStr);
        str += start - uriStr;
    }
    start += sfw_dfa_thig_markHeadLen;

    if(sfw_dfa_thig_decipher(start, str, &len)==0)
    {
        rc = SIP_E_FAIL;
        goto THIG_DECIPHERURI;
    }
    str += len;
    end += sfw_dfa_thig_markTailLen;
 
    len  = uriLen - (end - uriStr );

    if(OS_STRSTR(end,sfw_dfa_thig_markHead)!=NULL)
    {
        sfw_dfa_thig_decipherURI(end, len,str,&length);
        str += length;
        len  = 0;
    }

    if(len>0)
    {
        OS_STRNCPY(str,end,len);
        str += len;
    }
    
    *resLen = (NBB_INT)(str - resStr);
THIG_DECIPHERURI:
    SFED_TRACE(NBB_DETAIL_TRC, ("%s: resStr:%s, length:%d\n",__FUNCTION__,resStr, *resLen));
    return rc;
}

INLINE NBB_INT sfw_dfa_thig_decipher(
        NBB_CHAR*    inpStr,
        NBB_CHAR*    resStr,
        NBB_INT  *    resLen)
{
    NBB_INT    i=0;
    NBB_INT    len=0, chnb=0;
    NBB_CHAR   *chr=NULL;
    NBB_CHAR   key, nibbles[4];
    NBB_CHAR   *inpCpy=NULL;

    SFED_TRACE (NBB_DETAIL_TRC, ("%s: Entering \n", __FUNCTION__));

    key = 0;
    inpCpy = inpStr;

    /* Get the key and line length */
    len = 0;
    for(i = 2; i-- > 0; ) {
        len <<= 6;
        len += (sfw_dfa_thig_digits[(int) *inpStr++]) & 0x3f;
    }
    len &= 0xfff;
    if(len > sfw_dfa_thig_bufLen)
    {
        return 0;
    }

    key = len & 0xfc;

    *resLen = len;
    chnb = 0;

    while(len > 0) {
        chr = nibbles;
        for(i = 4; i-- > 0; ) {
            *chr++ = (sfw_dfa_thig_digits[(int) *inpStr++] - (key++)) & 0x3f;
            chnb ++;
        }
        chr = nibbles;
        *resStr = ((*chr++) & 0x3f) << 2;
        *resStr++ |= ((*chr) & 0x30) >> 4;
        if(--len > 0) {
            *resStr = ((*chr++) & 0x0f) << 4;
            *resStr++ |= ((*chr) & 0x3c) >> 2;
            if(--len > 0) {
                *resStr = ((*chr++) & 0x03) << 6;
                *resStr++ |= ((*chr) & 0x3f);
                --len;
            }
        }

        if(chnb == 40)
        {
            *inpStr++;
            *inpStr++;
            *inpStr++;
            chnb = 0;
        }
    }

    return (inpStr - inpCpy);
}

INLINE NBB_INT sfw_dfa_thig_encipherURI(
        NBB_CHAR    *uriStr,
        NBB_INT     uriLen,
        NBB_CHAR    *resStr,
        NBB_INT     *resLen,
        NBB_INT     ifNotTel)
{
    NBB_CHAR     *str = NULL;
    NBB_CHAR     *colon = NULL, *start = NULL, *end = NULL;
    NBB_CHAR     dquote;
    NBB_INT      len=0,length=0;
    NBB_USHORT rc = SIP_E_OK;

    str = resStr;

    /*
     *  ** Look for the start of URI.
     *      ** Look for the first ':' that is not between '"'.
     *          */
    start = uriStr;
    len = uriLen;
    dquote = 0;
    while((len--) > 0) {
        if((dquote == 0) && (*start == ':')) {
            break;
        }
        if(*start == '"') {
            dquote = 1 - dquote;
        }
        start++;
    }
    if(len == (NBB_INT) -1) {
        /*
         *      ** No sent-protocol : we shall cipher everything.
         *              */
        start = uriStr;
        len = uriLen;
    } else {
        /* FR#35926 : Check if it is a tel URI */
        if( (ifNotTel) &&
            (*((unsigned int*)(start-3)) == *((unsigned int*) "tel:")) )
        {
            rc = SIP_E_FAIL;
            return rc;
        }
        start++;
    }
    colon = start;

    /*
     *  ** Search for the user-name : it is ended by '@'
     *      */
    while((len--) > 0) {
        if(*start++ == '@') {
            break;
        }
    }
    if(len == (int) -1) {
        /*
         *      ** No user-name : we shall cipher everything.
         *              */
        start = colon;
        len = uriLen - (start - uriStr);
    }

    /*
     *  ** Copy the first part of the URI if any : it remains unchanged.
     *      */
    if(start != uriStr) {
        memcpy(str, uriStr, start - uriStr);
        str += start - uriStr;
    }

    /*
     *  ** Look for the end of URI.
     *      ** look for the next ';' or or '>' or '?' or end of
     *      string
     *          */
    end = start;
    while((len--) > 0) {
        if( (*end == ';') || (*end == '>') || (*end == '?') ){
            break;
        }
        end++;
    }

    /*
     *  ** Compute the tokenized form of the header.
     *      */
    memcpy(str, sfw_dfa_thig_markHead, sfw_dfa_thig_markHeadLen);
    str += sfw_dfa_thig_markHeadLen;
    str += sfw_dfa_thig_encipher(start, (int)(end - start), str);
    memcpy(str, sfw_dfa_thig_markTail, sfw_dfa_thig_markTailLen);
    str += sfw_dfa_thig_markTailLen;
    if(OS_STRSTR(resStr,sfw_dfa_thig_tokenByHead)==NULL)
    {
        memcpy(str,sfw_dfa_thig_tokenByHead,sfw_dfa_thig_tokenByHeadLen);
        str += sfw_dfa_thig_tokenByHeadLen;
    }

    /*
     *  ** Add the remaining part of the
     *  URI if any : it remains
     *  unchanged.
     *      */
    len = uriLen - (end - uriStr);
    if(OS_STRSTR(end,"@") != NULL)
    {
        sfw_dfa_thig_encipherURI(end,len,str,&length,ifNotTel);
        str += length;
        len = 0;
    }
    if(len > 0) {
        memcpy(str, end, len);
        str += len;
    }

    *resLen = (int) (str - resStr);
    rc = SIP_E_OK;
    return rc;
}
#endif
