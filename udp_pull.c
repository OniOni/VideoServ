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
