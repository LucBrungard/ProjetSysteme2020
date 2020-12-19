#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "Client.h"

struct _client_
{
    int _fdSocket;
};

Client Client_createI(char *username, struct in_addr ip, uint16_t port)
{
    Client client = (Client)malloc(sizeof(struct _client_));
    struct sockaddr_in servCoords;
    client->_fdSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (client->_fdSocket < 0)
    {
        printf("incorrect socket\n");
        return NULL;
    }

    // On prépare les coordonnées du serveur
    memset(&servCoords, 0x00, sizeof(struct sockaddr_in));
    servCoords.sin_family = PF_INET;
    // adresse du serveur
    servCoords.sin_addr = ip;
    // toutes les interfaces locales disponibles
    servCoords.sin_port = htons(port);
    if (connect(client->_fdSocket, (struct sockaddr *)&servCoords, sizeof(servCoords)) == -1)
    {
        printf("Unable to connect\n");
        return NULL;
    }
    send(client->_fdSocket, username, strlen(username), 0);
    return client;
}
Client Client_createS(char *username, char *ip, uint16_t port)
{
    struct in_addr address;
    inet_aton(ip, &address);
    return Client_createI(username, address, port);
}
void Client_disconnect(Client client)
{
    close(client->_fdSocket);
}
void Client_destroy(Client client)
{
    free(client);
}
ssize_t Client_send(Client client, void *request, size_t reqSize, void *response)
{
    if (reqSize == 0)
    {
        printf("data size can not be 0");
        return 0;
    }
    send(client->_fdSocket, request, reqSize, 0);
    if (response != NULL)
    {
        ssize_t respSize = recv(client->_fdSocket, response, 4800, 0);
        if (respSize == -1)
            close(client->_fdSocket);
        return respSize;
    }
    else
        return 0;
}
