#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "Client.h"
#include "../consoleManagement.h"
#include "../requests.h"
#include "../utils.h"

static Client client;

size_t serializeId(char *name, char *surname, void *buffer)
{
    uint64_t nameLen = strlen(name), surnameLen = strlen(surname);
    *(uint64_t *)buffer = nameLen;
    buffer += sizeof(uint64_t);
    memcpy(buffer, name, sizeof(char) * nameLen);
    buffer += nameLen * sizeof(char);
    *(uint64_t *)buffer = surnameLen;
    buffer += sizeof(uint64_t);
    memcpy(buffer, surname, sizeof(char) * surnameLen);
    return (sizeof(uint64_t) << 1) + nameLen * sizeof(char) + surnameLen * sizeof(char);
}

void viewList(char *name, char *surname)
{
    char _b1[300];
    void *buffer = _b1;
    char _b2[1024];
    void *response = _b2;
    *(uint8_t *)buffer = LIST_REQUEST;
    buffer += sizeof(uint8_t);
    int bufferSize = sizeof(uint8_t) + serializeId(name, surname, buffer);
    Client_send(client, _b1, bufferSize, response);
    int quantity = *(uint8_t *)response;
    response += sizeof(uint8_t);
    char **list = (char **)malloc(sizeof(char *));
    for (int i = 0; i < quantity; i++)
    {
        list[i] = (char *)malloc(sizeof(char) * 11);
        memcpy(list[i], response, sizeof(char) * 10);
        list[i][10] = '\0';
        response += sizeof(char) * 10;
        printf("%s\n", list[i]);
    }

    for (int i = 0; i < quantity; i++)
        free(list[i]);
    free(list);
    //TODO see/remove element
}

void requestPlace(char *name, char *surname)
{
    char _b1[300];
    void *buffer = _b1;
    char _b2[1024];
    void *response = _b2;
    *(uint8_t *)buffer = AVAILABILITY_REQUEST;
    buffer += sizeof(uint8_t);
    int bufferSize = sizeof(uint8_t) + serializeId(name, surname, buffer);
    Client_send(client, _b1, bufferSize, response);
    bool places[100];
    for (int i = 0; i < 13; ++i)
        for (int j = 0; j < 8; ++j)
        {
            int currID = i * 8 + j;
            if (currID < 100)
            {
                places[currID] = ((uint8_t *)response)[i] & (1 << j);
            }
            else
                break;
        }
    console_clearScreen();
    console_formatSystemForegroundMode("RETOUR", CONSOLE_COLOR_BRIGHT_YELLOW, CONSOLE_FLAG_REVERSE_COLOR);
    console_setCursorPosition(1, 2);
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
    int currSelection = 100;
    while (true)
    {
        int key = CONSOLE_KEY_OTHER;
        while (key == CONSOLE_KEY_OTHER)
            key = console_getArrowPressed();
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
        switch (key)
        {
        case CONSOLE_KEY_UP:
            if (currSelection <= 24)
                currSelection = 100;
            else if (currSelection < 100)
                currSelection -= 25;
            break;
        case CONSOLE_KEY_DOWN:
            if (currSelection == 100)
                currSelection = 0;
            else if (currSelection < 3 * 25)
                currSelection += 25;
            break;
        case CONSOLE_KEY_LEFT:
            if (currSelection != 100 && currSelection != 0 && currSelection != 25 && currSelection != 50 && currSelection != 75)
                --currSelection;
            break;
        case CONSOLE_KEY_RIGHT:
            if (currSelection != 100 && currSelection != 24 && currSelection != 49 && currSelection != 74 && currSelection != 99)
                ++currSelection;
            break;
        case CONSOLE_KEY_RETURN:

            return;
        }
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
    address = strcmp("localhost", argv[1]) ? argv[1] : "127.0.0.1";
    port = atoi(argv[2]);
    char username[100] = "unknown";
    username[0] = '\0';
    for (int i = 0; envVars[i] != NULL; i++)
    {
        char key[5];
        memcpy(key, envVars[i], sizeof(char) * 4);
        key[4] = '\0';
        if (!strcmp(key, "USER"))
        {
            int nameSize = strlen(envVars[i]) - 5;
            memcpy(username, envVars[i] + 5, sizeof(char) * nameSize);
            username[nameSize] = '\0';
            break;
        }
    }

    char name[100];
    char surname[100];

    console_clearScreen();
    printf("Prénom:");
    fgets(name, 100, stdin);
    name[strlen(name) - 1] = '\0';
    printf("Nom:");
    fgets(surname, 100, stdin);
    surname[strlen(surname) - 1] = '\0';
    client = Client_createS(username, address, port);
    if (!client)
        return 0;
    console_clearScreen();
    console_formatSystemForeground("Connecté à %s:%d", CONSOLE_COLOR_BRIGHT_GREEN, address, port);
    printf("\n");
    {
        uint8_t nameSize = strlen(name), surnameSize = strlen(surname);
        char _b[201];
        void *buffer = _b;
        *(uint8_t *)buffer = NAME_REQUEST;
        buffer += sizeof(uint8_t);
        *(uint8_t *)buffer = nameSize;
        buffer += sizeof(uint8_t);
        *(uint8_t *)buffer = surnameSize;
        buffer += sizeof(uint8_t);
        memcpy(buffer, name, nameSize);
        buffer += sizeof(char) * nameSize;
        memcpy(buffer, surname, surnameSize);
        Client_send(client, _b, sizeof(uint8_t) * 3 + sizeof(char) * (nameSize + surnameSize), NULL);
    }
    char *mainMenu[] =
        {"Voir les inscriptions",
         "Demander une inscription",
         "Quitter"};
    int mainMenuColors[] =
        {CONSOLE_COLOR_BRIGHT_GREY,
         CONSOLE_COLOR_BRIGHT_GREY,
         CONSOLE_COLOR_BRIGHT_RED};
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
    return 0;
}