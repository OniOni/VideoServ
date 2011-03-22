#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <errno.h>
#include <sys/epoll.h>

#include "serv.h"

#include "utils.h"

void prepare_sock(int port, char * addr, int * sock, struct sockaddr_in * saddr)
{
  *sock = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
  printf("socket : %s\n", strerror(errno));
  
  saddr->sin_addr.s_addr = inet_addr(addr);
  saddr->sin_family = AF_INET;
  saddr->sin_port = htons(port);
}


int mk_sock(int port, char * addr)
{
  int sock;
  struct sockaddr_in saddr;

  prepare_sock(port, addr, &sock, &saddr);

  bind(sock, (struct sockaddr *)&saddr, sizeof(saddr));
  printf("bind : %s\n", strerror(errno));

  listen(sock, 10);
  printf("listen : %s\n", strerror(errno));
  
  return sock;
}


char * build_date()
{
  return "00 / 00 / 00";
}

char * build_http_header(char * type, int size)
{
  char * header = malloc(MAX_HEADER * sizeof(char));
  memset(header, '\0', MAX_HEADER);
  strcat(header, "HTTP/1.1 200 OK\nDate: ");
  
  /*Insertion de la date actuelle*/
  strcat(header, build_date());

  strcat(header, "\r\nServer: ServLib (Unix) (Ubuntu/Linux)\r\nAccept-Ranges: bytes\r\n\
Content-Length: ");

  /*Insertion de la taille*/
  char tmp[MAX_STR] = {'\0'};
  sprintf(tmp, "%d", size);
  strcat(header, tmp);

  strcat(header, "\r\nConnection: close\r\nContent-Type: ");

  /*Insertion du type*/
  strcat(header, type);

  strcat(header, "; charset=UTF-8\r\n\r\n");

  return header;
}

void file_to_buffer(char * nomFic, char ** buff, int * size)
{
  /*Ouverture du fichier*/
  FILE * f = fopen(nomFic, "r");
  printf("fopen : %s\n", strerror(errno));

  /*Deplacement du curseur à la fin du fichier*/
  fseek(f, 0,SEEK_END);
  printf("fseek : %s\n", strerror(errno));

  /*On recupere le nombre de caractere*/
  *size = ftell(f);
  printf("fopen : %s\n", strerror(errno));

  int c;
  int i;
  /*On alloue un espace memoire de la taille du fichier*/
  *buff = malloc(*size * sizeof(char));

  /*On se replace au debut du fichier*/
  fseek(f, 0, SEEK_SET);
  printf("fseek : %s\n", strerror(errno));

  /*On recupere tout les caracteres*/
  for(i = 0; i < *size; i++)
  {
    c = fgetc(f);
    (*buff)[i] = c;
  }

  /*Et on referme le fichier*/
  fclose(f);
}


int connect_to(char * addr, int c_port)
{
  int sock;
  struct sockaddr_in saddr;
  prepare_sock(c_port, addr, &sock, &saddr);

  connect(sock, (struct sockaddr*)&saddr, sizeof(saddr));
  
  return sock;
}

