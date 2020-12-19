#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include "Server.h"
#include "../consoleManagement.h"
#include "../requests.h"

size_t deserializeId(void *buffer, char *name, char *surname)
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
    return 2 * sizeof(uint64_t) + sizeof(char) * (nameLen + surnameLen);
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
                    *(uint8_t *)buffer = i;
                    buffer += sizeof(uint8_t);
                    memcpy(buffer, serverData->places[i]->reservationNumber, sizeof(char) * 10);
                    buffer += 10 * sizeof(char);
                    ++quantity;
                }
        *(uint8_t *)quantityPtr = quantity;
        console_formatSystemForegroundMode("%s@%s", CONSOLE_COLOR_BRIGHT_BLUE, CONSOLE_FLAG_BOLD, Client_getUsername(client), Client_getIpS(client));
        printf(":Demande de visualisation des réservations, ");
        console_formatSystemForeground("%d", CONSOLE_COLOR_BRIGHT_YELLOW, quantity);
        printf(" réservations trouvées\n");
        return sizeof(uint8_t) + (sizeof(char) * 10 + sizeof(uint8_t)) * quantity;
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
                        currState >>= 1;
                    currState |= (serverData->places[currID]->reservationNumber == NULL) << 7;
                }
                else
                {
                    currState >>= 4;
                    break;
                }
            }
            *(uint8_t *)buffer = currState;
            buffer += sizeof(uint8_t);
        }
        console_formatSystemForegroundMode("%s@%s", CONSOLE_COLOR_BRIGHT_BLUE, CONSOLE_FLAG_BOLD, Client_getUsername(client), Client_getIpS(client));
        printf(":Demande de visualisation des places disponibles\n");
        return 13 * sizeof(uint8_t);
    }
    break;

    case PLACE_REQUEST:
    {
        char name[100], surname[100];
        uint8_t place;
        data += deserializeId(data, name, surname);
        place = *(uint8_t *)data;
        if (place > 99)
            *(uint8_t *)buffer = 0;
        else
        {
            if (serverData->places[place]->reservationNumber == NULL)
            {
                char code[11];
                code[10] = '\0';
                for (int i = 0; i < 10; i++)
                    code[i] = '0' + (rand() % 10);
                //TODO LOCK
                serverData->places[place]->name = (char *)malloc((strlen(name) + 1) * sizeof(char));
                serverData->places[place]->surname = (char *)malloc((strlen(surname) + 1) * sizeof(char));
                serverData->places[place]->reservationNumber = (char *)malloc(11 * sizeof(char));
                memcpy(serverData->places[place]->name, name, (strlen(name) + 1) * sizeof(char));
                memcpy(serverData->places[place]->surname, surname, (strlen(name) + 1) * sizeof(char));
                memcpy(serverData->places[place]->reservationNumber, code, 11 * sizeof(char));
                //TODO UNLOCK
                *(uint8_t *)buffer = 1;
            }
            else
                *(uint8_t *)buffer = 0;
        }
        console_formatSystemForegroundMode("%s@%s", CONSOLE_COLOR_BRIGHT_BLUE, CONSOLE_FLAG_BOLD, Client_getUsername(client), Client_getIpS(client));
        printf(":Demande de réservation de la place ");
        console_formatSystemForeground("%d", CONSOLE_COLOR_BRIGHT_YELLOW, place + 1);
        printf(" ");
        if (*(uint8_t *)buffer)
            console_formatSystemForeground("autorisée", CONSOLE_COLOR_BRIGHT_GREEN);
        else
            console_formatSystemForeground("refusée", CONSOLE_COLOR_BRIGHT_RED);
        printf(", dossier n°");
        console_formatMode(serverData->places[place]->reservationNumber, CONSOLE_FLAG_UNDERLINE);
        printf("\n");
        return sizeof(uint8_t);
    }
    break;
    }
    return 0;
}

int main(int argc, char **argv, char **args)
{
    srand(time(NULL));
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