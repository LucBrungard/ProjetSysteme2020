#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h> 

typedef struct
{
  struct sockaddr_in coordonneesAppelant;
  int fdSocketCommunication;
} var_struc;

void *thread(void *arg) {
  char tampon[100];
  var_struc *temp = (var_struc *)arg;

  printf("Client connecté : %s\n", inet_ntoa(temp->coordonneesAppelant.sin_addr)); //affiche l'adresse ip du client
  while(true) {
    nbRecu=recv(temp->fdSocketCommunication,tampon,99,0);
    if(nbRecu>0) {
      tampon[nbRecu]=0;
      printf("Recu:%s\n",tampon);
    }  
    fgets(tampon, 100, stdin);
    send(temp->fdSocketCommunication,tampon,strlen(tampon),0);
  } 

  close(temp->fdSocketCommunication);
  printf( "\n" );
}


#define PORT 6000
int main()
{
  int fdSocketAttente;
  int fdSocketCommunication;
  struct sockaddr_in coordonneesServeur;
  struct sockaddr_in coordonneesAppelant;
  char tampon[100];
  int nbRecu;
  int longueurAdresse;
  
  fdSocketAttente = socket(PF_INET, SOCK_STREAM, 0);
  if(fdSocketAttente < 0)
  {
    printf("socket incorrecte\n");
    exit(-1);
  }

  // On prépare l’adresse d’attachement locale
  longueurAdresse = sizeof(struct sockaddr_in);
  memset(&coordonneesServeur, 0x00, longueurAdresse);
  coordonneesServeur.sin_family = PF_INET;
  // toutes les interfaces locales disponibles
  coordonneesServeur.sin_addr.s_addr = htonl(INADDR_ANY);
  // toutes les interfaces locales disponibles
  coordonneesServeur.sin_port = htons(PORT);

  if(bind(fdSocketAttente,(struct sockaddr*)&coordonneesServeur,sizeof(coordonneesServeur) ) == -1)
  {
    printf("erreur de bind\n");
    exit(-1);
  }
  if(listen(fdSocketAttente,5) == -1)
  {
    printf("erreur de listen\n");
    exit(-1);
  }

  socklen_t tailleCoord=sizeof(coordonneesAppelant);

  while(true){ 
    printf("En attente d'une connexion :\n ");
    
    if((fdSocketCommunication=accept(fdSocketAttente,(struct sockaddr*)&coordonneesAppelant,&tailleCoord)) == -1)
      printf("erreur de accept\n");
    else {
      var_struc t;
      t.coordonneesAppelant = coordonneesAppelant;
      t.fdSocketCommunication = fdSocketCommunication;

      pthread_t my_thread1;
      int ret1 = pthread_create(&my_thread1, NULL, thread, (void *)&t);
      pthread_join(my_thread1, NULL);
    }
    
  } 

  close(fdSocketAttente);

  return 0;
}






