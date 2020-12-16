#include <stdio.h>
#include <string.h>
#include "Client.h"

int main()
{
    printf("Waiting for host...\n");
    Client client = Client_createS("nathan", "127.0.0.1", 6000);
    if (!client)
        return 0;
    printf("Connected\n");
    while (true)
    {
        char buffer[1000];
        fgets(buffer, 1000, stdin);
        char response[1000];
        if (Client_send(client, buffer, strlen(buffer), response) == -1)
            break;
        else
            printf("Response:%s\n", response);
    }
    return 0;
}