#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include "Client.h"
#include "../consoleManagement.h"
#include "../requests.h"

static Client client;
/*
* Fonction sérialisant un couple nom/prénom dans un buffer
*/
size_t serializeId(char *name, char *surname, void *buffer)
{
    //on récupère les tailles
    uint64_t nameLen = strlen(name), surnameLen = strlen(surname);
    //on indique la taille du nom dans les 8 premiers octets
    *(uint64_t *)buffer = nameLen;
    buffer += sizeof(uint64_t);
    //puis on écrit le nom
    memcpy(buffer, name, sizeof(char) * nameLen);
    //même chose, mais pour le prénom
    buffer += nameLen * sizeof(char);
    *(uint64_t *)buffer = surnameLen;
    buffer += sizeof(uint64_t);
    memcpy(buffer, surname, sizeof(char) * surnameLen);
    //on retourne la taille utilisée dans le buffer
    return (sizeof(uint64_t) << 1) + nameLen * sizeof(char) + surnameLen * sizeof(char);
}
/*
* Fonction permettant de visualiser les réservations
*/
void viewList(char *name, char *surname)
{

    int page = 0;
    int pageCount;
    bool exitLoop = false;
    bool fullRefresh = true;
    int quantity;
    uint8_t *listId = NULL;
    char **list;
    while (!exitLoop)
    {
        //la première boucle s'occupe d'afficher chaque ligne
        if (fullRefresh)
        {
            //un "fullRefresh" signifie qu'on récupère les données du server
            fullRefresh = false;
            if (listId != NULL)
            {
                //si c'est pas le premier full refresh, il fait libérer d'abord la mémoire
                for (int i = 0; i < quantity; i++)
                    free(list[i]);
                free(list);
                free(listId);
            }
            //buffer de la requete
            uint8_t _b1[300];
            void *buffer = _b1;
            //buffer de la réponse du server
            uint8_t _b2[1024];
            void *response = _b2;
            //on indique le type de requete dans le premier octet
            *(uint8_t *)buffer = LIST_REQUEST;
            buffer += sizeof(uint8_t);
            //puis on sérialize le nom/prénom et on envoie la requete
            if (Client_send(client, _b1, sizeof(uint8_t) + serializeId(name, surname, buffer), response) == -1)
            {
                console_clearScreen();
                console_formatSystemForeground("Client déconnecté", CONSOLE_COLOR_BRIGHT_RED);
                printf("\n");
                exit(-1);
            }
            //le premier octet de la réponse est la quantité
            quantity = *(uint8_t *)response;
            response += sizeof(uint8_t);
            //on alloue les listes
            list = (char **)malloc(sizeof(char *) * quantity);
            listId = (uint8_t *)malloc(sizeof(uint8_t) * quantity);
            for (int i = 0; i < quantity; i++)
            {
                //on remplie dans chaque liste, chaque réservation
                //le numéro de place se trouve au premier octet
                listId[i] = *(uint8_t *)response;
                response += sizeof(uint8_t);
                //on alloue les 10+1 caractères du numéro de dossier
                list[i] = (char *)malloc(sizeof(char) * 11);
                //puis on récupère le-dit numéro
                memcpy(list[i], response, sizeof(char) * 10);
                //sans oublier...
                list[i][10] = '\0';
                response += sizeof(char) * 10;
            }
            //10 éléments par page
            pageCount = quantity / 10 + (quantity % 10 != 0);
        }
        bool refresh = false;
        bool confirmation = false;
        console_clearScreen();
        printf("Page %d sur %d :\n", page + 1, pageCount == 0 ? 1 : pageCount);
        //sélection actuelle par défaut sur le bouton RETOUR
        int selection = -1;
        //vive les ternaires
        int displayedEntries = quantity <= 10 ? quantity : (pageCount - 1 == page ? quantity % 10 : 10);
        for (int i = page * 10; i < page * 10 + displayedEntries; ++i)
        {
            //on affiche le numéro de place
            console_formatSystemForeground("%d", CONSOLE_COLOR_BRIGHT_YELLOW, listId[i] + 1);
            printf(" : ");
            //puis le numéro de dossier
            console_formatSystemForeground("%s", CONSOLE_COLOR_BRIGHT_GREY, list[i]);
            printf(" ");
            //et une jolie petite croix pour dire qu'on peut supprimer, vous allez voir c'est tout mignon UwU
            console_formatSystemForeground("X", CONSOLE_COLOR_BRIGHT_RED);
            printf("\n");
        }
        //bouton RETOUR en bas de page
        console_formatSystemForegroundMode("RETOUR", CONSOLE_COLOR_BRIGHT_YELLOW, CONSOLE_FLAG_REVERSE_COLOR);
        printf("\n");
        while (!refresh)
        {
            //la deuxième boucle sert à la navigation

            //on attend une touche de flèche valide...
            int key = CONSOLE_KEY_OTHER;
            console_setCursorPosition(1, 15);
            while (key == CONSOLE_KEY_OTHER)
                key = console_getArrowPressed();
            //on affiche la valeur sur laquelle on se trouve, comme si notre curseur n'était pas desus (en gros, on efface la selection)
            if (selection == -1)
            {
                console_setCursorPosition(1, displayedEntries + 2);
                console_formatSystemForeground("RETOUR", CONSOLE_COLOR_BRIGHT_YELLOW);
            }
            else
            {
                console_setCursorPosition(1, 2 + selection);
                console_formatSystemForeground("%d", CONSOLE_COLOR_BRIGHT_YELLOW, listId[page * 10 + selection] + 1);
                printf(" : ");
                console_formatSystemForeground("%s", CONSOLE_COLOR_BRIGHT_GREY, list[page * 10 + selection]);
                printf(" ");
                console_formatSystemForeground("X", CONSOLE_COLOR_BRIGHT_RED);
                console_eraseEndOfLine();
            }
            //selon la touche flèche utilisée...
            switch (key)
            {
            case CONSOLE_KEY_UP:
                //si pas le bouton retour, on décrémente
                if (selection > -1)
                    selection--;
                else
                    //sinon on mets le curseur en bas de la liste
                    selection = displayedEntries - 1;
                //à chaque mouvement, on annule les éventuelles confirmations en attente
                confirmation = false;
                break;
            case CONSOLE_KEY_DOWN:
                //si pas le dernier de la liste choix
                if (selection < displayedEntries - 1)
                    selection++;
                else
                    //sinon on mets le curseur sur le bouton retour
                    selection = -1;
                confirmation = false;
                break;
            case CONSOLE_KEY_LEFT:
                //changement de page...
                if (page > 0)
                {
                    page--;
                    //un changement de page nécéssite de sortir de la deuxième boucle
                    refresh = true;
                    confirmation = false;
                }
                break;
            case CONSOLE_KEY_RIGHT:
                //changement de page...
                if (page < pageCount - 1)
                {
                    page++;
                    //un changement de page nécéssite de sortir de la deuxième boucle
                    refresh = true;
                    confirmation = false;
                }
                break;
            case CONSOLE_KEY_RETURN:
                if (selection == -1)
                {
                    //bouton retour -> on sort des deux boucles
                    refresh = true;
                    exitLoop = true;
                }
                else if (!confirmation)
                    //sinon, on tente une confirmation
                    confirmation = true;
                else
                {
                    //si la confirmation est déjà en attente, on tente de supprimer la réservation
                    //en supprimant, on doit réafficher les résultats, et refaire une requete pour
                    //obtenir les valeurs encore présentes
                    refresh = true;
                    fullRefresh = true;
                    //buffer de la requete
                    char _b[220];
                    void *buffer = _b;
                    //type de requete dans le premier octet
                    *(uint8_t *)buffer = REMOVE_REQUEST;
                    buffer += sizeof(uint8_t);
                    //on sérialise le nom/prénom en premier
                    buffer += serializeId(name, surname, buffer);
                    //puis on y ajoute le numéro de dossier
                    memcpy(buffer, list[selection + page * 10], 10 * sizeof(char));
                    uint8_t response;
                    //on envoie la requete en attente d'une réponse
                    if (Client_send(client, _b, (size_t)buffer - (size_t)_b + 10 * sizeof(char), &response) == -1)
                    {
                        console_clearScreen();
                        console_formatSystemForeground("Client déconnecté", CONSOLE_COLOR_BRIGHT_RED);
                        printf("\n");
                        exit(-1);
                    }
                    //petit menu récapitulatif de l'action
                    console_clearScreen();
                    console_formatSystemForegroundMode("RETOUR", CONSOLE_COLOR_BRIGHT_YELLOW, CONSOLE_FLAG_REVERSE_COLOR);
                    console_setCursorPosition(2, 4);
                    //la réponse vaut 1 si tout se passe bien, ou 0 s'il y a eu un problème. La raison du problème est visible côté serveur
                    if (response)
                        console_formatSystemForeground("Réservation annulée !", CONSOLE_COLOR_BRIGHT_GREEN);
                    else
                        console_formatSystemForeground("Impossible d'annuler la réservation, veuillez réessayer.", CONSOLE_COLOR_BRIGHT_RED);
                    console_setCursorPosition(1, 15);
                    //puis on attends que l'utilisateur appuies sur ENTRÉE
                    while (console_getArrowPressed() != CONSOLE_KEY_RETURN)
                        ;
                }
                break;
            }
            //Si on ne change pas de page/supprime pas de commande
            if (!refresh)
            {
                //dans ce cas, on affiche notre curseur dans la console
                if (selection == -1)
                {
                    console_setCursorPosition(1, displayedEntries + 2);
                    console_formatSystemForegroundMode("RETOUR", CONSOLE_COLOR_BRIGHT_YELLOW, CONSOLE_FLAG_REVERSE_COLOR);
                }
                else
                {
                    console_setCursorPosition(1, 2 + selection);
                    //quand on a une confirmation en attente, l'affichage change légèrement
                    if (!confirmation)
                    {
                        console_formatSystemForegroundMode("%d", CONSOLE_COLOR_BRIGHT_YELLOW, CONSOLE_FLAG_REVERSE_COLOR, listId[page * 10 + selection] + 1);
                        printf(" : ");
                        console_formatSystemForegroundMode("%s", CONSOLE_COLOR_BRIGHT_GREY, CONSOLE_FLAG_REVERSE_COLOR, list[page * 10 + selection]);
                        printf(" ");
                        console_formatSystemForegroundMode("X", CONSOLE_COLOR_BRIGHT_RED, CONSOLE_FLAG_REVERSE_COLOR);
                    }
                    else
                    {
                        console_formatSystemForeground("%d", CONSOLE_COLOR_BRIGHT_YELLOW, listId[page * 10 + selection] + 1);
                        printf(" : ");
                        console_formatSystemForeground("%s", CONSOLE_COLOR_BRIGHT_GREY, list[page * 10 + selection]);
                        printf(" ");
                        console_formatSystemColor("Annuler la réservation ?", CONSOLE_COLOR_WHITE, CONSOLE_COLOR_RED);
                    }
                }
            }
        }
    }
    //sans oublier...
    for (int i = 0; i < quantity; i++)
        free(list[i]);
    free(list);
    free(listId);
}
/*
* Fonction permettant de visualiser les places et demander une réservation
*/
void requestPlace(char *name, char *surname)
{
    //buffer de la requete
    uint8_t _b1[300];
    void *buffer = _b1;
    //buffer de la réponse
    uint8_t _b2[1024];
    void *response = _b2;
    //type de la requete dans le premier octet
    *(uint8_t *)buffer = AVAILABILITY_REQUEST;
    buffer += sizeof(uint8_t);
    //puis on ajoute le nom/prénom dans la requete
    int bufferSize = sizeof(uint8_t) + serializeId(name, surname, buffer);
    //on envoie tout ça
    if (Client_send(client, _b1, bufferSize, response) == -1)
    {
        console_clearScreen();
        console_formatSystemForeground("Client déconnecté", CONSOLE_COLOR_BRIGHT_RED);
        printf("\n");
        exit(-1);
    }
    bool places[100];
    //la réponse est codée sur 100 bits, soit 13 octets, un bit par place
    for (int i = 0; i < 13; ++i)
        for (int j = 0; j < 8; ++j)
        {
            //on récupère l'id réel de la place
            int currID = i * 8 + j;
            //comme 13 octets vaut 104 bits, on doit vérifier qu'on ne va pas trop loin
            if (currID < 100)
            {
                //on récupère le bit correspondant à la place
                //pour cela, on récupère d'abord l'octet correspondant, puis on fait un ET binaire
                //avec le bit qu'on recherche (j), grâce à un décalage binaire
                //quand j vaut 0, on a 0b1
                //quand j vaut 2, on a 0b100
                //quand j vaut 7, on a 0b10000000
                places[currID] = ((uint8_t *)response)[i] & (1 << j);
            }
            else
                break;
        }
    console_clearScreen();
    console_formatSystemForegroundMode("RETOUR", CONSOLE_COLOR_BRIGHT_YELLOW, CONSOLE_FLAG_REVERSE_COLOR);
    console_setCursorPosition(1, 2);
    //on dessine un tableau
    //le tableau a été réalisé au pifomètre (et surtout après beaucoup de tests) plutot que de perdre du temps avec des maths,
    //donc impossible à vraiment expliquer
    printf("┌");
    for (int i = 0; i < 25 * 3 + 24; ++i)
        printf((i + 1) % 4 ? "─" : "┬");
    printf("┐");
    console_setCursorPosition(1, 10);
    printf("└");
    for (int i = 0; i < 25 * 3 + 24; ++i)
        printf((i + 1) % 4 ? "─" : "┴");
    printf("┘");
    for (int i = 0; i < 4 + 3; ++i)
    {
        console_setCursorPosition(1, 3 + i);
        printf(i % 2 ? "├" : "│");
        console_setCursorPosition(25 * 3 + 24 + 2, 3 + i);
        printf(i % 2 ? "┤" : "│");
    }

    for (int i = 0; i < 4 + 3; ++i)
    {
        console_setCursorPosition(2, i + 3);
        if (i % 2)
            for (int j = 0; j < 25 * 3 + 24; ++j)
                printf((j + 1) % 4 ? "─" : "┼");
        else
            for (int j = 0; j < 25 * 3 + 24; ++j)
                printf((j + 1) % 4 ? " " : "│");
    }
    for (int i = 0; i < 100; ++i)
    {
        console_setCursorPosition(2 + (i != 99) + (i % 25) * 4, 3 + 2 * (i / 25));
        console_formatSystemForeground("%d", places[i] ? CONSOLE_COLOR_BRIGHT_GREEN : CONSOLE_COLOR_BRIGHT_RED, i + 1);
    }
    //le curseur se trouve de base sur RETOUR (valeur 100)
    int currSelection = 100;
    while (true)
    {
        int key = CONSOLE_KEY_OTHER;
        console_setCursorPosition(1, 15);
        while (key == CONSOLE_KEY_OTHER)
            key = console_getArrowPressed();
        //comme dans viewList(), on efface l'ancienne valeur du curseur
        if (currSelection == 100)
        {
            console_setCursorPosition(1, 1);
            console_formatSystemForeground("RETOUR", CONSOLE_COLOR_BRIGHT_YELLOW);
        }
        else
        {
            console_setCursorPosition(2 + (currSelection % 25) * 4, 3 + 2 * (currSelection / 25));
            if (currSelection < 9)
                console_formatSystemForeground(" %d ", places[currSelection] ? CONSOLE_COLOR_BRIGHT_GREEN : CONSOLE_COLOR_BRIGHT_RED, currSelection + 1);
            else if (currSelection < 99)
                console_formatSystemForeground(" %d", places[currSelection] ? CONSOLE_COLOR_BRIGHT_GREEN : CONSOLE_COLOR_BRIGHT_RED, currSelection + 1);
            else
                console_formatSystemForeground("%d", places[currSelection] ? CONSOLE_COLOR_BRIGHT_GREEN : CONSOLE_COLOR_BRIGHT_RED, currSelection + 1);
        }
        //selon la touche de flèche
        switch (key)
        {
        case CONSOLE_KEY_UP:
            //si on se trouve sur la première ligne -> bouton retour
            if (currSelection <= 24)
                currSelection = 100;
            //sinon on monte d'une ligne
            else if (currSelection < 100)
                currSelection -= 25;
            break;
        case CONSOLE_KEY_DOWN:
            //si on se trouve sur retour -> première case
            if (currSelection == 100)
                currSelection = 0;
            //sinon on descent d'une ligne
            else if (currSelection < 3 * 25)
                currSelection += 25;
            break;
        case CONSOLE_KEY_LEFT:
            //si on se trouve pas en première colonne
            if (currSelection != 100 && currSelection != 0 && currSelection != 25 && currSelection != 50 && currSelection != 75)
                --currSelection;
            break;
        case CONSOLE_KEY_RIGHT:
            //si on se trouve pas en dernière colonne
            if (currSelection != 100 && currSelection != 24 && currSelection != 49 && currSelection != 74 && currSelection != 99)
                ++currSelection;
            break;
        case CONSOLE_KEY_RETURN:
            if (currSelection == 100)
                return;
            if (places[currSelection])
            {
                //on construit la requete

                //buffer de la requete
                uint8_t _b[300];
                void *buffer = _b;
                //type de la requete dans le premier octet
                *(uint8_t *)buffer = PLACE_REQUEST;
                buffer += sizeof(uint8_t);
                //puis on sérialize le nom/prénom
                buffer += serializeId(name, surname, buffer);
                //puis le numéro de la place
                *(uint8_t *)buffer = currSelection;
                uint8_t response;
                if (Client_send(client, _b, (size_t)buffer - (size_t)_b + sizeof(uint8_t), &response) == -1)
                {
                    console_clearScreen();
                    console_formatSystemForeground("Client déconnecté", CONSOLE_COLOR_BRIGHT_RED);
                    printf("\n");
                    exit(-1);
                }
                //petit écran récapitulatif
                console_clearScreen();
                console_formatSystemForegroundMode("RETOUR", CONSOLE_COLOR_BRIGHT_YELLOW, CONSOLE_FLAG_REVERSE_COLOR);
                console_setCursorPosition(2, 4);
                if (response)
                    console_formatSystemForeground("Place réservée !", CONSOLE_COLOR_BRIGHT_GREEN);
                else
                    console_formatSystemForeground("Impossible de réserver la place, veuillez réessayer.", CONSOLE_COLOR_BRIGHT_RED);
                console_setCursorPosition(1, 15);
                while (console_getArrowPressed() != CONSOLE_KEY_RETURN)
                    ;
                return;
            }
        }
        //on affiche le curseur
        if (currSelection == 100)
        {
            console_setCursorPosition(1, 1);
            console_formatSystemForegroundMode("RETOUR", CONSOLE_COLOR_BRIGHT_YELLOW, CONSOLE_FLAG_REVERSE_COLOR);
        }
        else
        {
            console_setCursorPosition(2 + (currSelection % 25) * 4, 3 + 2 * (currSelection / 25));
            if (currSelection < 9)
                console_formatSystemForegroundMode(" %d ", places[currSelection] ? CONSOLE_COLOR_BRIGHT_GREEN : CONSOLE_COLOR_BRIGHT_RED, CONSOLE_FLAG_REVERSE_COLOR, currSelection + 1);
            else if (currSelection < 99)
                console_formatSystemForegroundMode(" %d", places[currSelection] ? CONSOLE_COLOR_BRIGHT_GREEN : CONSOLE_COLOR_BRIGHT_RED, CONSOLE_FLAG_REVERSE_COLOR, currSelection + 1);
            else
                console_formatSystemForegroundMode("%d", places[currSelection] ? CONSOLE_COLOR_BRIGHT_GREEN : CONSOLE_COLOR_BRIGHT_RED, CONSOLE_FLAG_REVERSE_COLOR, currSelection + 1);
        }
        console_setCursorPosition(1, 16);
    }
}

int main(int argc, char **argv, char **envVars)
{
    char *address;
    int port;
    if (argc < 3)
    {
        console_formatSystemForeground("Usage : client [<adresse IP>|localhost] <port>", CONSOLE_COLOR_BRIGHT_RED);
        printf("\n");
        return 1;
    }
    //si l'adresse donnée est localhost, on prends 127.0.0.1
    address = strcmp("localhost", argv[1]) ? argv[1] : "127.0.0.1";
    port = atoi(argv[2]);
    //par défaut, on ne connait pas l'username
    char username[100] = "unknown";
    for (int i = 0; envVars[i] != NULL; i++)
    {
        //on cherche l'username dans les variables d'environnement
        char key[5];
        memcpy(key, envVars[i], sizeof(char) * 4);
        key[4] = '\0';
        if (!strcmp(key, "USER"))
        {
            //trouvé
            int nameSize = strlen(envVars[i]) - 5;
            memcpy(username, envVars[i] + 5, sizeof(char) * nameSize);
            username[nameSize] = '\0';
            break;
        }
    }

    char name[100];
    char surname[100];

    client = Client_createS(username, address, port);
    //on demande les informations du client
    console_clearScreen();
    printf("Prénom:");
    fgets(name, 100, stdin);
    //on supprime le retour à la ligne
    name[strlen(name) - 1] = '\0';
    printf("Nom:");
    fgets(surname, 100, stdin);
    surname[strlen(surname) - 1] = '\0';
    if (!client)
        return 0;
    console_clearScreen();
    console_formatSystemForeground("Connecté à %s:%d", CONSOLE_COLOR_BRIGHT_GREEN, address, port);
    printf("\n");
    {
        //on prépare la requete indiquant le nom/prénom
        uint8_t nameSize = strlen(name), surnameSize = strlen(surname);
        uint8_t _b[201];
        void *buffer = _b;
        //type de requete dans le premier octet
        *(uint8_t *)buffer = NAME_REQUEST;
        buffer += sizeof(uint8_t);
        //les deux octets suivants contiennent la taille du nom/prénom
        *(uint8_t *)buffer = nameSize;
        buffer += sizeof(uint8_t);
        *(uint8_t *)buffer = surnameSize;
        buffer += sizeof(uint8_t);
        //puis on mets le nom/prénom
        memcpy(buffer, name, nameSize);
        buffer += sizeof(char) * nameSize;
        memcpy(buffer, surname, surnameSize);
        //puis on envoie simplement
        if (Client_send(client, _b, sizeof(uint8_t) * 3 + sizeof(char) * (nameSize + surnameSize), NULL) == -1)
        {
            console_clearScreen();
            console_formatSystemForeground("Client déconnecté", CONSOLE_COLOR_BRIGHT_RED);
            printf("\n");
            exit(-1);
        }
    }
    char *mainMenu[] =
        {"Voir les inscriptions",
         "Demander une inscription",
         "Quitter"};
    int mainMenuColors[] =
        {CONSOLE_COLOR_BRIGHT_GREY,
         CONSOLE_COLOR_BRIGHT_GREY,
         CONSOLE_COLOR_BRIGHT_RED};
    //selection à 0 par défaut (premier élément)
    int mainSelection = 0;
    while (true)
    {
        console_clearScreen();
        mainSelection %= 3;
        for (int i = 0; i < 3; i++)
        {
            console_setCursorPosition(0, i + 1);
            if (mainSelection == i)
                console_formatSystemForegroundMode(mainMenu[i], mainMenuColors[i], CONSOLE_FLAG_REVERSE_COLOR);
            else
                console_formatSystemForeground(mainMenu[i], mainMenuColors[i]);
        }
        console_setCursorPosition(0, 8);
        int key = CONSOLE_KEY_OTHER;
        while (key != CONSOLE_KEY_UP && key != CONSOLE_KEY_DOWN && key != CONSOLE_KEY_RETURN)
            key = console_getArrowPressed();
        switch (key)
        {
        case CONSOLE_KEY_DOWN:
            ++mainSelection;
            break;
        case CONSOLE_KEY_UP:
            mainSelection += 2;
            break;
        case CONSOLE_KEY_RETURN:
        {
            switch (mainSelection)
            {
            case 0:
                viewList(name, surname);
                break;
            case 1:
                requestPlace(name, surname);
                break;
            case 2:
                return 0;
            }
        }
        break;
        }
    }
    Client_destroy(client);
    return 0;
}