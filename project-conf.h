
#ifndef __PROJECT_SMART_SOCKET_CONF_H__
#define __PROJECT_SMART_SOCKET_CONF_H__

// Smart socket
#define NODE_EQUIPEMENT_TYPE           0x01

// Radio
#define IEEE802154_CONF_PANID           0xABCD
#define RF_CORE_CONF_CHANNEL            25
#define UIP_CONF_LLH_LEN				14

#define COAPTOHTTP_URI  "coaptohttp" /*!< Controller's endpoint used to send CoAP messages to the exterior */
#define CONSUME_PATH  "p=/consumo" /*!< External endpoint used to send periodic consumption readings */

// TODO just for tests
#define NETSTACK_CONF_RDC				nullrdc_driver

/* IP buffer size must match all other hops, in particular the border router. */
/* TODO: fazer testes para ajustar
   #undef UIP_CONF_BUFFER_SIZE
   #define UIP_CONF_BUFFER_SIZE           256
 */

#define UIP_CONF_IPV6_QUEUE_PKT  1 /*!< Enables Queue packet buffer */
#define QUEUEBUF_CONF_NUM  4 /*!< Limits the Queue buffer to 4 packets at a time. Uses ~830 bytes of RAM */

/* Disabling TCP on CoAP nodes. */
#undef UIP_CONF_TCP
#define UIP_CONF_TCP                   0

/* Increase rpl-border-router IP-buffer when using more than 64. */
#undef REST_MAX_CHUNK_SIZE
#define REST_MAX_CHUNK_SIZE            48

/* Estimate your header size, especially when using Proxy-Uri. */
/*
   #undef COAP_MAX_HEADER_SIZE
   #define COAP_MAX_HEADER_SIZE           70
 */

/* Multiplies with chunk size, be aware of memory constraints. */
#undef COAP_MAX_OPEN_TRANSACTIONS
#define COAP_MAX_OPEN_TRANSACTIONS     4

/* Filtering .well-known/core per query can be disabled to save space. */
#undef COAP_LINK_FORMAT_FILTERING
#define COAP_LINK_FORMAT_FILTERING     0
#undef COAP_PROXY_OPTION_PROCESSING
#define COAP_PROXY_OPTION_PROCESSING   0


#define UIP_CONF_IPV6_RPL							0
#define NETSTACK_CONF_WITH_IPV6						1
#define UIP_CONF_ND6_SEND_NA						1
#define UIP_CONF_ROUTER								0
#define UIP_CONF_UDP								1
// Multi Interfaces
#define UIP_CONF_DS6_INTERFACES_NUMBER				1

// Disable link statistics to avoid nbr-table bug
#define LINK_STATS_CONF_ENABLED                     0

/**
 * Está aqui para resolver o problema do Hard Fault por desalinhamento na execução de store/load multiplos.
 * uip_ip6addr_copy é utilizada nos sockets HTTP
 */
#define uip_ipaddr_copy(dest, src) 		memcpy(dest, src, sizeof(uip_ip6addr_t));
#define uip_ip6addr_copy(dest, src)		memcpy(dest, src, sizeof(uip_ip6addr_t));

#endif /* __PROJECT_SMART_SOCKET_CONF_H__ */
