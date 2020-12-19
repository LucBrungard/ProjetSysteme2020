#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <pthread.h>
#include <stddef.h>
#include "Server.h"

#define SOCKET_BUFFER_SIZE 4800

struct _server_
{
    int _fdWaitSocket;
    struct sockaddr_in _callerCoords;
    socklen_t _coordsSize;
    void (*_onConnect)(Client);
    void (*_onDisconnect)(Client);
    size_t (*_onMessage)(Client, void *, size_t, void *);
};

typedef struct
{
    struct sockaddr_in callerCoords;
    int fdSocket;
    Server server;
} connection;

/*
* fonction de démarrage des threads clients
*/
void *clientThread(void *arg)
{
    connection *con = (connection *)arg;
    //la structure du client géré par ce thread
    struct _client_ client;
    client._ip = con->callerCoords.sin_addr;
    char user[65];
    {
        //le client envoie toujours un nom d'utilisateur à sa connexion
        int readBytes = recv(con->fdSocket, user, 64, 0);
        if (readBytes >= 0)
        {
            user[readBytes] = '\0';
            client._username = user;
        }
        else
        {
            close(con->fdSocket);
            return NULL;
        }
    }
    //si une fonction à été donnée pour l'évènement de connexion
    if (con->server->_onConnect != NULL)
        //on l'appelle
        (*con->server->_onConnect)(&client);
    //boucle principale du thread
    while (true)
    {
        //buffer des données reçues du client
        uint8_t _b[SOCKET_BUFFER_SIZE];
        void *buffer = _b;
        ssize_t readBytes = recv(con->fdSocket, buffer, SOCKET_BUFFER_SIZE, 0);
        if (readBytes > 0)
        {
            //si l'évènement de message reçu est géré
            if (con->server->_onMessage != NULL)
            {
                //buffer des données répondues
                uint8_t _r[SOCKET_BUFFER_SIZE];
                void *response = _r;
                size_t respSize = 0;
                //on appelle l'évènement
                respSize = (*con->server->_onMessage)(&client, buffer, readBytes, response);
                if (respSize != 0)
                    //on réponds uniquement si on a une taille de réponse > 0
                    send(con->fdSocket, response, respSize, 0);
            }
        }
        else
            //déconnexion du client
            break;
    }
    //le client est déconnecté, on ferme la connexion
    close(con->fdSocket);
    //on lance l'évènement de déconnexion
    if (con->server->_onDisconnect != NULL)
        (*con->server->_onDisconnect)(&client);
    //on libère la mémoire allouée dans le thread principal
    free(con);
    pthread_exit(NULL);
}
void Server_destroy(Server server)
{
    //Attention, le server ne sera jamais vraiment détruit tant que toutes les connexions
    //avec les clients ne sont pas terminées
    free(server);
}
Server Server_create(uint16_t port)
{
    //ici on créé notre objet serveur de toute pièce
    Server server = (Server)malloc(sizeof(struct _server_));
    //beaucoup de copier/coller du cours
    struct sockaddr_in serverCoords;
    server->_fdWaitSocket = socket(PF_INET, SOCK_STREAM, 0);
    if (server->_fdWaitSocket < 0)
    {
        printf("incorrect socket\n");
        return false;
    }
    // On prépare l’adresse d’attachement locale
    memset(&serverCoords, 0x00, sizeof(struct sockaddr_in));
    serverCoords.sin_family = PF_INET;
    // toutes les interfaces locales disponibles
    serverCoords.sin_addr.s_addr = htonl(INADDR_ANY);
    // toutes les interfaces locales disponibles
    serverCoords.sin_port = htons(port);
    if (bind(server->_fdWaitSocket, (struct sockaddr *)&serverCoords, sizeof(serverCoords)) == -1)
    {
        printf("bind error\n");
        free(server);
        return NULL;
    }
    if (listen(server->_fdWaitSocket, 32) == -1)
    {
        printf("listen error\n");
        free(server);
        return NULL;
    }
    server->_coordsSize = sizeof(server->_callerCoords);
    return server;
}
bool Server_run(Server server)
{
    //boucle principale du serveur
    while (true)
    {
        int fdCommunicationSocket;
        //si une connexion entrente d'un client est disponible
        if ((fdCommunicationSocket = accept(server->_fdWaitSocket, (struct sockaddr *)&server->_callerCoords, &server->_coordsSize)) == -1)
        {
            printf("accept error\n");
            close(server->_fdWaitSocket);
            return false;
        }
        else
        {
            //on alloue les coordonnées de connexion, qui seront libérées dans le thread client correspondant
            connection *con = (connection *)malloc(sizeof(connection));
            con->callerCoords = server->_callerCoords;
            con->fdSocket = fdCommunicationSocket;
            con->server = server;
            pthread_t my_thread1;
            //lancement du thread client
            pthread_create(&my_thread1, NULL, clientThread, con);
        }
    }
    //aucune raison d'arriver ici, mais bon ¯\_(ツ)_/¯
    close(server->_fdWaitSocket);
    return false;
}
void Server_onConnectedClient(Server server, void (*fct)(Client))
{
    //on attribue l'évènement
    server->_onConnect = fct;
}
void Server_onDisconnectedClient(Server server, void (*fct)(Client))
{
    //on attribue l'évènement
    server->_onDisconnect = fct;
}
void Server_onMessageRecieved(Server server, size_t (*fct)(Client, void *, size_t, void *))
{
    //on attribue l'évènement
    server->_onMessage = fct;
}
