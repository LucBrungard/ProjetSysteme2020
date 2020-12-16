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

struct _server_
{
    int _fdCommunicationSocket;
    int _fdWaitSocket;
    struct sockaddr_in _callerCoords;
    socklen_t _coordsSize;
    void (*_onConnect)(Client);
    void (*_onDisconnect)(Client);
    size_t (*_onMessage)(Client, void *, size_t, void *);
};

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
    char user[65];
    {
        int readBytes = recv(con->fdSocket, user, 64, 0);
        if (readBytes > 0)
        {
            user[readBytes] = '\0';
            client->_username = user;
        }
        else
        {
            close(con->fdSocket);
            return NULL;
        }
    }
    if (con->server->_onConnect != NULL)
        (*con->server->_onConnect)(client);
    while (true)
    {
        void *buffer = malloc(32000);
        ssize_t readBytes = recv(con->fdSocket, buffer, 32000, 0);
        if (readBytes >= 0)
        {
            void *response = malloc(32000);
            size_t respSize = 0;
            if (con->server->_onMessage != NULL)
                respSize = (*con->server->_onMessage)(client, buffer, readBytes, response);
            send(con->fdSocket, response, respSize, 0);
            free(buffer);
            free(response);
        }
        else
        {
            free(buffer);
            break;
        }
    }
    close(con->fdSocket);
    if (con->server->_onDisconnect != NULL)
        (*con->server->_onDisconnect)(client);
}
void Server_destroy(Server server)
{
    free(server);
}
Server Server_create(uint16_t port)
{
    Server server = (Server)malloc(sizeof(struct _server_));
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
        free(server);
        return NULL;
    }
    if (listen(server->_fdWaitSocket, 5) == -1)
    {
        printf("listen error\n");
        free(server);
        return NULL;
    }
    server->_coordsSize = sizeof(server->_callerCoords);
    return server;
}
bool Server_run(Server server)
{
    while (true)
    {
        if ((server->_fdCommunicationSocket = accept(server->_fdCommunicationSocket, (struct sockaddr *)&server->_callerCoords, &server->_coordsSize)) == -1)
        {
            printf("accept error\n");
            return false;
        }
        else
        {
            connexion con;
            con.callerCoords = server->_callerCoords;
            con.fdSocket = server->_fdCommunicationSocket;

            pthread_t my_thread1;
            pthread_create(&my_thread1, NULL, clientThread, (void *)&con);
        }
    }

    close(server->_fdWaitSocket);
    return false;
}
void Server_onConnectedClient(Server server, void (*fct)(Client))
{
    server->_onConnect = fct;
}
void Server_onDisconnectedClient(Server server, void (*fct)(Client))
{
    server->_onDisconnect = fct;
}
void Server_onMessageRecieved(Server server, size_t (*fct)(Client, void *, size_t, void *))
{
    server->_onMessage = fct;
}
