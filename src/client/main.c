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
        int size = Client_send(client, buffer, strlen(buffer) + 1, response);
        if (size != -1)
            printf("Response:%s", response);
    }
    return 0;
}