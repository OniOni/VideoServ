#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <fcntl.h>
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

  strcat(header, "\nServer: ServLib (Unix) (Ubuntu/Linux)\nAccept-Ranges: bytes\n\
Content-Length: ");

  /*Insertion de la taille*/
  char tmp[MAX_STR] = {'\0'};
  sprintf(tmp, "%d", size);
  strcat(header, tmp);

  strcat(header, "\nConnection: close\nContent-Type: ");

  /*Insertion du type*/
  strcat(header, type);

  strcat(header, "; charset=UTF-8\n\n");

  return header;
}

void file_to_buffer(char * nomFic, char ** buff, int * size)
{
  /*Ouverture du fichier*/
  int fd;
  FILE * f;
  struct epoll_event ev, events[MAX_EVENTS];
  int nfds, epollfd;    
   
  fd = open(nomFic, O_RDONLY);
  if (fd == -1)
    perror("open");
  
  if (fd == -1 && errno == EINPROGRESS)
  {
    printf("fd: %d\n", fd);

    epollfd = epoll_create(10);
    if (epollfd == -1) {
      perror("epoll_create");
      exit(EXIT_FAILURE);
    }
    
    ev.events = EPOLLOUT;
    ev.data.fd = fd;
    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &ev) == -1) {
      perror("epoll_ctl: fd");
      exit(EXIT_FAILURE);
    }
    
    nfds = epoll_wait(epollfd, events, MAX_EVENTS, -1);
    puts("working ...");
    if (nfds == -1) {
      perror("epoll_pwait");
      exit(EXIT_FAILURE);
    }
  }

  f = fdopen(fd, "r");
  if (f == NULL)
    perror("fdopen");

  /*Deplacement du curseur à la fin du fichier*/
  if (fseek(f, 0,SEEK_END) == -1)
    perror("fseek");

  /*On recupere le nombre de caractere*/
  if ((*size = ftell(f)) == -1)
    perror("ftell");

  int c;
  int i;
  /*On alloue un espace memoire de la taille du fichier*/
  *buff = malloc(*size * sizeof(char));

  /*On se replace au debut du fichier*/
  if (fseek(f, 0, SEEK_SET) == -1)
    perror("fseek");

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

