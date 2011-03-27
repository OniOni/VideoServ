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

#include "udp_pull.h"

struct udp_info{
  int data_sock;
  int num_image;
  int frag_size;
  int listen_port;
  struct in_addr ip;
  int start;
};

GHFunc print_key_value(gpointer key, gpointer value, gpointer u_data)
{
  printf("%s:%s\n", (char*)key, (char*)value);
}

int get_fragment(char image[], int len, int start, int end, char ** frag)
{
  int size = end - start, i;
  //printf("Size : %d :: start : %d :: Len : %d\n", size, start, len);
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

  //printf("Read : %d\n", read);
  return read;
}

void read_init_udp(int sock, int * id, int * port_c, int * frag_size)
{
  int i, fini = 0, b_get = 1, b_end = 1, len;
  char buff[512], buff_pre;
  char * get = "GET ", * end = "END ";
  struct sockaddr_in addr;

  *id = 0;
  *port_c = 0;
  *frag_size = 0;

  recvfrom(sock, buff, 512, 0, (struct sockaddr*)&addr, &len);
  perror("recvfrom");
  
  sscanf(buff, "GET %d\r\nLISTEN_PORT %d\r\nFRAGMENT_SIZE %d\r\n", 
	 id, port_c, frag_size);
  perror("sscanf");
  
  /*printf("Got :%s: from %s::%d\n", buff, inet_ntoa(addr.sin_addr), 
    htons(addr.sin_port));*/
}

void read_get_udp(int sock, int * id)
{
  int len;
  char buff[512];
  struct sockaddr_in addr;

  *id = 0;

  recvfrom(sock, buff, 512, 0, (struct sockaddr*)&addr, &len);
  perror("recvfrom");
  
  sscanf(buff, "GET %d\r\n", 
	 id);
  perror("sscanf");
  
  /*printf("Got :%s: from %s::%d\n", buff, inet_ntoa(addr.sin_addr), 
    htons(addr.sin_port));*/
}

int send_image_udp(int sock, struct sockaddr_in dest, int image, int frag_size)
{
  int len_ima, len, sent, read, start = 0, pos_pack = 0;
  char str[12], *buff_ima;
  errno = 0;
  sprintf(str, "%d.jpg", image);
  puts(str);

  file_to_buffer(str, &buff_ima, &len_ima);
  
  sprintf(str, "dump%d.jpg", image);
  FILE * f = fopen(str, "a");
  fwrite(buff_ima, sizeof(char), len, f);
  fclose(f);  

  int num_frag = len_ima / frag_size;

  printf("Image size : %d, Number of fragments %d\n", len_ima, num_frag);

  char * buff;

  pos_pack = 0;
  do
  {
    //get next fragment
    read = get_fragment(buff_ima, len_ima, start, start+frag_size, &buff);
    
    //build "header"
    sprintf(buff, "%d\r\n%d\r\n%d\r\n", image, len_ima, pos_pack, read);
    //perror("sprintf");
      
    sent = len = strlen(buff);

    //send "header"
    //puts("Going to send image\n");

    sendto(sock, buff, len, 0, (struct sockaddr*)&dest, sizeof(dest));
    
    sent = len = len_ima;
    //send fragment
    //printf("Sending %d/%d\n", read, frag_size);
    sendto(sock, buff, read, 0, (struct sockaddr*)&dest, sizeof(dest));;      

    //printf("fragment n%d/%d\n", pos_pack, num_frag);
    start += read;
    pos_pack += 1;
  }
  while(pos_pack <= num_frag);
  
  puts("Image sent");
}


void udp_pull(int port, char * file)
{
  GHashTable * clients = g_hash_table_new(g_str_hash, g_str_equal);

  puts("in udp_pull");
  int sock = mk_sock_udp(port, "127.0.0.1");

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
      int len;
      char buff;
      
      if (events[n].data.fd == sock) {
	recvfrom(events[n].data.fd, &buff, 1, MSG_PEEK, (struct sockaddr*)&addr, &len);
	perror("recvfrom (peek)");
	printf("Connection de %s :: %d\n", inet_ntoa(addr.sin_addr), 
	       htons(addr.sin_port));
	/*putchar(buff);
	printf("\nClients %d\n", g_hash_table_size(clients));
	printf("value %s : key %s\n", (char*)g_hash_table_lookup(clients, (gpointer)inet_ntoa(addr.sin_addr)),
	inet_ntoa(addr.sin_addr));*/
	
	if(g_hash_table_lookup(clients, (gpointer)inet_ntoa(addr.sin_addr)) == NULL)
	{
	  int id, port, frag_size;
	  puts("Initialisation of data_socket");
	  struct udp_info client_info;

	  read_init_udp(events[n].data.fd, &id, &port, &frag_size);
	  printf("id : %d, port : %d, frag_size : %d\n", id, port, frag_size);
	  
	  //g_hash_table_foreach(clients, (GHFunc)print_key_value, NULL);
	  /*struct udp_info{
	    int num_image;
	    int num_frag;
	    int frag_size;
	    int data_socket;
	    int start;
	    };*/

	  client_info.listen_port = port;
	  client_info.num_image = 0;
	  client_info.frag_size = frag_size;
	  client_info.ip = addr.sin_addr;
	  client_info.data_sock = socket(AF_INET, SOCK_DGRAM, 0);

	  g_hash_table_insert(clients, (gpointer)inet_ntoa(addr.sin_addr), (gpointer)&client_info);
	}
	else
	{
	  struct sockaddr_in dest;

	  struct udp_info * client_info = 
	    (struct udp_info*)g_hash_table_lookup(clients, 
						  (gpointer)inet_ntoa(addr.sin_addr));

	  dest.sin_family = AF_INET;
	  dest.sin_port = htons(client_info->listen_port);
	  printf("port :%d::", client_info->listen_port);
	  dest.sin_addr = client_info->ip;
	  printf("ip :%s\n", inet_ntoa(client_info->ip));
	  	  
	  read_get_udp(events[n].data.fd, &id);
	  printf("id : %d\n", id);

	  id = client_info->num_image + 1;
	  id = (id % (nombre_image + 1));

	  send_image_udp(client_info->data_sock, dest, id, client_info->frag_size);

	  client_info->num_image = id;
	  
	}
      }
    }
  }
}

