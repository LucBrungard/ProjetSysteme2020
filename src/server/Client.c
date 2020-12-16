#include <arpa/inet.h>
#include "Client.h"

char *Client_getIpS(Client client)
{
    return inet_ntoa(client->_ip);
}
in_addr_t Client_getIpI(Client client)
{
    return client->_ip.s_addr;
}
char *Client_getUsername(Client client)
{
    return client->_username;
}
