#include <stdio.h>
#include <string.h>
#include "Server.h"

void clientConnected(Client client)
{
    printf("%s@%s:Connected\n", Client_getUsername(client), Client_getIpS(client));
}
void clientDisconnected(Client client)
{
    printf("%s@%s:Disconnected\n", Client_getUsername(client), Client_getIpS(client));
}
size_t message(Client client, void *data, size_t dataSize, void *buffer)
{
    printf("%s@%s:%s\nResponse:", Client_getUsername(client), Client_getIpS(client), (char *)data);
    fgets(buffer, 32000, stdin);
    return strlen(buffer);
}

int main()
{
    Server server = Server_create(6000);
    if (!server)
        return 0;
    Server_onConnectedClient(server, &clientConnected);
    Server_onDisconnectedClient(server, &clientDisconnected);
    Server_onMessageRecieved(server, &message);
    printf("Server initialized\n");
    Server_run(server);
    return 0;
}