#include "packet_handler.h"

UINT64 handle_incoming_packet(const Packet& packet)
{
	switch (packet.header.type)
	{
	case PacketType::packet_copy_memory:
		return handle_copy_memory(packet.data.copy_memory);

	case PacketType::packet_get_base_address:
		return handle_get_base_address(packet.data.get_base_address);

	case PacketType::packet_get_pid:
		return handle_get_pid(packet.data.get_pid);
	default:
		break;
	}

	return UINT64(STATUS_NOT_IMPLEMENTED);
}

// Send completion packet.
bool complete_request(const SOCKET client_connection, const UINT64 result)
{
	Packet packet{ };

	//packet.header.magic				= packet_magic;
	packet.header.type = PacketType::packet_completed;
	packet.data.completed.result = result;

	return send(client_connection, &packet, sizeof(packet), 0) != SOCKET_ERROR;
}

static UINT64 handle_copy_memory(const PacketCopyMemory& packet)
{
	PEPROCESS dest_process = nullptr;
	PEPROCESS src_process  = nullptr;

//#ifdef DEBUG
//	DbgPrintEx(0, 0, "PacketCopyMemory packet\n");
//	unsigned char* buff = (unsigned char*)&packet;
//	for (size_t i = 0; i < sizeof(PacketCopyMemory); i++) {
//		DbgPrintEx(0, 0, "%hhx", buff[i]);
//	}
//#endif // DEBUG

	
	
	if (packet.src_process_id == 0) { //ignore pid not set
		return UINT64(STATUS_INVALID_CID);
	}
	log("CPMem received from %d at %#010x to %d at %#010x", packet.src_process_id, packet.src_address, packet.dest_process_id, packet.dest_address);
	if (!NT_SUCCESS(PsLookupProcessByProcessId(HANDLE(packet.dest_process_id), &dest_process)))
	{
		J();
		return UINT64(STATUS_INVALID_CID);
	}

	if (!NT_SUCCESS(PsLookupProcessByProcessId(HANDLE(packet.src_process_id), &src_process)))
	{
		J();
		ObDereferenceObject(dest_process);
		return UINT64(STATUS_INVALID_CID);
	}
	J();
	SIZE_T   return_size = 0;
	NTSTATUS status = MmCopyVirtualMemory(
		src_process,
		(void*)packet.src_address,
		dest_process,
		(void*)packet.dest_address,
		packet.size,
		UserMode,
		&return_size
	);
	ObDereferenceObject(dest_process);
	ObDereferenceObject(src_process);

	return UINT64(status);
}

static UINT64 handle_get_base_address(const PacketGetBaseAddress& packet)
{
	PEPROCESS process = nullptr;
	
//#ifdef DEBUG
//	DbgPrintEx(0, 0, "PacketGetBaseAddress packet pid: %d\n",packet.process_id);
//	unsigned char* buff = (unsigned char*)&packet;
//	for (size_t i = 0; i < sizeof(PacketGetBaseAddress); i++) {
//		DbgPrintEx(0, 0, "%hhx", buff[i]);
//	}
//#endif // DEBUG
	
	NTSTATUS  status  = PsLookupProcessByProcessId(HANDLE(packet.process_id), &process);

	if (!NT_SUCCESS(status))
		return 0;
	J();
	UINT64 base_address = (UINT64)PsGetProcessSectionBaseAddress(process);

	log("Base Address: %#010x", base_address);

	ObDereferenceObject(process);

	return base_address;
}

static UINT64 handle_get_pid(const PacketGetPid& packet)
{
	
	HANDLE pid = FindProcessByName((wchar_t*)packet.name, packet.len);

	RtlSecureZeroMemory((PVOID)&packet.name[0], 200);

	return UINT64(pid);
}


