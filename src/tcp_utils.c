#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <errno.h>
#include <sys/epoll.h>
#include <unistd.h>

#include "serv.h"
#include "utils.h"

#include "tcp_utils.h"


int send_image_tcp(int sock, int image, char * rep)
{
  int len, len_buff, sent;
  char str[32], *buff_ima;
  sprintf(str, "%s/%d.jpg", rep, image);

  file_to_buffer(str, &buff_ima, &len);
  errno = 0;

  char buff[100];

  sprintf(buff, "%d\r\n%d\r\n", image, len);

  sent = len_buff = strlen(buff);

  do{
    sent -= send(sock, buff, len_buff, MSG_MORE);
  }
  while(sent > 0);

  sent = len;
  sent = send(sock, buff_ima, len, 0);

  free(buff_ima);
  perror("free");
}

