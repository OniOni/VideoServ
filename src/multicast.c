#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <errno.h>

#include "serv.h"
#include "utils.h"
#include "net_utils.h"
#include "udp_utils.h"

#include "multicast.h"

void multicast(int port, char * file, int tempo)
{
  struct sockaddr_in addr;
  int sock, num_image = 1, tot_image;
  
  tot_image = get_nombre_image(file);
  
  if ((sock=socket(AF_INET,SOCK_DGRAM,0)) < 0) {
    perror("socket");
    exit(1);
  }
  
  /* set up destination address */
  memset(&addr,0,sizeof(addr));
  addr.sin_family=AF_INET;
  addr.sin_addr.s_addr=inet_addr(MULTI_ADDR);
  addr.sin_port=htons(port);
  
  for (;;) {
    if (num_image > tot_image)
      num_image = 1;
	
    send_image_udp(sock, addr, file, num_image++, MULTI_FRAG);
    usleep(tempo);
  }
}

  
