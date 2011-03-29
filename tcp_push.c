#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <errno.h>
#include <sys/epoll.h>
#include <unistd.h>

#include "serv.h"
#include "utils.h"

#include "tcp_push.h"
#include "tcp_utils.h"

#define MAX_FORK 100


void read_init_tcp_push(int sock, int * id, int * port_c)
{
  int i, fini = 0, b_get = 1;
  char buff;
  char * get = "GET ";

  *id = 0;
  *port_c = 0;
  
  /* on identifie la commande GET: peu permissif */
  for(i = 0; i < 4; i++)
  {
    do{}
    while(recv(sock, &buff, 1, 0) != 1);
      
    printf(":%c", buff);
    if (buff != get[i])
      b_get = 0;
  }

  if(b_get) /* le client veux une image ID */
  {
    fini = 0;
    while(!fini) /* récupération de l'ID */
    {
      do{}
      while(recv(sock, &buff, 1, 0) != 1);
      if (buff == ' ' || buff == '\n' || buff == '\r')
	fini = 1;
      else
	*id = (*id * 10) + atoi(&buff);
    }
        
    fini = 0;
    while(!fini) /* on ignore les caratère jusqu'au prochain space: d'après le protocole il est situé après LISTEN_PORT */
      {
	do{}
	while(recv(sock, &buff, 1, 0) != 1);
	if (buff == ' ')
	  fini = 1;
      }
    
    fini = 0;
    while(!fini) /* on récupère la valeur de "port_c" */
    {
      do{}
      while(recv(sock, &buff, 1, 0) != 1);
      if (buff == ' ' || buff == '\n' || buff == '\r')
	fini = 1;
      else
	*port_c = (*port_c * 10) + atoi(&buff);
    }
    int nb_nl = 0;
    while(nb_nl < 2) /* on attend la fin de la commande */
      {
	do{}
	while(recv(sock, &buff, 1, 0) != 1);
	
	if (buff == '\n')
	  nb_nl++;
      }
  }
  else
    {
      close(sock);
    }
}

void read_commande_B(int sock, char* commande) /* version bloquante */
{
  char buff;
  int fini=0;
  /* Determination de la commande : resultat indeterminé si le protocole n'est pas respecté */
  while(!fini)
  {
    do{}
    while(recv(sock, &buff, 1, 0) != 1);
    if (buff == 'S')
      {
        *commande = 'S';
	fini=1;
      }
    else if (buff == 'P')
      {
	*commande = 'P';
	fini=1;
      }
    else if (buff == 'E')
      {
	*commande = 'E';
	fini=1;
      }
  }

  /* recuperer deux CRLF */
  int nb_nl = 0;
  while(nb_nl < 2)
    {
      do{}
      while(recv(sock, &buff, 1, 0) != 1);
      
      if (buff == '\n')
	nb_nl++;
    }
  
}

void read_commande_NB(int sock, char* commande) /* version non bloquante */
{
  char buff ='0';
  int fini = 0;
  
  /* Determination de la commande */
  recv(sock, &buff, 1, MSG_DONTWAIT);
  if (buff == 'P')
    {
      *commande = 'P';
      fini=1;
    }
  else if (buff == 'E')
    {
      *commande = 'E';
      fini=1;
    }
    
  if(fini==1)
    {
      /* recuperer deux CRLF */
      int nb_nl = 0;
      while(nb_nl < 2)
      {
	recv(sock, &buff, 1, MSG_DONTWAIT);
    
	if (buff == '\n')
	  nb_nl++;
      }
    }
}

void instance_tcp_push(int csock, int dsock)
{
  char commande='0';
  int numImage=1;

  read_commande_B(csock, &commande);
 
 int nombre_image =  get_nombre_image(); 

  if (commande == 'E')
  {
    close(csock);
  }
  else 
  {
    while (commande == 'S')
    {
      commande = '0';

      while(commande == '0' )
      {
	printf("Preparing to send image %d\n", numImage);
	send_image_tcp(dsock, numImage);
	sleep(1);
	printf("Sent image %d\n", numImage);
	read_commande_NB(csock, &commande);
	printf("Commande %c\n", commande);
	numImage++;
	if (numImage >= nombre_image)
	  numImage = 1;
      }

      while(commande == 'P' )
      {
	read_commande_B(csock, &commande);
      }

      if (commande == 'E')
      {
	close(csock);
	puts("closed socket");
      }
    }  
  }
}


void tcp_push(int port, char * file)
{
  /* Creation d'un hander de signal */
  
  printf("Dans process client\n");

  int sock = mk_sock(port, INADDR_ANY, SOCK_STREAM);

  /* Creation des instances à chaque nouveau client */
  pid_t pid_instances[MAX_FORK];
  for(;;)
  {
    int c_port;
    struct sockaddr_in saddr_client;
    socklen_t size_addr = sizeof(struct sockaddr_in);
    int un_csock=-1;

    un_csock = accept(sock, (struct sockaddr *)&saddr_client, &size_addr);
    perror("accept");

    printf("Connection de %s :: %d\n", inet_ntoa(saddr_client.sin_addr), 
	   htons(saddr_client.sin_port));    

    struct sockaddr_in addr;
    int id, len;
    read_init_tcp_push(un_csock, &id, &c_port);
    getsockname(un_csock, (struct sockaddr*)&addr, &len);
    int data_sock = connect_to(inet_ntoa(addr.sin_addr), c_port, SOCK_STREAM);

    if(fork() == 0)
      instance_tcp_push(un_csock, data_sock);
  }
}

