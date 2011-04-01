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

#include "udp_pull.h"


GHFunc print_key_value(gpointer key, gpointer value, gpointer u_data)
{
  /*printf("%s: (%d, %d, %d)\n", (char*)key, 
	 ((struct tcp_info*)value)->listen_port,
	 ((struct tcp_info*)value)->num_image,
	 ((struct tcp_info*)value)->frag_size,
	 );*/
}

void read_get_udp(int sock, int * id)
{
  int len = sizeof(struct sockaddr_in);
  char buff[512];
  struct sockaddr_in addr;

  *id = 0;

  recvfrom(sock, buff, 512, 0, (struct sockaddr*)&addr, &len);
  perror("recvfrom");
  
  sscanf(buff, "GET %d\r\n", id);
  perror("sscanf");
  
  /*printf("Got :%s: from %s::%d\n", buff, inet_ntoa(addr.sin_addr), 
    htons(addr.sin_port));*/
}

void udp_pull(int port, char * file)
{
  GHashTable * clients = g_hash_table_new(g_str_hash, g_str_equal);

  puts("in udp_pull");
  int sock = mk_sock_udp(port, INADDR_ANY);

  int id, c_port;
  struct epoll_event ev, events[MAX_EVENTS];
  int nfds, epollfd;
  struct sockaddr_in saddr_client;

  struct udp_info connected_clients[1024];

  int nombre_image = get_nombre_image(file);
  
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
	
	sprintf(key, "%s:%d", inet_ntoa(addr.sin_addr), htons(addr.sin_port));
	if(g_hash_table_lookup(clients, (gpointer)key) == NULL)
	{
	  int id, port, frag_size;
	  puts("Initialisation of data_socket");
	  struct udp_info * client_info = malloc(sizeof(struct udp_info));

	  read_init_udp(events[n].data.fd, &id, &port, &frag_size);
	  printf("id : %d, port : %d, frag_size : %d\n", id, port, frag_size);

	  client_info->listen_port = port;
	  client_info->num_image = 0;
	  client_info->frag_size = frag_size;
	  client_info->ip = addr.sin_addr;
	  client_info->data_sock = socket(AF_INET, SOCK_DGRAM, 0);

	  g_hash_table_insert(clients, (gpointer)key, (gpointer)client_info);
	  printf("%p\n", client_info);
	}
	else if (buff == 'G')
	{
	  struct sockaddr_in dest;

	  struct udp_info * client_info = 
	    (struct udp_info*)g_hash_table_lookup(clients, 
						  (gpointer)key);

	  dest.sin_family = AF_INET;
	  dest.sin_port = htons(client_info->listen_port);
	  dest.sin_addr = client_info->ip;
	  	  
	  read_get_udp(events[n].data.fd, &id);
	  printf("id : %d\n", id);

	  if (id == -1)
	    id = client_info->num_image + 1;

	  if (id > nombre_image)
	    id = 1;

	  printf("id : %d\n", id);
	  send_image_udp(client_info->data_sock, dest, id, client_info->frag_size);

	  client_info->num_image = id;
	  
	}
	else if (buff == 'E')
	{
	  puts("Close connection");
	  recvfrom(events[n].data.fd, &buff, 1, 0, (struct sockaddr*)&addr, &len);

	  struct udp_info * client_info = 
	    (struct udp_info*)g_hash_table_lookup(clients, 
						  (gpointer)key);

	  close(client_info->data_sock);

	  g_hash_table_remove(clients, inet_ntoa(addr.sin_addr));
	  free(client_info);
	  perror("free");
	}
      }
    }
  }
}

