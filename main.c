#include <stdlib.h>
#include <stdio.h>

#include "serv.h"


int main(int argc, char ** argv)
{
  pid_t p_central = fork();
  if (p_central == 0)
    central(8080);
    
  pid_t p_tcp_pull = fork();
  if (p_tcp_pull == 0)
    tcp_pull(8083, NULL);

  wait(-1);
  wait(-1);
  return 0;
}
