#ifndef DEF_CLIENT_H
#define DEF_CLIENT_H
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <netinet/in.h>

typedef struct _client_ *Client;

/**
 * @brief  Initialize the client
 * @param  char* username
 * @param char* IP address
 * @param uint16_t port
 * @return the newly created server, or NULL if there is an error
 */
Client Client_createS(char *, char *, uint16_t);

/**
 * @brief  Initialize the client
 * @param  Client client to initialize 
 * @param  char* username
 * @param in_addr IP address
 * @param uint16_t port
 * @return the newly created server, or NULL if there is an error
 */
Client Client_createI(char *, struct in_addr, uint16_t);
/**
 * @brief  estroys the client
 * @param Client client to use
 * @return void
 */
void Client_disconnect(Client);
/**
 * @brief  estroys the client
 * @param Client client to use
 * @return void
 */
void Client_destroy(Client);
/**
 * @brief  Send data to the server
 * @param Client client to use
 * @param void* data to send
 * @param  size_t size of the data to send
 * @param  void* buffer to write the response data inside
 * @return the size of the response, or -1 if the client got disconnected
 */
ssize_t Client_send(Client, void *, size_t, void *);

#endif