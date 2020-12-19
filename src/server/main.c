#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <pthread.h>
#include <semaphore.h>
#include "Server.h"
#include "../consoleManagement.h"
#include "../requests.h"

//mutex
static sem_t serverDataAccess;
/*
* fonction désérialisant un nom/prénom passé via un buffer
*/
size_t deserializeId(void *buffer, char *name, char *surname)
{
    //le premier octet est la taille du prénom
    uint64_t nameLen = *(uint64_t *)buffer;
    buffer += sizeof(uint64_t);
    memcpy(name, buffer, sizeof(char) * nameLen);
    //on oublie pas
    name[nameLen] = '\0';
    buffer += nameLen * sizeof(char);
    //pareil pour le nom
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
    //on affiche le client qui s'est connecté
    console_formatSystemForegroundMode("%s@%s", CONSOLE_COLOR_BRIGHT_BLUE, CONSOLE_FLAG_BOLD, Client_getUsername(client), Client_getIpS(client));
    printf(":");
    console_formatSystemForeground("Connecté", CONSOLE_COLOR_BRIGHT_GREEN);
    printf("\n");
}
void clientDisconnected(Client client)
{
    //on affiche le client qui s'est déconnecté
    console_formatSystemForegroundMode("%s@%s", CONSOLE_COLOR_BRIGHT_BLUE, CONSOLE_FLAG_BOLD, Client_getUsername(client), Client_getIpS(client));
    printf(":");
    console_formatSystemForeground("Déconnecté", CONSOLE_COLOR_BRIGHT_RED);
    printf("\n");
}
size_t message(Client client, void *data, size_t dataSize, void *buffer)
{
    //le type de la requete est dans le premier octet
    uint8_t requestType = *(uint8_t *)data;
    data += sizeof(uint8_t);
    //selon le type de requete...
    switch (requestType)
    {
    case LIST_REQUEST:
    {
        char name[100], surname[100];
        //on récupère le nom/prénom
        deserializeId(data, name, surname);
        //le premier octet de la réponse servira plus tard pour la quantité
        void *quantityPtr = buffer;
        buffer += sizeof(uint8_t);
        uint8_t quantity = 0;
        //point sensible
        sem_wait(&serverDataAccess);

        for (int i = 0; i < 100; i++)
            //on cherche une réservation attribuée
            if (serverData->places[i]->reservationNumber != NULL)
                //on vérifie les coordonnées du client
                if (!strcmp(serverData->places[i]->name, name) && !strcmp(serverData->places[i]->surname, surname))
                {
                    //on stocke le numéro de la place
                    *(uint8_t *)buffer = i;
                    buffer += sizeof(uint8_t);
                    //on stocke le numéro de dossier
                    memcpy(buffer, serverData->places[i]->reservationNumber, sizeof(char) * 10);
                    buffer += 10 * sizeof(char);
                    ++quantity;
                }
        //fin de point sensible
        sem_post(&serverDataAccess);
        //puis on mets la quantité au début de la réponse
        *(uint8_t *)quantityPtr = quantity;
        //des logs
        console_formatSystemForegroundMode("%s@%s", CONSOLE_COLOR_BRIGHT_BLUE, CONSOLE_FLAG_BOLD, Client_getUsername(client), Client_getIpS(client));
        printf(":Demande de visualisation des réservations, ");
        console_formatSystemForeground("%d", CONSOLE_COLOR_BRIGHT_YELLOW, quantity);
        printf(" réservations trouvées\n");
        //on renvoie la taille de la réponse
        return sizeof(uint8_t) + (sizeof(char) * 10 + sizeof(uint8_t)) * quantity;
    }
    break;
    case NAME_REQUEST:
    {
        char name[100], surname[100];
        //les deux premiers octets sont les tailles du nom/prénom
        uint8_t nameSize = *(uint8_t *)data;
        data += sizeof(uint8_t);
        uint8_t surnameSize = *(uint8_t *)data;
        data += sizeof(uint8_t);
        //on récupère le prénom
        memcpy(name, data, sizeof(char) * nameSize);
        data += sizeof(char) * nameSize;
        //sans oublier...
        name[nameSize] = '\0';
        //rebelotte
        memcpy(surname, data, sizeof(char) * surnameSize);
        surname[surnameSize] = '\0';
        //logs
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
        //on récupère le nom/prénom
        deserializeId(data, name, surname);
        //point sensible
        sem_wait(&serverDataAccess);
        //la réponse ferra 13 octets (100 bits seront utilisés)
        for (int i = 0; i < 13; i++)
        {
            //pour chaque octet de la réponse
            uint8_t currState = 0; //l'octet actuel
            for (int j = 0; j < 8; j++)
            {
                //pour chaque bit

                //on calcule l'id réel de la place
                int currID = i * 8 + j;
                //on vérifie à pas aller trop loin comme on utilise 104 bits et non 100
                if (currID < 100)
                {
                    //si on est au deuxième bit minimum
                    if (j > 0)
                        //on décale tout vers la droite
                        currState >>= 1;
                    //on fait un OU binaire avec 0b10000000 (si la place n'est pas réservée)
                    currState |= (serverData->places[currID]->reservationNumber == NULL) << 7;
                }
                else
                {
                    //si on arrive au bout, on doit pousser les 4 derniers bits vers la droite
                    currState >>= 4;
                    break;
                }
            }
            //on ecrit notre octet
            *(uint8_t *)buffer = currState;
            buffer += sizeof(uint8_t);
        }
        sem_post(&serverDataAccess);
        //logs
        console_formatSystemForegroundMode("%s@%s", CONSOLE_COLOR_BRIGHT_BLUE, CONSOLE_FLAG_BOLD, Client_getUsername(client), Client_getIpS(client));
        printf(":Demande de visualisation des places disponibles\n");
        return 13 * sizeof(uint8_t);
    }
    break;

    case REMOVE_REQUEST:
    {
        char name[100], surname[100];
        //numero du dossier
        char code[11];
        code[10] = '\0';
        //on récuppère le nom/prénom
        data += deserializeId(data, name, surname);
        //on récupère le numéro de dossier
        memcpy(code, data, 10 * sizeof(char));
        //variable qui servira pour les logs
        bool found = false;
        //par défaut, on renvera 0 (erreur)
        *(uint8_t *)buffer = 0;
        //point sensible
        sem_wait(&serverDataAccess);
        //pour chaque place
        for (int i = 0; i < 100; i++)
            //si elle n'est pas réservée
            if (serverData->places[i]->reservationNumber != NULL)
            {
                //si le numéro de dossier correspond
                if (!strcmp(code, serverData->places[i]->reservationNumber))
                {
                    found = true;
                    //si le nom/prénom correspond à celui donné
                    if (!strcmp(name, serverData->places[i]->name) && !strcmp(surname, serverData->places[i]->surname))
                    {
                        //la réponse vaut 1 (sans erreur)
                        *(uint8_t *)buffer = 1;
                        //on supprime et libère la réservation
                        free(serverData->places[i]->reservationNumber);
                        free(serverData->places[i]->name);
                        free(serverData->places[i]->surname);
                        serverData->places[i]->reservationNumber = NULL;
                    }
                    break;
                }
            }
        sem_post(&serverDataAccess);
        //logs
        console_formatSystemForegroundMode("%s@%s", CONSOLE_COLOR_BRIGHT_BLUE, CONSOLE_FLAG_BOLD, Client_getUsername(client), Client_getIpS(client));
        printf("Demande d'annulation du dossier n°");
        console_formatMode(code, CONSOLE_FLAG_UNDERLINE);
        printf(" ");
        if (*(uint8_t *)buffer)
            console_formatSystemForeground("autorisée", CONSOLE_COLOR_BRIGHT_GREEN);
        else
        {
            console_formatSystemForeground("refusée", CONSOLE_COLOR_BRIGHT_RED);
            printf(", raison : ");
            if (found)
                console_formatSystemForeground("mauvais utilisateur", CONSOLE_COLOR_BRIGHT_YELLOW);
            else
                console_formatSystemForeground("dossier introuvable", CONSOLE_COLOR_BRIGHT_YELLOW);
        }
        printf("\n");
        return sizeof(uint8_t);
    }
    break;

    case PLACE_REQUEST:
    {
        char name[100], surname[100];
        uint8_t place;
        //on récupère le nom/prénom
        data += deserializeId(data, name, surname);
        place = *(uint8_t *)data;
        if (place > 99)
            //place incorrecte, erreur
            *(uint8_t *)buffer = 0;
        else
        {
            //point sensible
            sem_wait(&serverDataAccess);
            if (serverData->places[place]->reservationNumber == NULL)
            {
                //numero de dossier à générer
                char code[11];
                code[10] = '\0';
                for (int i = 0; i < 10; i++)
                    //on génère le dossier
                    code[i] = '0' + (rand() % 10);
                //on alloue tout comme il faut
                serverData->places[place]->name = (char *)malloc((strlen(name) + 1) * sizeof(char));
                serverData->places[place]->surname = (char *)malloc((strlen(surname) + 1) * sizeof(char));
                serverData->places[place]->reservationNumber = (char *)malloc(11 * sizeof(char));
                //on copie les valeurs à sauvegarder
                memcpy(serverData->places[place]->name, name, (strlen(name) + 1) * sizeof(char));
                memcpy(serverData->places[place]->surname, surname, (strlen(name) + 1) * sizeof(char));
                memcpy(serverData->places[place]->reservationNumber, code, 11 * sizeof(char));
                *(uint8_t *)buffer = 1; //plus d'erreurs
            }
            else
                *(uint8_t *)buffer = 0; //dossier déjà réservé
            sem_post(&serverDataAccess);
        }
        //logs
        console_formatSystemForegroundMode("%s@%s", CONSOLE_COLOR_BRIGHT_BLUE, CONSOLE_FLAG_BOLD, Client_getUsername(client), Client_getIpS(client));
        printf(":Demande de réservation de la place ");
        console_formatSystemForeground("%d", CONSOLE_COLOR_BRIGHT_YELLOW, place + 1);
        printf(" ");
        if (*(uint8_t *)buffer)
        {
            //aucune erreur
            console_formatSystemForeground("autorisée", CONSOLE_COLOR_BRIGHT_GREEN);
            printf(", dossier n°");
            sem_wait(&serverDataAccess);
            console_formatMode(serverData->places[place]->reservationNumber, CONSOLE_FLAG_UNDERLINE);
            sem_post(&serverDataAccess);
        }
        else
        {
            //erreur
            console_formatSystemForeground("refusée", CONSOLE_COLOR_BRIGHT_RED);
            printf(", raison : ");
            if (place > 99)
                console_formatSystemForeground("numéro de place incorrect", CONSOLE_COLOR_BRIGHT_YELLOW);
            else
                console_formatSystemForeground("place déjà occupée", CONSOLE_COLOR_BRIGHT_YELLOW);
        }
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
    sem_init(&serverDataAccess, PTHREAD_PROCESS_SHARED, 1);
    //création des données du serveur
    Place _pl[100];
    struct _serverData_ _serv;
    serverData = &_serv;
    serverData->places = _pl;
    //on alloue toutes les places
    //note : pas besoin de free() pour l'instant comme le
    //serveur sera nécéssaire pour toute la durée de vie du programme
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
    //on lie tous les évènements aux fonctions
    Server_onConnectedClient(server, &clientConnected);
    Server_onDisconnectedClient(server, &clientDisconnected);
    Server_onMessageRecieved(server, &message);
    console_formatSystemForeground("Serveur initialisé au port %d", CONSOLE_COLOR_BRIGHT_GREEN, port);
    printf("\n");
    Server_run(server);
    return 0;
}