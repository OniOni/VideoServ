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
  //printf("Size : %d :: start : %d :: Len : %d\n", size, start, len);
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
   
  //printf("First: %d\tLast: %d\n", start, last);

  /*if (read != size)
    printf("Read : %d\n", read);*/
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

int send_image_udp(int sock, struct sockaddr_in dest, int image, int frag_size)
{
  int len_ima = 0, len, sent = 0, read, start = 0, pos_pack = 0;
  char str[32], *buff_ima, header[32];
  errno = 0;
  sprintf(str, "%d.jpg", image);
  //puts(str);

  frag_size -= 100;

  file_to_buffer(str, &buff_ima, &len_ima);
  
  /*sprintf(str, "dump%d.jpg", image);
  FILE * f = fopen(str, "a");
  fwrite(buff_ima, sizeof(char), len_ima + 20, f);
  fclose(f);*/

  int num_frag = len_ima / frag_size;

  //printf("Image size : %d, Number of fragments %d\n", len_ima, num_frag);

  char * buff, * buff_fin = malloc((len_ima + 20) * sizeof(char));

  pos_pack = 0;
  do
  {
    //get next fragment
    read = get_fragment(buff_ima, len_ima, start, frag_size, &buff, &buff_fin);
    
    //build "header"
    sprintf(header, "%d\r\n%d\r\n%d\r\n%d\r\n", image, len_ima, start, read);
    //perror("sprintf");    
    //puts(str);

    //printf("Image :%d Taille :%d Début frag :%d Caractere lu%d Fin :%d\n", image, len_ima, start, read, start + read);
        
    len = strlen(header);

    //send "header"
    //puts("Going to send image\n");

    sendto(sock, header, len, MSG_MORE, (struct sockaddr*)&dest, sizeof(dest));
    

    //send fragment
    //printf("Sending %d/%d\n", read, frag_size);
    sent += sendto(sock, buff, read, 0, (struct sockaddr*)&dest, sizeof(dest));;      

    //printf("fragment n%d/%d\n", pos_pack, num_frag);
    start += read;
    pos_pack += 1;
  }
  while(sent < len_ima);

  /*FILE * f = fopen("all.jpg", "a");
  fwrite(buff_fin, sizeof(char), len_ima + 20, f);
  fclose(f);*/

  //printf("Sent : %d/%d\t%d fragments of %d\n", sent, len_ima, pos_pack, frag_size);
  //puts("Image sent");
}

