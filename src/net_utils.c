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

void read_get(int sock, int * id)
{
  int i = 0, fini = 0, b_end = 1;
  char buff, buff_pre, buff_num[32] = {'\0'};
  char * get = "GET ", * end = "END ";

  *id = 0;

  while (!fini)
  {
    do{}
    while(recv(sock, &buff, 1, 0) != 1);

    if (buff == ' ' || buff == 'E' || buff == 'G')
      fini = 1;

    if (buff == 'E' || buff == 'G')
    {
      i = 1;
      putchar(buff);
    }
  }

  fini = 0;
  while (!fini)
  {   
    do{}
    while(recv(sock, &buff, 1, 0) != 1);
      
    if (buff == ' ' || buff == '\n' || buff == '\r')
      fini = 1;

    if (i < 3 && buff != end[i])
      b_end = 0;

    i++;
  }

  if (b_end)
  {
    *id = -42;
    return;
  }
  
  fini = 0;
  while(!fini)
    {
      do{}
      while(recv(sock, &buff, 1, 0) != 1);
      if (buff == ' ' || buff == '\n' || buff == '\r')
	fini = 1;
      else
      {
	sprintf(buff_num, "%s%c", buff_num, buff);
      }
    }

  *id = atoi(buff_num);

  int nb_nl = 0;
  while(nb_nl < 2)
  {
    recv(sock, &buff, 1, 0);
    
    if (buff == '\n')
      nb_nl++;
  }
}
