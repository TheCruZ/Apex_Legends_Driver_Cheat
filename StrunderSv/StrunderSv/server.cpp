#include "server.h"

int tc = 0;



// Connection handling thread.
static void NTAPI connection_thread(void* connection_socket)
{
	const auto client_connection = SOCKET(ULONG_PTR(connection_socket));
	log("New connection.");
	J();
	Packet packet{ };
	while (GetGlobalActive())
	{
		const auto result = recv(client_connection, (void*)&packet, sizeof(packet), 0);
		if (result <= 0)
			break;

		if (result < sizeof(PacketHeader))
			continue;

		if (!GetGlobalActive())
			break;


//#ifdef DEBUG
//		
//		DbgPrintEx(0, 0, "int size %d type size %d\n",sizeof(unsigned int),sizeof(packet.header.type));
//
//		DbgPrintEx(0, 0, "Full packet\n");
//		unsigned char* buff = (unsigned char*)&packet;
//		for (size_t i = 0; i < sizeof(packet); i++) {
//			DbgPrintEx(0, 0, "%hhx", buff[i]);
//		}
//#endif // DEBUG

		//if (packet.header.magic != packet_magic)
		//	continue;

		const auto packet_result = handle_incoming_packet(packet);
		if (!complete_request(client_connection, packet_result))
			break;
	}
	J();
	log("Connection closed.");
	closesocket(client_connection);
	tc--;
}

// Main server thread.
void NTAPI server_thread(void*)
{
	J();
	auto status = KsInitialize();
	J();
	if (!NT_SUCCESS(status))
	{
		log("Failed to initialize KSOCKET. Status code: %X.", status);
		return;
	}

	const auto listen_socket = create_listen_socket();
	if (listen_socket == INVALID_SOCKET)
	{
		log("Failed to initialize listening socket.");

		KsDestroy();
		return;
	}
	J();
	log("Listening on port %d.", server_port);

	while (GetGlobalActive())
	{
		sockaddr  socket_addr{ };
		socklen_t socket_length{ };

		while (tc > 10) {
			Sleep(1000);
		}
		if (!GetGlobalActive()) {
			break;
		}
		const auto client_connection = accept(listen_socket, &socket_addr, &socket_length);
		if (client_connection == INVALID_SOCKET)
		{
			log("Failed to accept client connection.");
			break;
		}

		HANDLE thread_handle = nullptr;
		J();
		// Create a thread that will handle connection with client.
		// TODO: Limit number of threads.
		status = StartThread(connection_thread,(void*)client_connection);

		if (!NT_SUCCESS(status))
		{
			log("Failed to create thread for handling client connection.");
			closesocket(client_connection);
			break;
		}
		tc++;
		J();
		ZwClose(thread_handle);
	}
	J();
	closesocket(listen_socket);

	SetGlobalActive(FALSE);

	while (tc > 0) {
		log("Waiting for %d active connection threads\n", tc);
		Sleep(1000);
	}
	KsDestroy();
}
static SOCKET create_listen_socket()
{
	SOCKADDR_IN address{ };

	address.sin_family = AF_INET;
	address.sin_port = htons(server_port);

	const auto listen_socket = socket_listen(AF_INET, SOCK_STREAM, 0);
	if (listen_socket == INVALID_SOCKET)
	{
		log("Failed to create listen socket.");
		return INVALID_SOCKET;
	}

	if (bind(listen_socket, (SOCKADDR*)&address, sizeof(address)) == SOCKET_ERROR)
	{
		log("Failed to bind socket.");

		closesocket(listen_socket);
		return INVALID_SOCKET;
	}

	//if (listen(listen_socket, 10) == SOCKET_ERROR)
	//{
	//	log("Failed to set socket mode to listening.");

	//	closesocket(listen_socket);
	//	return INVALID_SOCKET;
	//}

	return listen_socket;
}