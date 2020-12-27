set (LWIP_SOURCES
	src/api/api_lib.c
    src/api/api_msg.c
    src/api/err.c
    src/api/netbuf.c
    src/api/netdb.c
    src/api/netifapi.c
    src/api/tcpip.c
    src/api/sockets.c
    src/netif/ethernet.c
    src/core/init.c
    src/core/tcp_out.c
    src/core/tcp.c
    src/core/raw.c
    src/core/sys.c
    src/core/netif.c
    src/core/timeouts.c
    src/core/mem.c
    src/core/tcp_in.c
    src/core/ip.c
    src/core/udp.c
    src/core/stats.c
    src/core/ipv4/ip4_addr.c
    src/core/ipv4/dhcp.c
    src/core/ipv4/ip4_frag.c
    src/core/ipv4/icmp.c
    src/core/ipv4/ip4.c
    src/core/ipv4/igmp.c
    src/core/ipv4/autoip.c
    src/core/ipv4/etharp.c
    src/core/ipv6/inet6.c
    src/core/ipv6/ip6_addr.c
    src/core/ipv6/ethip6.c
    src/core/ipv6/mld6.c
    src/core/ipv6/ip6_frag.c
    src/core/ipv6/nd6.c
    src/core/ipv6/dhcp6.c
    src/core/ipv6/ip6.c
    src/core/ipv6/icmp6.c
    src/core/def.c
    src/core/memp.c
    src/core/pbuf.c
    src/core/inet_chksum.c
    src/core/dns.c
    system/OS/sys_arch.c
	)
	
add_library(lwip STATIC ${LWIP_SOURCES})

target_link_libraries(lwip cmsis)

target_include_directories(lwip
	SYSTEM
	PUBLIC
	${CMAKE_SOURCE_DIR}/lib/LwIP/src/include
	${CMAKE_SOURCE_DIR}/lib/LwIP/system
	${PROJECT_CONFIG_DIR}
	)