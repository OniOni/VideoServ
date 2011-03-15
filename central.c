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

#include "central.h"

void send_get_answer(int fd)
{
  int size;
  char * buf = NULL;
  /*On recuper le fichier sous forme de chaine de cara*/
  file_to_buffer("catalogue.txt", &buf, &size);
  /*On construit l'entete HTML aproprié*/
  char * header = build_http_header("text/plain", size);

  /*Et on envoie les données*/
  puts("Going to send");
  send(fd, header, strlen(header), MSG_MORE);
  printf("send : %s\n", strerror(errno));
  send(fd, buf, size, 0);
  printf("send : %s\n", strerror(errno));

  /*On libere les ressources alloué*/
  free(header);
  free(buf);
}

void central(int port)
{
  struct epoll_event ev, events[MAX_EVENTS];
  int nfds, epollfd;
  struct sockaddr_in saddr_client;
  
  int sock = mk_sock(port, "127.0.0.1");
  
  epollfd = epoll_create(10);
  if (epollfd == -1) {
    perror("epoll_create");
    exit(EXIT_FAILURE);
  }
  
  ev.events = EPOLLIN;
  ev.data.fd = sock;
  if (epoll_ctl(epollfd, EPOLL_CTL_ADD, sock, &ev) == -1) {
    perror("epoll_ctl: sock");
    exit(EXIT_FAILURE);
  }

  for (;;) {
    nfds = epoll_wait(epollfd, events, MAX_EVENTS, -1);
    puts("working ...");
    if (nfds == -1) {
      perror("epoll_pwait");
      exit(EXIT_FAILURE);
    }
    int n;
    for (n = 0; n < nfds; ++n) {
      if (events[n].data.fd == sock) {
	socklen_t size_addr = sizeof(struct sockaddr_in);
	int csock = accept(sock, (struct sockaddr *)&saddr_client, &size_addr);
	printf("accept : %s\n", strerror(errno));

	printf("Connection de %s :: %d\n", inet_ntoa(saddr_client.sin_addr), 
	       htons(saddr_client.sin_port));
	if (csock == -1) {
	  perror("accept");
	  exit(EXIT_FAILURE);
	}
	/*setnonblocking(csock);*/
	ev.events = EPOLLIN | EPOLLET;
	ev.data.fd = csock;
	if (epoll_ctl(epollfd, EPOLL_CTL_ADD, csock,
		      &ev) == -1) {
	  perror("epoll_ctl: csock");
	  exit(EXIT_FAILURE);
	}
      } 
      else{
	printf("%d\n", events[n].events == 1);
	if (events[n].events == 1)
	  send_get_answer(events[n].data.fd);
      }
    }
  }
}
