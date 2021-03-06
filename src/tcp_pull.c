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
#include "net_utils.h"
#include "tcp_utils.h"

#include "tcp_pull.h"


struct tcp_info{
  int num_image;
  int data_socket;
  int start;
};

void init_tcp_info(struct tcp_info * tab[], int len)
{
  int i = 0;
  for(i = 0; i < len; i++)
  {
    (*tab)[i].num_image = 0;
    (*tab)[i].data_socket = -1;
    (*tab)[i].start = 0;
  }
}

void read_init(int sock, int * id, int * port_c)
{
  int i, fini = 0, b_get = 1, b_end = 1;
  char buff, buff_pre;
  char * get = "GET ", * end = "END ";


  *id = 0;
  *port_c = 0;

  for(i = 0; i < 4; i++)
  {
    do{}
    while(recv(sock, &buff, 1, 0) != 1);
      
    //printf(":%c", buff);
    if (buff != get[i])
      b_get = 0;
    if (buff != end[i])
      b_end = 0;
  }

  //printf("get : %d  end : %d\n", b_get, b_end);

  if(b_get)
  {
    fini = 0;
    while(!fini)
    {
      do{}
      while(recv(sock, &buff, 1, 0) != 1);
      if (buff == ' ' || buff == '\n' || buff == '\r')
	fini = 1;
      else
	*id = (*id * 10) + atoi(&buff); 	
    }
    printf("id %d\n", *id);
    
    fini = 0;
    while(!fini)
      {
	do{}
	while(recv(sock, &buff, 1, 0) != 1);
	if (buff == ' ')
	  fini = 1;
      }
    puts("espace\n");
  
    fini = 0;
    while(!fini)
    {
      do{}
      while(recv(sock, &buff, 1, 0) != 1);
      if (buff == ' ' || buff == '\n' || buff == '\r')
	fini = 1;
      else
	*port_c = (*port_c * 10) + atoi(&buff); 	
    }
    printf("port : %d\n", *port_c);    
  }
  else
  {
    close(sock);
  }
  int nb_nl = 0;
  while(nb_nl < 2)
  {
    do{}
    while(recv(sock, &buff, 1, 0) != 1);
    
    if (buff == '\n')
      nb_nl++;
  }
}


void tcp_pull(int port, char * file)
{
  int sock = mk_sock(port, INADDR_ANY, SOCK_STREAM);

  int id, c_port;
  struct epoll_event ev, events[MAX_EVENTS];
  int nfds, epollfd;
  struct sockaddr_in saddr_client;

  struct tcp_info connected_clients[1024];

  //TODO : Write get_nombre_image

  int nombre_image = get_nombre_image(file);
  
  //init_tcp_info((struct tcp_info**)&connected_clients, 1024);

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
	printf("envent type %d\n", events[n].events);
	if (events[n].events == 1)
	{
	  struct sockaddr_in addr;
	  int len;

	  if(connected_clients[events[n].data.fd].data_socket == 0)
	  {
	    puts("Initialisation of data_socket");
	    read_init(events[n].data.fd, &id, &c_port);
	    printf("%d::%d\n", id, c_port);
	    getpeername(events[n].data.fd, (struct sockaddr*)&addr, &len);
	    printf("Connection de %s :: %d\n", inet_ntoa(addr.sin_addr), 
	       htons(addr.sin_port));
	    connected_clients[events[n].data.fd].data_socket = 
	      connect_to(addr.sin_addr.s_addr, c_port, SOCK_STREAM);
	    connected_clients[events[n].data.fd].num_image = 0;
	  }
	  else
	  {
	    int id = 0;
	    printf("Getting image number\n");
	    read_get(events[n].data.fd, &id);
	    printf("Image number %d\n", id);
	    if (id > 0)
	    {	      
	      id = (id % nombre_image);
	      printf("id %d\n", id);
	      send_image_tcp(connected_clients[events[n].data.fd].data_socket, id, file);
	      connected_clients[events[n].data.fd].num_image = id;
	    }
	    else if (id == -1)
	    {
	      id = connected_clients[events[n].data.fd].num_image + 1;
	      id = (id % (nombre_image + 1));
	      if (id == 0) id = 1;
	      printf("id %d\n", id);
	      send_image_tcp(connected_clients[events[n].data.fd].data_socket, id, file);
	      connected_clients[events[n].data.fd].num_image = id;
	    }
	    else if (id == -42)
	    {
	      //do closing stuff here
	      close(connected_clients[events[n].data.fd].data_socket);
	      close(events[n].data.fd);
	      connected_clients[events[n].data.fd].data_socket = 0;
	      connected_clients[events[n].data.fd].num_image = 0;
	      puts("closed connection");
	    }
	    printf("Sent image %d\n", connected_clients[events[n].data.fd].num_image);
	  }
	}
      }
    }
  }
}
