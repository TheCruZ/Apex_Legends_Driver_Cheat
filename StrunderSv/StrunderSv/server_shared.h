#pragma once
//#include "util.h"
//#include <stdint.h>

//constexpr auto packet_magic = 0x12345568;
constexpr auto server_ip	= 0x7F000001; // 127.0.0.1
constexpr auto server_port  = 4662;

enum class PacketType
{
	packet_copy_memory,
	packet_get_base_address,
	packet_get_pid,
	packet_completed
};

struct PacketCopyMemory
{
	unsigned int dest_process_id;
	unsigned long long dest_address;

	unsigned int src_process_id;
	unsigned long long src_address;

	unsigned int size;
};

struct PacketGetBaseAddress
{
	unsigned int process_id;
};

struct PacketGetPid
{
	size_t len;
	wchar_t name[256];
};

struct PackedCompleted
{
	unsigned long long result;
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