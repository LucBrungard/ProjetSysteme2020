#ifndef DEF_SERVER_SERVER_H
#define DEF_SERVER_SERVER_H
#include <stdbool.h>
#include <netinet/in.h>

#include "Client.h"

typedef struct
{
    int _fdCommunicationSocket;
    int _fdWaitSocket;
    struct sockaddr_in _callerCoords;
    socklen_t _coordsSize;
    void (*_onConnect)(Client);
    void (*_onDisconnect)(Client);
    void (*_onMessage)(Client, void *, int, void **, int *);
} * Server;
/**
 * @brief  Initialize a new instance of a server
 * @param Server server to initiate
 * @param int port to use for the server
 * @return true if the server initialized correctly
 */
bool Server_init(Server, int);
/**
 * @brief  Event triggered when a client is connected to the server
 * @param Server server to use
 * @param  void(*)(Client) function to call when the event is triggered 
 * @return void
 */
void Server_onConnectedClient(Server, void (*)(Client));
/**
 * @brief  Event triggered when a client is disconnected to the server
 * @param Server server to use
 * @param  void(*)(Client) function to call when the event is triggered. Note : the Client instance is deleted after this call.
 * @return void
 */
void Server_onDisconnectedClient(Server, void (*)(Client));
/**
 * @brief  Event triggered when a client sent a message to the server
 * @param Server server to use
 * @param  void(*)(Client,void*,int,void**,int*) function to call when the event is triggered. Note : void* is the data, int id its size, void** is a pointer to the response data and int* the size of the response.
 * @return void
 */
void Server_onMessageRecieved(Server, void (*)(Client, void *, int, void **, int *));
/**
 * @brief  Starts the server. The thread will be blocked.
 * @param Server server to use
 * @return void
 */
void Server_run(Server);

#endif