#ifndef DEF_SERVER_SERVER_H
#define DEF_SERVER_SERVER_H
#include <stdbool.h>
#include <netinet/in.h>

#include "Client.h"

typedef struct _server_ *Server;
/**
 * @brief  Initialize a new instance of a server
 * @param uint16_t port to use for the server
 * @return the newly created server, or NULL if there is an error
 */
Server Server_create(uint16_t);
/**
 * @brief  Destroies the server
 * @param Server server to use
 * @return void
 */
void Server_destroy(Server);
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
 * @brief  Event triggered when a client sent a message to the server. The data is already allocated for 32000 bytes.
 * @param Server server to use
 * @param  size_t(*)(Client,void*,size_t,void*) function to call when the event is triggered. Note : void* is the data, size_t is its size, void* is the response data and the return value is the size of the response 
 * @return void
 */
void Server_onMessageRecieved(Server, size_t (*)(Client, void *, size_t, void *));
/**
 * @brief  Starts the server. The thread will be blocked.
 * @param Server server to use
 * @return false if the server couldn't run
 */
bool Server_run(Server);

#endif