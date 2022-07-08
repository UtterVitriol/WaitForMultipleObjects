#pragma once

#pragma comment(lib, "Ws2_32.lib")

#define DEFAULT_PORT "23666"

SOCKET create_socket(void);
SOCKET listen_socket(SOCKET ListenSocket);