/**
 * @file net_config.h
 * @brief CycloneTCP configuration file
 *
 * @section License
 *
 * Copyright (C) 2010-2017 Oryx Embedded SARL. All rights reserved.
 *
 * This file is part of CycloneTCP Open.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 * @author Oryx Embedded SARL (www.oryx-embedded.com)
 * @version 1.7.6
 **/

#ifndef _NET_CONFIG_H
#define _NET_CONFIG_H

//Trace level for TCP/IP stack debugging
#define MEM_TRACE_LEVEL          0
#define NIC_TRACE_LEVEL          0
#define ETH_TRACE_LEVEL          0
#define ARP_TRACE_LEVEL          0
#define IP_TRACE_LEVEL           0
#define IPV4_TRACE_LEVEL         0
#define IPV6_TRACE_LEVEL         0
#define ICMP_TRACE_LEVEL         0
#define IGMP_TRACE_LEVEL         0
#define ICMPV6_TRACE_LEVEL       0
#define MLD_TRACE_LEVEL          0
#define NDP_TRACE_LEVEL          0
#define UDP_TRACE_LEVEL          0
#define TCP_TRACE_LEVEL          0
#define SOCKET_TRACE_LEVEL       0
#define RAW_SOCKET_TRACE_LEVEL   0
#define BSD_SOCKET_TRACE_LEVEL   0
#define SLAAC_TRACE_LEVEL        0
#define DHCP_TRACE_LEVEL         0
#define DHCPV6_TRACE_LEVEL       0
#define DNS_TRACE_LEVEL          0
#define MDNS_TRACE_LEVEL         0
#define NBNS_TRACE_LEVEL         0
#define LLMNR_TRACE_LEVEL        0
#define FTP_TRACE_LEVEL          0
#define HTTP_TRACE_LEVEL         0
#define SMTP_TRACE_LEVEL         0
#define SNTP_TRACE_LEVEL         0
#define STD_SERVICES_TRACE_LEVEL 0

//Number of network adapters
#define NET_INTERFACE_COUNT 1

//Size of the multicast MAC filter
#define MAC_MULTICAST_FILTER_SIZE 12

//TCP/IP stack tick interval
#define NET_TICK_INTERVAL   100

//CRC32 calculation using a pre-calculated lookup table
#define ETH_FAST_CRC_SUPPORT ENABLED

//IPv4 support
#define IPV4_SUPPORT ENABLED
//Size of the IPv4 multicast filter
#define IPV4_MULTICAST_FILTER_SIZE 4

//IPv4 fragmentation support
#define IPV4_FRAG_SUPPORT ENABLED
//Maximum number of fragmented packets the host will accept
//and hold in the reassembly queue simultaneously
#define IPV4_MAX_FRAG_DATAGRAMS 4
//Maximum datagram size the host will accept when reassembling fragments
#define IPV4_MAX_FRAG_DATAGRAM_SIZE 8192

//Size of ARP cache
#define ARP_CACHE_SIZE 8
//Maximum number of packets waiting for address resolution to complete
#define ARP_MAX_PENDING_PACKETS 2

//IGMP support
#define IGMP_SUPPORT ENABLED

//IPv6 support
#define IPV6_SUPPORT DISABLED
//Size of the IPv6 multicast filter
#define IPV6_MULTICAST_FILTER_SIZE 8

//IPv6 fragmentation support
#define IPV6_FRAG_SUPPORT DISABLED
//Maximum number of fragmented packets the host will accept
//and hold in the reassembly queue simultaneously
#define IPV6_MAX_FRAG_DATAGRAMS 4
//Maximum datagram size the host will accept when reassembling fragments
#define IPV6_MAX_FRAG_DATAGRAM_SIZE 8192

//MLD support
#define MLD_SUPPORT DISABLED

//Neighbor cache size
#define NDP_NEIGHBOR_CACHE_SIZE 8
//Destination cache size
#define NDP_DEST_CACHE_SIZE 8
//Maximum number of packets waiting for address resolution to complete
#define NDP_MAX_PENDING_PACKETS 2

//TCP support
#define TCP_SUPPORT ENABLED
//Default buffer size for transmission
#define TCP_DEFAULT_TX_BUFFER_SIZE (1430*2)
//Default buffer size for reception
#define TCP_DEFAULT_RX_BUFFER_SIZE (1430*2)

//Default SYN queue size for listening sockets
#define TCP_DEFAULT_SYN_QUEUE_SIZE    1
#define TCP_MAX_SYN_QUEUE_SIZE        1

//Maximum number of retransmissions
#define TCP_MAX_RETRIES               5
#define TCP_MAX_RTO                   5000

//Selective acknowledgment support
#define TCP_SACK_SUPPORT DISABLED

//UDP support
#define UDP_SUPPORT ENABLED
//Receive queue depth for connectionless sockets
#define UDP_RX_QUEUE_SIZE 4

//Raw socket support
#define RAW_SOCKET_SUPPORT DISABLED
//Receive queue depth for raw sockets
#define RAW_SOCKET_RX_QUEUE_SIZE      4

//Number of sockets that can be opened simultaneously
#define SOCKET_MAX_COUNT              6

//Use fixed-size blocks allocation?
#define NET_MEM_POOL_SUPPORT ENABLED

//Number of buffers available
#define NET_MEM_POOL_BUFFER_COUNT     10

//Size of the buffers
#define NET_MEM_POOL_BUFFER_SIZE      1536

#define NET_TASK_PRIORITY             150

#define NET_TASK_STACK_SIZE           256

//Number of TX buffers
#define STM32F107_ETH_TX_BUFFER_COUNT 2
//Number of RX buffers
#define STM32F107_ETH_RX_BUFFER_COUNT 2

#endif
