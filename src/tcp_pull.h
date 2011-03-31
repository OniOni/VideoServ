
#ifndef TCP_PULL
#define TCP_PULL

void read_init(int sock, int * id, int * port_c);

int send_image(int sock, int image);

void tcp_pull(int port, char * file);

#endif

