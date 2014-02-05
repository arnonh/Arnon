#ifndef __NETBIOS_H__
#define __NETBIOS_H__

#include "lwip/udp.h"
#include "lwip/netif.h"


/** NetBIOS name of LWIP device
 * This must be uppercase until NETBIOS_STRCMP() is defined to a string
 * comparision function that is case insensitive.
 * If you want to use the netif's hostname, use this (with LWIP_NETIF_HOSTNAME):
 * (ip_current_netif() != NULL ? ip_current_netif()->hostname != NULL ? ip_current_netif()->hostname : "" : "")
 */
#ifndef NETBIOS_LWIP_NAME
#define NETBIOS_LWIP_NAME "CHIBIOSSERVER"
#endif

/** Since there's no standard function for case-insensitive string comparision,
 * we need another define here:
 * define this to stricmp() for windows or strcasecmp() for linux.
 * If not defined, comparision is case sensitive and NETBIOS_LWIP_NAME must be
 * uppercase
 */
#ifndef NETBIOS_STRCMP
#define NETBIOS_STRCMP(str1, str2) strcmp(str1, str2)
#endif

/** default port number for "NetBIOS Name service */
#define NETBIOS_PORT     137

/** size of a NetBIOS name */
#define NETBIOS_NAME_LEN 16

/** The Time-To-Live for NetBIOS name responds (in seconds)
 * Default is 300000 seconds (3 days, 11 hours, 20 minutes) */
#define NETBIOS_NAME_TTL 300000

/** NetBIOS header flags */
#define NETB_HFLAG_RESPONSE           0x8000U
#define NETB_HFLAG_OPCODE             0x7800U
#define NETB_HFLAG_OPCODE_NAME_QUERY  0x0000U
#define NETB_HFLAG_AUTHORATIVE        0x0400U
#define NETB_HFLAG_TRUNCATED          0x0200U
#define NETB_HFLAG_RECURS_DESIRED     0x0100U
#define NETB_HFLAG_RECURS_AVAILABLE   0x0080U
#define NETB_HFLAG_BROADCAST          0x0010U
#define NETB_HFLAG_REPLYCODE          0x0008U
#define NETB_HFLAG_REPLYCODE_NOERROR  0x0000U

/** NetBIOS name flags */
#define NETB_NFLAG_UNIQUE             0x8000U
#define NETB_NFLAG_NODETYPE           0x6000U
#define NETB_NFLAG_NODETYPE_HNODE     0x6000U
#define NETB_NFLAG_NODETYPE_MNODE     0x4000U
#define NETB_NFLAG_NODETYPE_PNODE     0x2000U
#define NETB_NFLAG_NODETYPE_BNODE     0x0000U

/** NetBIOS message header */
#ifdef PACK_STRUCT_USE_INCLUDES
#  include "arch/bpstruct.h"
#endif
PACK_STRUCT_BEGIN
struct netbios_hdr {
  PACK_STRUCT_FIELD(u16_t trans_id);
  PACK_STRUCT_FIELD(u16_t flags);
  PACK_STRUCT_FIELD(u16_t questions);
  PACK_STRUCT_FIELD(u16_t answerRRs);
  PACK_STRUCT_FIELD(u16_t authorityRRs);
  PACK_STRUCT_FIELD(u16_t additionalRRs);
} PACK_STRUCT_STRUCT;
PACK_STRUCT_END
#ifdef PACK_STRUCT_USE_INCLUDES
#  include "arch/epstruct.h"
#endif

/** NetBIOS message name part */
#ifdef PACK_STRUCT_USE_INCLUDES
#  include "arch/bpstruct.h"
#endif
PACK_STRUCT_BEGIN
struct netbios_name_hdr {
  PACK_STRUCT_FIELD(u8_t  nametype);
  PACK_STRUCT_FIELD(u8_t  encname[(NETBIOS_NAME_LEN*2)+1]);
  PACK_STRUCT_FIELD(u16_t type);
  PACK_STRUCT_FIELD(u16_t cls);
  PACK_STRUCT_FIELD(u32_t ttl);
  PACK_STRUCT_FIELD(u16_t datalen);
  PACK_STRUCT_FIELD(u16_t flags);
  PACK_STRUCT_FIELD(ip_addr_p_t addr);
} PACK_STRUCT_STRUCT;
PACK_STRUCT_END
#ifdef PACK_STRUCT_USE_INCLUDES
#  include "arch/epstruct.h"
#endif

/** NetBIOS message */
#ifdef PACK_STRUCT_USE_INCLUDES
#  include "arch/bpstruct.h"
#endif
PACK_STRUCT_BEGIN
struct netbios_resp
{
  struct netbios_hdr      resp_hdr;
  struct netbios_name_hdr resp_name;
} PACK_STRUCT_STRUCT;
PACK_STRUCT_END
#ifdef PACK_STRUCT_USE_INCLUDES
#  include "arch/epstruct.h"
#endif

void netbios_init(void);

#endif /* __NETBIOS_H__ */
