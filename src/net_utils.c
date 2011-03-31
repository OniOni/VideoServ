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

    printf("::%c(%d)", buff, i);

    if (i < 3 && buff != end[i])
      b_end = 0;

    i++;
  }

  printf("\n\nend : %d\n", b_end);
  
  if (b_end)
  {
    puts("received end");
    *id = -42;
    return;
  }
  
  /*for(i = 0; i < 4; i++)
  {
    recv(sock, &buff, 1, 0);
    if (buff != get[i])
      cont = 0;
      }*/
  fini = 0;
  while(!fini)
    {
      do{}
      while(recv(sock, &buff, 1, 0) != 1);
      if (buff == ' ' || buff == '\n' || buff == '\r')
	fini = 1;
      else
      {
	//*id = (*id * 10) + atoi(&buff); 	
	sprintf(buff_num, "%s%c", buff_num, buff);
	puts(buff_num);
      }
    }

  *id = atoi(buff_num);

  int nb_nl = 0;
  while(nb_nl < 2)
  {
    do{}
    while(recv(sock, &buff, 1, 0) != 1);
    
    if (buff == '\n')
      nb_nl++;
  }
}
