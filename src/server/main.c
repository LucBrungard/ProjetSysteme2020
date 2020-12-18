#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "Server.h"
#include "../consoleManagement.h"
#include "../requests.h"

void deserializeId(void *buffer, char *name, char *surname)
{
    uint64_t nameLen = *(uint64_t *)buffer;
    buffer += sizeof(uint64_t);
    memcpy(name, buffer, sizeof(char) * nameLen);
    name[nameLen] = '\0';
    buffer += nameLen * sizeof(char);

    uint64_t surnameLen = *(uint64_t *)buffer;
    buffer += sizeof(uint64_t);
    memcpy(surname, buffer, sizeof(char) * surnameLen);
    surname[surnameLen] = '\0';
    buffer += surnameLen * sizeof(char);
}

typedef struct _place_
{
    char *reservationNumber;
    char *name;
    char *surname;
} * Place;

typedef struct _serverData_
{
    Place *places;
} * ServerData;

static ServerData serverData;

void clientConnected(Client client)
{
    console_formatSystemForegroundMode("%s@%s", CONSOLE_COLOR_BRIGHT_BLUE, CONSOLE_FLAG_BOLD, Client_getUsername(client), Client_getIpS(client));
    printf(":");
    console_formatSystemForeground("Connecté", CONSOLE_COLOR_BRIGHT_GREEN);
    printf("\n");
}
void clientDisconnected(Client client)
{
    console_formatSystemForegroundMode("%s@%s", CONSOLE_COLOR_BRIGHT_BLUE, CONSOLE_FLAG_BOLD, Client_getUsername(client), Client_getIpS(client));
    printf(":");
    console_formatSystemForeground("Déconnecté", CONSOLE_COLOR_BRIGHT_RED);
    printf("\n");
}
size_t message(Client client, void *data, size_t dataSize, void *buffer)
{
    uint8_t requestType = *(uint8_t *)data;
    data += sizeof(uint8_t);
    switch (requestType)
    {
    case LIST_REQUEST:
    {
        char name[100], surname[100];
        deserializeId(data, name, surname);
        void *quantityPtr = buffer;
        buffer += sizeof(uint8_t);
        uint8_t quantity = 0;
        for (int i = 0; i < 100; i++)
            if (serverData->places[i]->reservationNumber != NULL)
                if (!strcmp(serverData->places[i]->name, name) && !strcmp(serverData->places[i]->surname, surname))
                {
                    memcpy(buffer, serverData->places[i]->reservationNumber, sizeof(char) * 10);
                    buffer += 10 * sizeof(char);
                    ++quantity;
                }
        *(uint8_t *)quantityPtr = quantity;
        console_formatSystemForegroundMode("%s@%s", CONSOLE_COLOR_BRIGHT_BLUE, CONSOLE_FLAG_BOLD, Client_getUsername(client), Client_getIpS(client));
        printf(":Demande de visualisation des réservations, ");
        console_formatSystemForeground("%d", CONSOLE_COLOR_BRIGHT_YELLOW, quantity);
        printf(" réservations trouvées\n");
        return sizeof(uint8_t) + sizeof(char) * 10 * quantity;
    }
    break;
    case NAME_REQUEST:
    {
        char name[100], surname[100];
        uint8_t nameSize = *(uint8_t *)data;
        data += sizeof(uint8_t);
        uint8_t surnameSize = *(uint8_t *)data;
        data += sizeof(uint8_t);
        memcpy(name, data, sizeof(char) * nameSize);
        data += sizeof(char) * nameSize;
        name[nameSize] = '\0';
        memcpy(surname, data, sizeof(char) * surnameSize);
        surname[surnameSize] = '\0';
        console_formatSystemForegroundMode("%s@%s", CONSOLE_COLOR_BRIGHT_BLUE, CONSOLE_FLAG_BOLD, Client_getUsername(client), Client_getIpS(client));
        printf(":En tant que ");
        console_formatMode("%s %s", CONSOLE_FLAG_UNDERLINE, name, surname);
        printf("\n");
        return 0;
    }
    break;
    case AVAILABILITY_REQUEST:
    {
        char name[100], surname[100];
        deserializeId(data, name, surname);
        for (int i = 0; i < 13; i++)
        {
            uint8_t currState = 0;
            for (int j = 0; j < 8; j++)
            {
                int currID = i * 8 + j;
                if (currID < 100)
                {
                    if (j > 0)
                        currState <<= 1;
                    currState |= serverData->places[currID]->reservationNumber == NULL;
                }
                else
                    break;
            }
            *(uint8_t *)buffer = currState;
            buffer += sizeof(uint8_t);
        }
        console_formatSystemForegroundMode("%s@%s", CONSOLE_COLOR_BRIGHT_BLUE, CONSOLE_FLAG_BOLD, Client_getUsername(client), Client_getIpS(client));
        printf(":Demande de visualisation des places disponibles\n");
        return 13 * sizeof(uint8_t);
    }
    break;
    default:
        break;
    }
    return 0;
}

int main(int argc, char **argv, char **args)
{
    Place _pl[100];
    serverData = (ServerData)malloc(sizeof(struct _serverData_));
    serverData->places = _pl;
    for (int i = 0; i < 100; i++)
    {
        serverData->places[i] = (Place)malloc(sizeof(struct _place_));
        serverData->places[i]->reservationNumber = NULL;
    }
    if (argc < 2)
    {
        console_formatSystemForeground("Usage : server <port>", CONSOLE_COLOR_BRIGHT_RED);
        printf("\n");
        return 1;
    }
    uint16_t port = atoi(argv[1]);
    Server server = Server_create(port);
    if (!server)
        return 0;
    Server_onConnectedClient(server, &clientConnected);
    Server_onDisconnectedClient(server, &clientDisconnected);
    Server_onMessageRecieved(server, &message);
    console_formatSystemForeground("Serveur initialisé au port %d", CONSOLE_COLOR_BRIGHT_GREEN, port);
    printf("\n");
    Server_run(server);
    return 0;
}