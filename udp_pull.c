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

#include "udp_pull.h"

struct udp_info{
  int num_image;
  int num_frag;
  int data_socket;
  int start;
};

int get_fragment(char image[], int len, int start, int end, char ** frag)
{
  int size = end - start, i;
  *frag = malloc(size * sizeof(char));
  int read = 0;

  for (i = 0; i < size; i++)
  {
    if (start + i < len)
    {
      (*frag)[i] = image[start + i];
      read++;
    }
  }

  return read;
}

int send_image_udp(int sock, int image, int frag_size)
{
  int len_ima, len, sent, read, start, pos_pack;
  char str[12], *buff_ima;
  sprintf(str, "%d.jpg", image);

  file_to_buffer(str, &buff_ima, &len_ima);

  char * buff;

  pos_pack = 0;
  do
  {
    //get next fragment
    read = get_fragment(buff_ima, len_ima, start, start+frag_size, &buff);
    
    //build "header"
    sprintf(buff, "%d\r\n%d\r\n%d\r\n", image, len_ima, pos_pack, read);
    perror("sprintf");
      
    sent = len = strlen(buff);

    //send "header"
    puts("Going to send image\n");
    do{
      sent -= send(sock, buff, len, 0);
    }
    while(sent > 0);
    
    sent = len = len_ima;
    //send fragment
    do{
      sent -= send(sock, buff, read, 0);
    }
    while(sent > 0);

    start += read;
    pos_pack += 1;
  }
  while(read == frag_size);
}

/*void udp_pull(int port, char * file)
{
  int sock = mk_sock(port, "127.0.0.1", SOCK_DGRAM | SOCK_NONBLOCK);

  int id, c_port;
  struct epoll_event ev, events[MAX_EVENTS];
  int nfds, epollfd;
  struct sockaddr_in saddr_client;

  struct udp_info connected_clients[1024];

  //TODO : Write get_nombre_image

  int nombre_image = 6;
  
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
	    getsockname(events[n].data.fd, (struct sockaddr*)&addr, &len); 
	    connected_clients[events[n].data.fd].data_socket = 
	      connect_to(inet_ntoa(addr.sin_addr), c_port);
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
	      send_image_tcp(connected_clients[events[n].data.fd].data_socket, id);
	      connected_clients[events[n].data.fd].num_image = id;
	    }
	    else if (id == -1)
	    {
	      id = connected_clients[events[n].data.fd].num_image + 1;
	      id = (id % (nombre_image + 1));
	      if (id == 0) id = 1;
	      printf("id %d\n", id);
	      send_image_tcp(connected_clients[events[n].data.fd].data_socket, id);
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
}*/

