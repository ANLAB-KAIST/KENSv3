/**
 * @file   E_RoutingInfo.hpp
 * @Author leeopop (dlrmsghd@gmail.com)
 * @date   November, 2014
 * @brief  Header for E::RoutingInfo
 */

#ifndef E_ROUTINGINFO_HPP_
#define E_ROUTINGINFO_HPP_

#include <E/E_Common.hpp>

namespace E
{

/**
 * @brief This class provides routing information
 * to its child class.
 */
class RoutingInfo
{
private:
	struct mac_entry
	{
		uint8_t mac[6];
		int port;
	};

	struct ip_entry
	{
		uint8_t ip[4];
		int port;
	};

	struct arp_entry
	{
		uint8_t ip[4];
		uint8_t mac[6];
	};

	struct route_entry
	{
		uint8_t ip_mask[4];
		int prefix;
		int port;
	};

	std::vector<struct mac_entry> mac_vector;
	std::vector<struct ip_entry> ip_vector;
	std::vector<struct arp_entry> arp_vector;
	std::vector<struct route_entry> route_vector;

public:
	RoutingInfo();
	virtual ~RoutingInfo();
public:
	/**
	 * @param ip IP address.
	 * @param port Interface index to bind the given IP address.
	 * @note You cannot override this function.
	 */
	virtual void setIPAddr(const uint8_t* ip, int port) final;

	/**
	 * @param mac MAC address.
	 * @param port Interface index to bind the given MAC address.
	 * @note You cannot override this function.
	 */
	virtual void setMACAddr(const uint8_t* mac, int port) final;

	/**
	 * @brief Add (MAC,IP) entry to its ARP table.
	 * @param mac MAC address.
	 * @param ip IP address.
	 * @note You cannot override this function.
	 */
	virtual void setARPTable(const uint8_t* mac, const uint8_t* ipv4) final;

	/**
	 * @param mask IP address mask.
	 * @param prefix Prefix length for routing.
	 * @param port Interface index to bind the given routing entry.
	 * @note You cannot override this function.
	 */
	virtual void setRoutingTable(const uint8_t* mask, int prefix, int port) final;

	/**
	 * @param ip_buffer Address of the buffer to save the IP address.
	 * @param port Interface index to retrieve its IP address.
	 * @return Whether this request is successful or not.
	 * @note You cannot override this function.
	 */
	virtual bool getIPAddr(uint8_t* ip_buffer, int port) final;

	/**
	 * @param mac_buffer Address of the buffer to save the MAC address.
	 * @param port Interface index to retrieve its MAC address.
	 * @return Whether this request is successful or not.
	 * @note You cannot override this function.
	 */
	virtual bool getMACAddr(uint8_t* mac_buffer, int port) final;

	/**
	 * @param mac_buffer Address of the buffer to save the MAC address.
	 * @param ip_buffer IP address to find its corresponding MAC address.
	 * @return Whether this request is successful or not.
	 * @note You cannot override this function.
	 */
	virtual bool getARPTable(uint8_t* mac_buffer, const uint8_t* ipv4) final;

	/**
	 * @param ip_buffer IP address to find its destination.
	 * @return Interface index to this packet should go to.
	 * @note You cannot override this function.
	 */
	virtual int getRoutingTable(const uint8_t* ip_addr) final;
};

}


#endif /* E_ROUTINGINFO_HPP_ */
