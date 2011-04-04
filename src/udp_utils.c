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

int get_fragment(char image[], int len, int start, int size, char ** frag, char ** all)
{
  int i = 0, last = 0;
  *frag = malloc(size * sizeof(char));
  int read = 0;  

  while ( (i < size) && (start + i < len) )
  {    
    (*frag)[i] = image[start + i];
    (*all)[start + i] = image[start + i];
    last = start + i;
    read++;
    i++;
  }
   
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
  
}

int send_image_udp(int sock, struct sockaddr_in dest, char * rep, int image, int frag_size)
{
  int len_ima = 0, len, sent = 0, read, start = 0, pos_pack = 0;
  char str[32], *buff_ima, header[32];
  errno = 0;
  sprintf(str, "%s/%d.jpg", rep, image);

  frag_size -= 100;

  file_to_buffer(str, &buff_ima, &len_ima);
  
  int num_frag = len_ima / frag_size;

  char * buff, * buff_fin = malloc((len_ima + 20) * sizeof(char));

  pos_pack = 0;
  do
  {
    //get next fragment
    read = get_fragment(buff_ima, len_ima, start, frag_size, &buff, &buff_fin);
    
    //build "header"
    sprintf(header, "%d\r\n%d\r\n%d\r\n%d\r\n", image, len_ima, start, read);
        
    len = strlen(header);

    //send "header"
    sendto(sock, header, len, MSG_MORE, (struct sockaddr*)&dest, sizeof(dest));
    
    //send fragment
    sent += sendto(sock, buff, read, 0, (struct sockaddr*)&dest, sizeof(dest));;      

    start += read;
    pos_pack += 1;
  }
  while(sent < len_ima);

  free(buff_ima);
  perror("free");
  free(buff);
  perror("free");
  free(buff_fin);
  perror("free");
}

