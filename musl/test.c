#include <assert.h>
#include <stdio.h>

#include <net/ethernet.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <net/route.h>

#include <netinet/ether.h>
#include <netinet/icmp6.h>
#include <netinet/if_ether.h>
#include <netinet/igmp.h>
#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <netinet/ip6.h>
#include <netinet/ip_icmp.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>

int main() {
  printf("ether_addr: %zu\n", sizeof(struct ether_addr));
  printf("ether_header: %zu\n", sizeof(struct ether_header));
  printf("arphdr: %zu\n", sizeof(struct arphdr));
  printf("arpreq: %zu\n", sizeof(struct arpreq));
  printf("arpreq_old: %zu\n", sizeof(struct arpreq_old));
  //   printf("arpd_request: %zu\n", sizeof(struct arpd_request));

  printf("icmp6_filter: %zu\n", sizeof(struct icmp6_filter));
  printf("icmp6_hdr: %zu\n", sizeof(struct icmp6_hdr));
  printf("nd_router_solicit: %zu\n", sizeof(struct nd_router_solicit));
  printf("nd_router_advert: %zu\n", sizeof(struct nd_router_advert));
  printf("nd_neighbor_advert: %zu\n", sizeof(struct nd_neighbor_advert));
  printf("nd_redirect: %zu\n", sizeof(struct nd_redirect));
  printf("nd_opt_hdr: %zu\n", sizeof(struct nd_opt_hdr));
  printf("nd_opt_prefix_info: %zu\n", sizeof(struct nd_opt_prefix_info));
  printf("nd_opt_rd_hdr: %zu\n", sizeof(struct nd_opt_rd_hdr));
  printf("nd_opt_mtu: %zu\n", sizeof(struct nd_opt_mtu));
  printf("mld_hdr: %zu\n", sizeof(struct mld_hdr));
  printf("rr_pco_match: %zu\n", sizeof(struct rr_pco_match));
  printf("rr_pco_use: %zu\n", sizeof(struct rr_pco_use));
  printf("rr_result: %zu\n", sizeof(struct rr_result));
  printf("nd_opt_adv_interval: %zu\n", sizeof(struct nd_opt_adv_interval));
  printf("nd_opt_home_agent_info: %zu\n",
         sizeof(struct nd_opt_home_agent_info));
  printf("ethhdr: %zu\n", sizeof(struct ethhdr));
  printf("ether_arp: %zu\n", sizeof(struct ether_arp));
  printf("igmp: %zu\n", sizeof(struct igmp));
  printf("in_addr: %zu\n", sizeof(struct in_addr));
  printf("sockaddr_in: %zu\n", sizeof(struct sockaddr_in));
  printf("in6_addr: %zu\n", sizeof(struct in6_addr));
  printf("sockaddr_in6: %zu\n", sizeof(struct sockaddr_in6));
  printf("ipv6_mreq: %zu\n", sizeof(struct ipv6_mreq));
  printf("ip_opts: %zu\n", sizeof(struct ip_opts));
  printf("ip_mreq: %zu\n", sizeof(struct ip_mreq));
  printf("ip_mreqn: %zu\n", sizeof(struct ip_mreqn));
  printf("ip_mreq_source: %zu\n", sizeof(struct ip_mreq_source));
  printf("ip_msfilter: %zu\n", sizeof(struct ip_msfilter));
  //   printf("group_req: %zu\n", sizeof(struct group_req));
  //   printf("group_source_req: %zu\n", sizeof(struct group_source_req));
  //   printf("group_filter: %zu\n", sizeof(struct group_filter));
  printf("in_pktinfo: %zu\n", sizeof(struct in_pktinfo));
  printf("icmphdr: %zu\n", sizeof(struct icmphdr));
  printf("icmp_ra_addr: %zu\n", sizeof(struct icmp_ra_addr));
  printf("icmp: %zu\n", sizeof(struct icmp));
  printf("timestamp: %zu\n", sizeof(struct timestamp));
  printf("iphdr: %zu\n", sizeof(struct iphdr));
  printf("ip: %zu\n", sizeof(struct ip));
  printf("ip_timestamp: %zu\n", sizeof(struct ip_timestamp));
  printf("ip6_hdr: %zu\n", sizeof(struct ip6_hdr));
  printf("ip6_ext: %zu\n", sizeof(struct ip6_ext));
  printf("ip6_hbh: %zu\n", sizeof(struct ip6_hbh));
  printf("ip6_dest: %zu\n", sizeof(struct ip6_dest));
  printf("ip6_rthdr: %zu\n", sizeof(struct ip6_rthdr));
  printf("ip6_rthdr0: %zu\n", sizeof(struct ip6_rthdr0));
  printf("ip6_frag: %zu\n", sizeof(struct ip6_frag));
  printf("ip6_opt: %zu\n", sizeof(struct ip6_opt));
  printf("ip6_opt_jumbo: %zu\n", sizeof(struct ip6_opt_jumbo));
  printf("ip6_opt_nsap: %zu\n", sizeof(struct ip6_opt_nsap));
  printf("ip6_opt_tunnel: %zu\n", sizeof(struct ip6_opt_tunnel));
  printf("ip6_opt_router: %zu\n", sizeof(struct ip6_opt_router));
  printf("tcphdr: %zu\n", sizeof(struct tcphdr));
  printf("udphdr: %zu\n", sizeof(struct udphdr));
  printf("icmphdr: %zu\n", sizeof(struct icmphdr));

  assert(sizeof(struct ether_addr) == 6);
  assert(sizeof(struct ether_header) == 14);
  assert(sizeof(struct arphdr) == 8);
  assert(sizeof(struct arpreq) == 68);
  assert(sizeof(struct arpreq_old) == 52);
  //   assert(sizeof(struct arpd_request) == 40);
  assert(sizeof(struct icmp6_filter) == 32);
  assert(sizeof(struct icmp6_hdr) == 8);
  assert(sizeof(struct nd_router_solicit) == 8);
  assert(sizeof(struct nd_router_advert) == 16);
  assert(sizeof(struct nd_neighbor_advert) == 24);
  assert(sizeof(struct nd_redirect) == 40);
  assert(sizeof(struct nd_opt_hdr) == 2);
  assert(sizeof(struct nd_opt_prefix_info) == 32);
  assert(sizeof(struct nd_opt_rd_hdr) == 8);
  assert(sizeof(struct nd_opt_mtu) == 8);
  assert(sizeof(struct mld_hdr) == 24);
  assert(sizeof(struct rr_pco_match) == 24);
  assert(sizeof(struct rr_pco_use) == 32);
  assert(sizeof(struct rr_result) == 24);
  assert(sizeof(struct nd_opt_adv_interval) == 8);
  assert(sizeof(struct nd_opt_home_agent_info) == 8);
  assert(sizeof(struct ethhdr) == 14);
  assert(sizeof(struct ether_arp) == 28);
  assert(sizeof(struct igmp) == 8);
  assert(sizeof(struct in_addr) == 4);
  assert(sizeof(struct sockaddr_in) == 16);
  assert(sizeof(struct in6_addr) == 16);
  assert(sizeof(struct sockaddr_in6) == 28);
  assert(sizeof(struct ipv6_mreq) == 20);
  assert(sizeof(struct ip_opts) == 44);
  assert(sizeof(struct ip_mreq) == 8);
  assert(sizeof(struct ip_mreqn) == 12);
  assert(sizeof(struct ip_mreq_source) == 12);
  assert(sizeof(struct ip_msfilter) == 20);
  //   assert(sizeof(struct group_req) == 136);
  //   assert(sizeof(struct group_source_req) == 264);
  //   assert(sizeof(struct group_filter) == 272);
  assert(sizeof(struct in_pktinfo) == 12);
  assert(sizeof(struct icmphdr) == 8);
  assert(sizeof(struct icmp_ra_addr) == 8);
  assert(sizeof(struct icmp) == 28);
  assert(sizeof(struct timestamp) == 40);
  assert(sizeof(struct iphdr) == 20);
  assert(sizeof(struct ip) == 20);
  assert(sizeof(struct ip_timestamp) == 40);
  assert(sizeof(struct ip6_hdr) == 40);
  assert(sizeof(struct ip6_ext) == 2);
  assert(sizeof(struct ip6_hbh) == 2);
  assert(sizeof(struct ip6_dest) == 2);
  assert(sizeof(struct ip6_rthdr) == 4);
  assert(sizeof(struct ip6_rthdr0) == 8);
  assert(sizeof(struct ip6_frag) == 8);
  assert(sizeof(struct ip6_opt) == 2);
  assert(sizeof(struct ip6_opt_jumbo) == 6);
  assert(sizeof(struct ip6_opt_nsap) == 4);
  assert(sizeof(struct ip6_opt_tunnel) == 3);
  assert(sizeof(struct ip6_opt_router) == 4);
  assert(sizeof(struct tcphdr) == 20);
  assert(sizeof(struct udphdr) == 8);
  assert(sizeof(struct icmphdr) == 8);
}