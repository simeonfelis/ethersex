/* uip configuration */

#ifndef __UIP_CONF_H__
#define __UIP_CONF_H__

#include <inttypes.h>
#include "config.h"

/**
 * 8 bit datatype
 *
 * This typedef defines the 8-bit type used throughout uIP.
 *
 * \hideinitializer
 */
typedef uint8_t u8_t;

/**
 * 16 bit datatype
 *
 * This typedef defines the 16-bit type used throughout uIP.
 *
 * \hideinitializer
 */
typedef uint16_t u16_t;

/**
 * Statistics datatype
 *
 * This typedef defines the dataype used for keeping statistics in
 * uIP.
 *
 * \hideinitializer
 */
typedef unsigned short uip_stats_t;

/**
 * Maximum number of TCP connections.
 *
 * \hideinitializer
 */
#define UIP_CONF_MAX_CONNECTIONS 3

/**
 * Maximum number of listening TCP ports.
 *
 * \hideinitializer
 */
#define UIP_CONF_MAX_LISTENPORTS 10

/**
 * uIP buffer size.
 *
 * \hideinitializer
 */
#define UIP_CONF_BUFFER_SIZE     NET_MAX_FRAME_LENGTH

/**
 * CPU byte order.
 *
 * \hideinitializer
 */
#define UIP_CONF_BYTE_ORDER      UIP_LITTLE_ENDIAN

/**
 * Logging on or off
 *
 * \hideinitializer
 */
#ifdef DEBUG
#   define UIP_CONF_LOGGING         1
#else
#   define UIP_CONF_LOGGING         0
#endif

/** TCP support on or off */
#ifdef TCP_SUPPORT
#   define UIP_CONF_TCP             1
#else
#   define UIP_CONF_TCP             0
#endif

/** UDP support on or off */
#ifdef UDP_SUPPORT
#   define UIP_CONF_UDP             1
#else
#   define UIP_CONF_UDP             0
#endif

#define UIP_CONF_UDP_CONNS            5

/**
 * UDP checksums on or off
 *
 * \hideinitializer
 */
#if defined(IPV6_SUPPORT) || !defined(BOOTLOADER_SUPPORT)
#define UIP_CONF_UDP_CHECKSUMS   1
#else
#define UIP_CONF_UDP_CHECKSUMS   0
#endif

/**
 * uIP statistics on or off
 *
 * \hideinitializer
 */
#if defined(IPSTATS_SUPPORT)
#define UIP_CONF_STATISTICS      1
#else
#define UIP_CONF_STATISTICS      0
#endif

#ifdef IPV6_SUPPORT
#  define UIP_CONF_IPV6          1
#else
#  define UIP_CONF_IPV6          0
#endif

#ifdef BROADCAST_SUPPORT
#  define UIP_CONF_BROADCAST     1
#else
#  define UIP_CONF_BROADCAST     0
#endif

#define UIP_ARCH_ADD32           0
#define UIP_ARCH_CHKSUM          0

#define RFM12_LLH_LEN            2


#ifdef ENC28J60_SUPPORT
/* On stand-alone ethersex and on rfm12/zbus-bridge always use 14 byte LLH. */
#  define __LLH_LEN  14

#elif defined(RFM12_IP_SUPPORT)	  /* cf. zbus */
#  define __LLH_LEN              RFM12_LLH_LEN

#elif defined(ZBUS_SUPPORT)	  /* cf. rfm12 */
#  define __LLH_LEN  0

#elif defined(USB_NET_SUPPORT)	  /* cf. rfm12 */
#  define __LLH_LEN  0
#endif


#ifndef UIP_MULTI_STACK
#  define UIP_CONF_LLH_LEN       __LLH_LEN
#endif /* not UIP_MULTI_STACK */


/**
 * Some forward declarations
 *
 */

struct __uip_conn;
typedef struct __uip_conn uip_conn_t;
struct __uip_udp_conn;
typedef struct __uip_udp_conn uip_udp_conn_t;

/**
 * Repressentation of an IP address.
 *
 */
typedef u16_t uip_ip4addr_t[2];
typedef u16_t uip_ip6addr_t[8];
#if UIP_CONF_IPV6
typedef uip_ip6addr_t uip_ipaddr_t;
#else /* UIP_CONF_IPV6 */
typedef uip_ip4addr_t uip_ipaddr_t;
#endif /* UIP_CONF_IPV6 */

enum {
/* BE CAREFUL, the order of the stacks in this enum
   always has to match the uip_stacks definition in
   uip_multi.c! */
#if defined(RFM12_IP_SUPPORT)
  STACK_RFM12,
#endif
#if defined(ZBUS_SUPPORT)
  STACK_ZBUS,
#endif
#if defined(OPENVPN_SUPPORT)
  STACK_OPENVPN,
#endif
#if defined(USB_NET_SUPPORT)
  STACK_USB,
#endif
#if defined(ENC28J60_SUPPORT)
  STACK_ENC,
#endif

  /* STACK_LEN must be the last! */
  STACK_LEN
};


#if UIP_MULTI_STACK
#  include "uip_multi.h"

#else /* not UIP_MULTI_STACK */

#define RFM12_BRIDGE_OFFSET  0
#define ZBUS_BRIDGE_OFFSET   0
#define USB_BRIDGE_OFFSET    0

#  define uip_stack_get_active()   do { } while(0)
#  define uip_stack_set_active(i)  do { } while(0)

#endif

extern u16_t upper_layer_chksum(u8_t);
u8_t uip_ipaddr_prefixlencmp(uip_ip6addr_t _a, uip_ip6addr_t _b, u8_t prefix);

#endif /* __UIP_CONF_H__ */
