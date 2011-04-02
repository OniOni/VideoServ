#include <stdlib.h>
#include <stdio.h>

#include "serv.h"


int main(int argc, char ** argv)
{
  puts("central");
  pid_t p_central = fork();
  if (p_central == 0)
    central(8080);

  puts("tcp_pull");    
  pid_t p_tcp_pull = fork();
  if (p_tcp_pull == 0)
    tcp_pull(8083, "super");

  puts("udp_pull");
  pid_t p_udp_pull = fork();
  if (p_udp_pull == 0)
    udp_pull(8084, "super");

  puts("tcp_push");
  pid_t p_tcp_push = fork();
  if (p_tcp_push == 0)
    tcp_push(8086, "super");

  puts("udp_push");
  pid_t p_udp_push = fork();
  if (p_udp_push == 0)
    udp_push(8087, "Doctor");

  wait(-1);
  wait(-1);
  wait(-1);
  wait(-1);
  wait(-1);

  return 0;
}
