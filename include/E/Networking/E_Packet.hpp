/**
 * @file   E_Packet.hpp
 * @Author leeopop (dlrmsghd@gmail.com)
 * @date   November, 2014
 * @brief  Header for E::Packet
 */

#ifndef E_PACKET_HPP_
#define E_PACKET_HPP_


#include <E/E_Common.hpp>
#include <E/E_Module.hpp>


namespace E
{
class NetworkSystem;

/**
 * @brief This class abstracts a packet.
 * You cannot directly allocate/deallocate Packet.
 * Also you cannot directly access the internal buffer.
 * Use access functions.
 *
 * @see NetworkModule
 */
class Packet : public Module::Message
{
private:
	Packet(UUID uuid, size_t maxSize);
	~Packet();
	void* buffer;
	size_t bufferSize;
	size_t dataSize;

	UUID packetID;
public:
	/**
	 * @param offset Start write skipping first n bytes of the given buffer.
	 * @param data Data to be written in this packet.
	 * @param length Length of data to be written.
	 * @return Actual written bytes. If length + offset is
	 * larger than the internal buffer size, some data may be truncated.
	 */
	size_t writeData(size_t offset, const void* data, size_t length);

	/**
	 * @param offset Start read skipping first n bytes of the internal buffer.
	 * @param data Destination of the packet content.
	 * @param length Length of data to be retrieved.
	 * @return Actual retrieved bytes.
	 */
	size_t readData(size_t offset, void* data, size_t length);

	/**
	 * @brief Change the size of this Packet
	 * The size cannot be larger than the internal buffer.
	 * @param size New size.
	 * @return Actual changed size.
	 */
	size_t setSize(size_t size);

	/**
	 * @return Current packet size.
	 */
	size_t getSize();

	void clearContext();

	friend class NetworkSystem;
};

}


#endif /* E_PACKET_HPP_ */
