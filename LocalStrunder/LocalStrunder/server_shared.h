#pragma once
#include <cstdint>

constexpr auto server_ip = 0x7F000001; // 127.0.0.1
constexpr auto server_port = 4662;

enum class PacketType
{
	packet_copy_memory,
	packet_get_base_address,
	packet_get_pid,
	packet_completed
};

struct PacketCopyMemory
{
	uint32_t dest_process_id;
	uint64_t dest_address;

	uint32_t src_process_id;
	uint64_t src_address;

	uint32_t size;
};

struct PacketGetBaseAddress
{
	uint32_t process_id;
};

struct PacketGetPid
{
	size_t len;
	wchar_t name[256];
};

struct PackedCompleted
{
	uint64_t result;
};

struct PacketHeader
{
	//uint32_t   magic;
	PacketType type;
};

struct Packet
{
	PacketHeader header;
	union
	{
		PacketCopyMemory	 copy_memory;
		PacketGetBaseAddress get_base_address;
		PacketGetPid get_pid;
		PackedCompleted		 completed;
	} data;
};