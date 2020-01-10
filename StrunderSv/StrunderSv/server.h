#pragma once

#include "util.h"
#include "server_shared.h"
#include "sockets.h"
#include "packet_handler.h"
#include "log.h"




static SOCKET create_listen_socket();
static void NTAPI connection_thread(void* connection_socket);
void NTAPI server_thread(void*);