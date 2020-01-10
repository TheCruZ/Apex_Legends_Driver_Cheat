#include "driver.h"

int driver::currentProcessId = 0;
// Link to winsock.
#pragma comment(lib, "Ws2_32")

#pragma warning(disable : 4267)

// Send request packet and wait for completion.
static bool send_packet(
	const SOCKET	connection,
	const Packet& packet,
	uint64_t& out_result)
{
	Packet completion_packet{ };

	if (send(connection, (const char*)&packet, sizeof(Packet), 0) == SOCKET_ERROR)
		return false;

	const auto result = recv(connection, (char*)&completion_packet, sizeof(Packet), 0);
	if (result < sizeof(PacketHeader) ||
		//completion_packet.header.magic != packet_magic ||
		completion_packet.header.type != PacketType::packet_completed)
		return false;

	out_result = completion_packet.data.completed.result;
	return true;
}
uint64_t driver::get_process_pid(SOCKET connection, wchar_t pname[], size_t size)
{
	J();
	Packet packet{ };

	//packet.header.magic = packet_magic;
	packet.header.type = PacketType::packet_get_pid;

	J();
	auto& data = packet.data.get_pid;
	memset(data.name, 0x00, 256);
	memcpy(data.name, pname, size*2);
	data.len = size;

	uint64_t result = 0;
	if (send_packet(connection, packet, result)) {
		
		memset(data.name, 0x00, 256);
		return result;
	}
	memset(data.name, 0x00, 256);

	return 0;
}
uint64_t driver::get_process_base_address(const SOCKET connection, const uint32_t process_id)
{
	J();
	Packet packet{ };

	//packet.header.magic = packet_magic;
	packet.header.type = PacketType::packet_get_base_address;

	J();
	auto& data = packet.data.get_base_address;
	data.process_id = process_id;

	uint64_t result = 0;
	if (send_packet(connection, packet, result))
		return result;

	return 0;
}
static uint32_t copy_memory(
	const SOCKET	connection,
	const uint32_t	src_process_id,
	const uintptr_t src_address,
	const uint32_t	dest_process_id,
	const uintptr_t	dest_address,
	const size_t	size)
{
	Packet packet{ };

	//packet.header.magic = packet_magic;
	packet.header.type = PacketType::packet_copy_memory;

	auto& data = packet.data.copy_memory;
	data.src_process_id = src_process_id;
	data.src_address = uint64_t(src_address);
	data.dest_process_id = dest_process_id;
	data.dest_address = uint64_t(dest_address);
	data.size = uint64_t(size);

	uint64_t result = 0;
	if (send_packet(connection, packet, result))
	{
		return uint32_t(result);
	}

	return 0;
}
void driver::initialize()
{
	J();
	WSADATA wsa_data;
	WSAStartup(MAKEWORD(2, 2), &wsa_data);
	currentProcessId = GetCurrentProcessId();
}



SOCKET driver::connect()
{
	J();
	SOCKADDR_IN address{ };

	address.sin_family = AF_INET;
	address.sin_addr.s_addr = htonl(server_ip);
	address.sin_port = htons(server_port);

	const auto connection = socket(AF_INET, SOCK_STREAM, 0);
	if (connection == INVALID_SOCKET)
		return INVALID_SOCKET;

	J();
	if (connect(connection, (SOCKADDR*)&address, sizeof(address)) == SOCKET_ERROR)
	{
		closesocket(connection);
		return INVALID_SOCKET;
	}

	return connection;
}

void driver::disconnect(const SOCKET connection)
{
	J();
	closesocket(connection);
}
void driver::deinitialize()
{
	J();
	WSACleanup();
}
uint32_t driver::read_memory(
	const SOCKET	connection,
	const uint32_t	process_id,
	const uintptr_t address,
	const uintptr_t buffer,
	const size_t	size)
{
	return copy_memory(connection, process_id, address, currentProcessId, buffer, size);
}

uint32_t driver::write_memory(
	const SOCKET	connection,
	const uint32_t	process_id,
	const uintptr_t address,
	const uintptr_t buffer,
	const size_t	size)
{
	return copy_memory(connection, currentProcessId, buffer, process_id, address, size);
}

