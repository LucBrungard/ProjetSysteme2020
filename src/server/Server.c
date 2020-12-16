#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <pthread.h>
#include <stddef.h>
#include "Server.h"

typedef struct _server
{
    int _fdCommunicationSocket;
    int _fdWaitSocket;
    struct sockaddr_in _callerCoords;
    socklen_t _coordsSize;
    void (*_onConnect)(Client);
    void (*_onDisconnect)(Client);
    void (*_onMessage)(Client, void *, int, void **, int *);
} * Server;

typedef struct
{
    struct sockaddr_in callerCoords;
    int fdSocket;
    Server server;
} connexion;

void *clientThread(void *arg)
{
    connexion *con = (connexion *)arg;
    Client client;
    client->_ip = con->callerCoords.sin_addr;
}

bool Server_init(Server server, int port)
{
    struct sockaddr_in serverCoords;
    server->_fdWaitSocket = socket(PF_INET, SOCK_STREAM, 0);
    if (server->_fdWaitSocket < 0)
    {
        printf("incorrect socket\n");
        return false;
    }
    // On prépare l’adresse d’attachement locale
    memset(&serverCoords, 0x00, sizeof(struct sockaddr_in));
    serverCoords.sin_family = PF_INET;
    // toutes les interfaces locales disponibles
    serverCoords.sin_addr.s_addr = htonl(INADDR_ANY);
    // toutes les interfaces locales disponibles
    serverCoords.sin_port = htons(port);
    if (bind(server->_fdWaitSocket, (struct sockaddr *)&serverCoords, sizeof(serverCoords)) == -1)
    {
        printf("bind error\n");
        return false;
    }
    if (listen(server->_fdWaitSocket, 5) == -1)
    {
        printf("listen error\n");
        return false;
    }
    server->_coordsSize = sizeof(server->_callerCoords);
    return true;
}
void Server_run(Server server)
{
    while (true)
    {
        printf("En attente d'une connexion :\n ");

        if ((server->_fdCommunicationSocket = accept(server->_fdCommunicationSocket, (struct sockaddr *)&server->_callerCoords, &server->_coordsSize)) == -1)
            printf("erreur de accept\n");
        else
        {
            connexion con;
            con.callerCoords = server->_callerCoords;
            con.fdSocket = server->_fdCommunicationSocket;

            pthread_t my_thread1;
            int ret1 = pthread_create(&my_thread1, NULL, clientThread, (void *)&con);
        }
    }

    close(server->_fdWaitSocket);
}
void Server_onConnectedClient(Server server, void (*fct)(Client))
{
    server->_onConnect = fct;
}
void Server_onDisconnectedClient(Server server, void (*fct)(Client))
{
    server->_onDisconnect = fct;
}
void Server_onMessageRecieved(Server server, void (*fct)(Client, void *, int, void **, int *))
{
    server->_onMessage = fct;
}
