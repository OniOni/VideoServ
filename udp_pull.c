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

void read_init_udp(int sock, int * id, int * port_c, int * frag_size)
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
    //receive ID
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
  
    //Receive listening port
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

    fini = 0;
    while(!fini)
    {
      do{}
      while(recv(sock, &buff, 1, 0) != 1);
      if (buff == ' ' || buff == '\n' || buff == '\r')
	fini = 1;
      else
	*frag_size = (*frag_size * 10) + atoi(&buff); 	
    }
    printf("frag_size : %d\n", *port_c);
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
  int sock = mk_sock_udp(port, "127.0.0.1");

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
	
	    printf("Sent image %d\n", connected_clients[events[n].data.fd].num_image);
	  }
	}
      }
    }
  }
}*/

