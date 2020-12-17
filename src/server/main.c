#include <stdio.h>
#include <string.h>
#include "Server.h"
#include "../consoleManagement.h"

void clientConnected(Client client)
{
    console_formatSystemForegroundMode("%s@%s", CONSOLE_COLOR_BRIGHT_BLUE, CONSOLE_FLAG_BOLD, Client_getUsername(client), Client_getIpS(client));
    printf(":");
    console_formatSystemForeground("Connected\n", CONSOLE_COLOR_BRIGHT_GREEN);
    printf("\n");
}
void clientDisconnected(Client client)
{
    console_formatSystemForegroundMode("%s@%s", CONSOLE_COLOR_BRIGHT_BLUE, CONSOLE_FLAG_BOLD, Client_getUsername(client), Client_getIpS(client));
    printf(":");
    console_formatSystemForeground("Disconnected\n", CONSOLE_COLOR_BRIGHT_RED);
    printf("\n");
}
size_t message(Client client, void *data, size_t dataSize, void *buffer)
{
    //TODO analyze client request
    return 1;
}

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        console_formatSystemForeground("Usage : server <port>", CONSOLE_COLOR_BRIGHT_RED);
        return 1;
    }
    uint16_t port = atoi(argv[1]);
    Server server = Server_create(port);
    if (!server)
        return 0;
    Server_onConnectedClient(server, &clientConnected);
    Server_onDisconnectedClient(server, &clientDisconnected);
    Server_onMessageRecieved(server, &message);
    console_formatSystemColor("Server initialized to port %d", CONSOLE_COLOR_BRIGHT_GREEN, port);
    printf("\n");
    Server_run(server);
    return 0;
}