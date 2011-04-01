#ifndef CENTRAL
#define CENTRAL

enum protocol {TCP_PULL, TCP_PUSH, UDP_PULL, UDP_PUSH};

struct action
{
  int port;
  enum protocol proto;
  char * file;
};
  
enum protocol conv_proto(char * proto);

void parse_catalogue(char * catalogue, struct action * acts, int * len);

void send_get_answer(int fd);

void central(int port);

#endif
