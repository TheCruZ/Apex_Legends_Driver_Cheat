#pragma once

//#include "util.h"
#include <ntddk.h>
#include "server_shared.h"
#include "sockets.h"
#include "eimport.h"
#include "JUNK.h"
#include "log.h"
#include "util.h"

static UINT64 handle_copy_memory(const PacketCopyMemory& packet);
static UINT64 handle_get_base_address(const PacketGetBaseAddress& packet);
static UINT64 handle_get_pid(const PacketGetPid& packet);
UINT64 handle_incoming_packet(const Packet& packet);
bool complete_request(const SOCKET client_connection, const UINT64 result);