#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <errno.h>
#include <sys/epoll.h>
#include <glib.h>

#include "serv.h"
#include "utils.h"
#include "net_utils.h"
#include "udp_utils.h"

#include "udp_push.h"

struct t_udp_push{
  struct udp_info udp;
  int pipe[2];
};

void udp_push_client(struct t_udp_push c, char * file, int tempo)
{
  struct epoll_event ev, events[MAX_EVENTS];
  int nfds, epollfd;
  char buff;
  struct sockaddr_in dest;

  dest.sin_family = AF_INET;
  dest.sin_port = htons(c.udp.listen_port);
  dest.sin_addr = c.udp.ip;

  int nombre_image = get_nombre_image(file);
  
  epollfd = epoll_create(10);
  if (epollfd == -1) {
    perror("epoll_create");
    exit(EXIT_FAILURE);
  }
  
  ev.events = EPOLLIN;
  ev.data.fd = c.pipe[0];
  if (epoll_ctl(epollfd, EPOLL_CTL_ADD, c.pipe[0], &ev) == -1) {
    perror("epoll_ctl: pipe[0]");
    exit(EXIT_FAILURE);
  }

  for (;;) {
    nfds = epoll_wait(epollfd, events, MAX_EVENTS, tempo);
    puts("working ...");
    if (nfds == -1) {
      perror("epoll_pwait");
      exit(EXIT_FAILURE);
    }
    int n;
    for (n = 0; n < nfds; ++n) {
      if (events[n].data.fd == c.pipe[0]) {
	read(c.pipe[0], &buff, 1);
	putchar(buff);
	if (buff == 'S')
	  c.udp.start = 1;
	else
	  c.udp.start = 0;
      }
    }
    if (!nfds /*==0*/){	  
      if (c.udp.start /*==1*/){
	c.udp.num_image += 1;	  
	if (c.udp.num_image > nombre_image || c.udp.num_image <= 0)
	  c.udp.num_image = 1;
	
	printf("Going to send image %d\n", c.udp.num_image);
	send_image_udp(c.udp.data_sock, dest, c.udp.num_image, c.udp.frag_size);
      }
    }
  }
}

void udp_push(int port, char * file)
{
  GHashTable * clients = g_hash_table_new(g_str_hash, g_str_equal);

  puts("in udp_push");
  int sock = mk_sock_udp(port, INADDR_ANY);

  int id, c_port;
  struct epoll_event ev, events[MAX_EVENTS];
  int nfds, epollfd;
  struct sockaddr_in saddr_client;

  struct udp_info connected_clients[1024];

  //TODO : Write get_nombre_image

  int nombre_image = 6;
  
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
      struct sockaddr_in addr;
      int len = sizeof(struct sockaddr_in);      
      char buff;
      char key[32];
      
      if (events[n].data.fd == sock) {
	errno = 0;
	recvfrom(events[n].data.fd, &buff, 1, MSG_PEEK, (struct sockaddr*)&addr, &len);
	perror("recvfrom (peek)");
	printf("Connection de %s :: %d\n", inet_ntoa(addr.sin_addr), 
	       htons(addr.sin_port));
	/*putchar(buff);
	printf("\nClients %d\n", g_hash_table_size(clients));
	printf("value %s : key %s\n", (char*)g_hash_table_lookup(clients, (gpointer)inet_ntoa(addr.sin_addr)),
	inet_ntoa(addr.sin_addr));*/
	
	sprintf(key, "%s:%d", inet_ntoa(addr.sin_addr), htons(addr.sin_port));
	if(g_hash_table_lookup(clients, (gpointer)key) == NULL)
	{
	  int id, port, frag_size;
	  puts("Initialisation of data_socket");
	  struct t_udp_push * client_info = malloc(sizeof(struct t_udp_push));

	  read_init_udp(events[n].data.fd, &id, &port, &frag_size);
	  printf("id : %d, port : %d, frag_size : %d\n", id, port, frag_size);
	  
	  //g_hash_table_foreach(clients, (GHFunc)print_key_value, NULL);

	  client_info->udp.listen_port = port;
	  client_info->udp.num_image = 0;
	  client_info->udp.frag_size = frag_size;
	  client_info->udp.ip = addr.sin_addr;
	  client_info->udp.data_sock = socket(AF_INET, SOCK_DGRAM, 0);
	  pipe(client_info->pipe);
	  perror("pipe");

	  g_hash_table_insert(clients, (gpointer)key, (gpointer)client_info);
	  printf("%p\n", client_info);

	  //TODO : Change tempo
	  if(fork() == 0)
	    udp_push_client(*client_info, file, 1000);
	}
	else if (buff == 'S' || buff == 'P')
	{
	  struct sockaddr_in dest;

	  //Clear buffer
	  recvfrom(events[n].data.fd, &buff, 1, 0, (struct sockaddr*)&addr, &len);

	  struct t_udp_push * client_info = 
	    (struct t_udp_push*)g_hash_table_lookup(clients, 
						  (gpointer)key);

	  write(client_info->pipe[1], &buff, 1);
	  perror("write");
	}
	else if (buff == 'E')
	{
	  puts("Close connection");	  
	  //Clear buffer
	  recvfrom(events[n].data.fd, &buff, 1, 0, (struct sockaddr*)&addr, &len);

	  struct t_udp_push * client_info = 
	    (struct t_udp_push*)g_hash_table_lookup(clients, 
						  (gpointer)key);

	  close(client_info->udp.data_sock);

	  g_hash_table_remove(clients, inet_ntoa(addr.sin_addr));
	}
	else {
	  recvfrom(events[n].data.fd, &buff, 1, 0, (struct sockaddr*)&addr, &len);
	  printf("Received %c\n", buff);
	}
      }
    }
  }
}


