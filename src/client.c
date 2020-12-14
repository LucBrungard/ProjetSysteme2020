#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#define PORT 6000
int main()
{
int fdSocket;
int nbRecu;
struct sockaddr_in coordonneesServeur;
int longueurAdresse;
char tampon[100];
fdSocket = socket(AF_INET, SOCK_STREAM, 0);
if(fdSocket < 0)
  {
    printf("socket incorrecte\n");
    exit(-1);
  }


// On prépare les coordonnées du serveur
longueurAdresse = sizeof(struct sockaddr_in);
memset(&coordonneesServeur, 0x00, longueurAdresse);
coordonneesServeur.sin_family = PF_INET;
// adresse du serveur
inet_aton("127.0.0.1", &coordonneesServeur.sin_addr);
  // toutes les interfaces locales disponibles
coordonneesServeur.sin_port = htons(PORT);
if(connect(fdSocket,(struct sockaddr*)&coordonneesServeur,sizeof(coordonneesServeur))== -1)
  {
  printf("connexion impossible\n");
  exit(-1);
  }
printf("Connexion ok\n");

for (int i = 0; i<3; i++){
  fgets(tampon, 100, stdin);
  send(fdSocket,tampon,strlen(tampon),0);
  nbRecu=recv(fdSocket,tampon,99,0);
  if(nbRecu>0) {
    tampon[nbRecu]=0;
    printf("Recu:%s\n",tampon);
  } 
} 

close(fdSocket);
}
