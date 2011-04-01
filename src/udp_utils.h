#ifndef UDP_UTILS
#define UDP_UTILS

struct udp_info{
  int data_sock;
  int num_image;
  int frag_size;
  int listen_port;
  struct in_addr ip;
  int start;
};

int get_fragment(char image[], int len, int start, int size, char ** frag, char ** all);

void read_init_udp(int sock, int * id, int * port_c, int * frag_size);

int send_image_udp(int sock, struct sockaddr_in dest, char *, int image, int frag_size);

#endif
