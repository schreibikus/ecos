//==========================================================================
//
//      lib/net_support.c
//
//      Misc network support functions
//
//==========================================================================
//####COPYRIGHTBEGIN####
//                                                                          
// -------------------------------------------                              
// The contents of this file are subject to the Red Hat eCos Public License 
// Version 1.1 (the "License"); you may not use this file except in         
// compliance with the License.  You may obtain a copy of the License at    
// http://www.redhat.com/                                                   
//                                                                          
// Software distributed under the License is distributed on an "AS IS"      
// basis, WITHOUT WARRANTY OF ANY KIND, either express or implied.  See the 
// License for the specific language governing rights and limitations under 
// the License.                                                             
//                                                                          
// The Original Code is eCos - Embedded Configurable Operating System,      
// released September 30, 1998.                                             
//                                                                          
// The Initial Developer of the Original Code is Red Hat.                   
// Portions created by Red Hat are                                          
// Copyright (C) 1998, 1999, 2000 Red Hat, Inc.                             
// All Rights Reserved.                                                     
// -------------------------------------------                              
//                                                                          
//####COPYRIGHTEND####
//####BSDCOPYRIGHTBEGIN####
//
// -------------------------------------------
//
// Portions of this software may have been derived from OpenBSD or other sources,
// and are covered by the appropriate copyright disclaimers included herein.
//
// -------------------------------------------
//
//####BSDCOPYRIGHTEND####
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    gthomas
// Contributors: gthomas, sorin@netappi.com ("Sorin Babeanu"), hmt
// Date:         2000-01-10
// Purpose:      
// Description:  
//              
//
//####DESCRIPTIONEND####
//
//==========================================================================

// BOOTP support

#include <pkgconf/net.h>
#undef _KERNEL
#include <sys/param.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/errno.h>

#include <net/if.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <net/route.h>

#include <cyg/infra/diag.h>
#include <cyg/kernel/kapi.h>

#include <stdio.h>    // for 'sprintf()'

#include <bootp.h>
#include <network.h>
#include <arpa/inet.h>

#include <eth_drv.h>

#ifdef CYGPKG_NET_DHCP
#include <dhcp.h>
#endif

#ifdef CYGHWR_NET_DRIVER_ETH0
struct bootp eth0_bootp_data;
cyg_bool_t   eth0_up = false;
const char  *eth0_name = "eth0";
#endif
#ifdef CYGHWR_NET_DRIVER_ETH1
struct bootp eth1_bootp_data;
cyg_bool_t   eth1_up = false;
const char  *eth1_name = "eth1";
#endif

#define _string(s) #s
#define string(s) _string(s)

#ifndef CYGPKG_LIBC_STDIO
#define perror(s) diag_printf(#s ": %s\n", strerror(errno))
#endif

#ifdef CYGPKG_NET_NLOOP
#if 0 < CYGPKG_NET_NLOOP
//  
//   Initialize loopback interface  ----------   Added by sorin@netappi.com 
//
cyg_bool_t init_loopback_interface(int lo)
{
    struct sockaddr_in *addrp;
    struct ifreq ifr;
    int s;
    int one = 1;
    struct ecos_rtentry route;
    struct in_addr netmask, gateway;

    //diag_printf("Init_loopback_interface\n");
    s = socket(AF_INET, SOCK_DGRAM, 0); 
    if (s < 0) {
        perror("socket");
        return false;
    }
    if (setsockopt(s, SOL_SOCKET, SO_BROADCAST, &one, sizeof(one))) {
        perror("setsockopt");
        return false;
    }

    addrp = (struct sockaddr_in *) &ifr.ifr_addr;
    memset(addrp, 0, sizeof(*addrp));
    addrp->sin_family = AF_INET;
    addrp->sin_len = sizeof(*addrp);
    addrp->sin_port = 0;
    // Make an address 127.0.<lo>.1 to manage multiple loopback ifs.
    // (There is normally only 1, so it's the standard 127.0.0.1)
    addrp->sin_addr.s_addr = htonl((0x100 * lo) + INADDR_LOOPBACK) ; 

#if CYGPKG_NET_NLOOP > 1
    // Init the one we were told to
    sprintf(ifr.ifr_name, "lo%d", lo);
#else
    strcpy(ifr.ifr_name, "lo0");
#endif    

    if (ioctl(s, SIOCSIFADDR, &ifr)) {
        perror("SIOCIFADDR");
        return false;
    }
    
#if 1 < CYGPKG_NET_NLOOP
    // We cheat to make different nets for multiple loopback devs
    addrp->sin_addr.s_addr = netmask.s_addr = htonl(IN_CLASSC_NET);
#else
    // 
    addrp->sin_addr.s_addr = netmask.s_addr = htonl(IN_CLASSA_NET);
#endif
    if (ioctl(s, SIOCSIFNETMASK, &ifr)) {
        perror("SIOCSIFNETMASK");
        return false;
    }
    ifr.ifr_flags = IFF_UP | IFF_BROADCAST | IFF_RUNNING;
    if (ioctl(s, SIOCSIFFLAGS, &ifr)) {
        perror("SIOCSIFFLAGS");
        return false;
    }

    gateway.s_addr = htonl(INADDR_LOOPBACK);  
    memset(&route, 0, sizeof(route));
    addrp->sin_family = AF_INET;
    addrp->sin_port = 0;
    addrp->sin_addr.s_addr = htonl((0x100 * lo) + INADDR_LOOPBACK) & netmask.s_addr;
    memcpy(&route.rt_dst, addrp, sizeof(*addrp));
    addrp->sin_addr = netmask;
    memcpy(&route.rt_genmask, addrp, sizeof(*addrp));
    addrp->sin_addr = gateway;
    memcpy(&route.rt_gateway, addrp, sizeof(*addrp));
    
    route.rt_dev = ifr.ifr_name;
    route.rt_flags = RTF_UP|RTF_GATEWAY;
    route.rt_metric = 0;

    if (ioctl(s, SIOCADDRT, &route)) {
        diag_printf("Route - dst: %s", inet_ntoa(((struct sockaddr_in *)&route.rt_dst)->sin_addr));
        diag_printf(", mask: %s", inet_ntoa(((struct sockaddr_in *)&route.rt_genmask)->sin_addr));
        diag_printf(", gateway: %s\n", inet_ntoa(((struct sockaddr_in *)&route.rt_gateway)->sin_addr));
        if (errno != EEXIST) {
            perror("SIOCADDRT 3");
            return false;
        }
    }
    close(s);
    return true;
}
#endif
#endif


//
// Internal function which builds up a fake BOOTP database for
// an interface.
//

static unsigned char *
add_tag(unsigned char *vp,
        unsigned char tag,
        void *val,
        int len)
{
    int i;
    unsigned char *xp = (unsigned char *)val;
    *vp++ = tag;
    *vp++ = len;
    for (i = 0;  i < len;  i++) {
        *vp++ = *xp++;
    }
    return vp;
}

void
build_bootp_record(struct bootp *bp,
                   const char *addrs_ip,
                   const char *addrs_netmask,
                   const char *addrs_broadcast,
                   const char *addrs_gateway,
                   const char *addrs_server)
{
    int i;
    in_addr_t addr;
    unsigned char *vp;
    unsigned char cookie[] = VM_RFC1048;

    bzero(bp, sizeof(struct bootp));
    bp->bp_op = BOOTREPLY;
    bp->bp_htype = HTYPE_ETHERNET;
    bp->bp_hlen = 6;
    for (i = 0;  i < bp->bp_hlen;  i++) {
        bp->bp_chaddr[i] = 0xFF;
    }
    bp->bp_ciaddr.s_addr = inet_addr(addrs_ip);
    bp->bp_yiaddr.s_addr = inet_addr(addrs_ip);
    bp->bp_siaddr.s_addr = inet_addr(addrs_server);
    bp->bp_giaddr.s_addr = inet_addr(addrs_gateway);
    vp = &bp->bp_vend[0];
    bcopy(&cookie, vp, sizeof(cookie));
    vp += sizeof(cookie);
    addr = inet_addr(addrs_netmask);
    vp = add_tag(vp, TAG_SUBNET_MASK, &addr, sizeof(in_addr_t));
    addr = inet_addr(addrs_broadcast);
    vp = add_tag(vp, TAG_IP_BROADCAST, &addr, sizeof(in_addr_t));
    addr = inet_addr(addrs_gateway);
    vp = add_tag(vp, TAG_GATEWAY, &addr, sizeof(in_addr_t));
    *vp = TAG_END;
}


//
// Initialize network interface[s] using BOOTP/DHCP
//
void
init_all_network_interfaces(void)
{
    static volatile int in_init_all_network_interfaces = 0;
#ifdef CYGPKG_IO_PCMCIA
    cyg_netdevtab_entry_t *t;
#endif // CYGPKG_IO_PCMCIA

    cyg_scheduler_lock();
    while ( in_init_all_network_interfaces ) {
        // Another thread is doing this...
        cyg_scheduler_unlock();
        cyg_thread_delay( 10 );
        cyg_scheduler_lock();
    }
    in_init_all_network_interfaces = 1;
    cyg_scheduler_unlock();

#ifdef CYGHWR_NET_DRIVER_ETH0
    if ( ! eth0_up ) { // Make this call idempotent
#ifdef CYGPKG_IO_PCMCIA
        if ((t = eth_drv_netdev("eth0")) != (cyg_netdevtab_entry_t *)NULL) {
            while (t->status != CYG_NETDEVTAB_STATUS_AVAIL) {
                diag_printf("... Waiting for PCMCIA device 'eth0'\n");
                cyg_thread_delay(100);
            }
        }
#endif // CYGPKG_IO_PCMCIA
#ifdef CYGHWR_NET_DRIVER_ETH0_BOOTP
        // Perform a complete initialization, using BOOTP/DHCP
        eth0_up = true;
#ifdef CYGHWR_NET_DRIVER_ETH0_DHCP
        eth0_dhcpstate = 0; // Says that initialization is external to dhcp
        if (do_dhcp(eth0_name, &eth0_bootp_data, &eth0_dhcpstate, &eth0_lease)) 
#else
#ifdef CYGPKG_NET_DHCP
        eth0_dhcpstate = DHCPSTATE_BOOTP_FALLBACK;
        // so the dhcp machine does no harm if called
#endif
        if (do_bootp(eth0_name, &eth0_bootp_data)) 
#endif
        {
#ifdef CYGHWR_NET_DRIVER_ETH0_BOOTP_SHOW
            show_bootp(eth0_name, &eth0_bootp_data);
#endif
        } else {
            diag_printf("BOOTP/DHCP failed on eth0\n");
            eth0_up = false;
        }
#elif defined(CYGHWR_NET_DRIVER_ETH0_ADDRS_IP)
        eth0_up = true;
        build_bootp_record(&eth0_bootp_data,
                       string(CYGHWR_NET_DRIVER_ETH0_ADDRS_IP),
                       string(CYGHWR_NET_DRIVER_ETH0_ADDRS_NETMASK),
                       string(CYGHWR_NET_DRIVER_ETH0_ADDRS_BROADCAST),
                       string(CYGHWR_NET_DRIVER_ETH0_ADDRS_GATEWAY),
                       string(CYGHWR_NET_DRIVER_ETH0_ADDRS_SERVER));
        show_bootp(eth0_name, &eth0_bootp_data);
#endif
    }
#endif // CYGHWR_NET_DRIVER_ETH0
#ifdef CYGHWR_NET_DRIVER_ETH1
    if ( ! eth1_up ) { // Make this call idempotent
#ifdef CYGPKG_IO_PCMCIA
        if ((t = eth_drv_netdev("eth1")) != (cyg_netdevtab_entry_t *)NULL) {
            while (t->status != CYG_NETDEVTAB_STATUS_AVAIL) {
                diag_printf("... Waiting for PCMCIA device 'eth1'\n");
                cyg_thread_delay(100);
            }
        }
#endif // CYGPKG_IO_PCMCIA
#ifdef CYGHWR_NET_DRIVER_ETH1_BOOTP
        // Perform a complete initialization, using BOOTP/DHCP
        eth1_up = true;
#ifdef CYGHWR_NET_DRIVER_ETH1_DHCP
        eth1_dhcpstate = 0; // Says that initialization is external to dhcp
        if (do_dhcp(eth1_name, &eth1_bootp_data, &eth1_dhcpstate, &eth1_lease)) 
#else
#ifdef CYGPKG_NET_DHCP
        eth1_dhcpstate = DHCPSTATE_BOOTP_FALLBACK;
        // so the dhcp machine does no harm if called
#endif
        if (do_bootp(eth1_name, &eth1_bootp_data))
#endif
        {
#ifdef CYGHWR_NET_DRIVER_ETH1_BOOTP_SHOW
            show_bootp(eth1_name, &eth1_bootp_data);
#endif
        } else {
            diag_printf("BOOTP/DHCP failed on eth1\n");
            eth1_up = false;
        }
#elif defined(CYGHWR_NET_DRIVER_ETH1_ADDRS_IP)
        eth1_up = true;
        build_bootp_record(&eth1_bootp_data,
                       string(CYGHWR_NET_DRIVER_ETH1_ADDRS_IP),
                       string(CYGHWR_NET_DRIVER_ETH1_ADDRS_NETMASK),
                       string(CYGHWR_NET_DRIVER_ETH1_ADDRS_BROADCAST),
                       string(CYGHWR_NET_DRIVER_ETH1_ADDRS_GATEWAY),
                       string(CYGHWR_NET_DRIVER_ETH1_ADDRS_SERVER));
        show_bootp(eth1_name, &eth1_bootp_data);
#endif
    }
#endif // CYGHWR_NET_DRIVER_ETH1
#ifdef CYGHWR_NET_DRIVER_ETH0
#ifndef CYGHWR_NET_DRIVER_ETH0_MANUAL
    if (eth0_up) {
        if (!init_net(eth0_name, &eth0_bootp_data)) {
            diag_printf("Network initialization failed for eth0\n");
            eth0_up = false;
        }
    }
#endif
#endif
#ifdef CYGHWR_NET_DRIVER_ETH1
#ifndef CYGHWR_NET_DRIVER_ETH1_MANUAL
    if (eth1_up) {
        if (!init_net(eth1_name, &eth1_bootp_data)) {
            diag_printf("Network initialization failed for eth1\n");
            eth1_up = false;
        }
    }
#endif
#endif

#ifdef CYGPKG_NET_NLOOP
#if 0 < CYGPKG_NET_NLOOP
    {
        static int loop_init = 0;
        int i;
        if ( 0 == loop_init++ )
            for ( i = 0; i < CYGPKG_NET_NLOOP; i++ )
                init_loopback_interface( i );
    }
#endif
#endif

#ifdef CYGOPT_NET_DHCP_DHCP_THREAD
    dhcp_start_dhcp_mgt_thread();
#endif

    // Open the monitor to other threads.
    in_init_all_network_interfaces = 0;

}

// EOF network_support.c
