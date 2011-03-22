#ifndef UTILS
#define UTILS

void prepare_sock(int port, char * addr, int * sock, struct sockaddr_in * saddr);

int mk_sock(int port, char * addr);

char * build_date();

char * build_http_header(char * type, int size);

void file_to_buffer(char * nomFic, char ** buff, int * size);

int connect_to(char * addr, int c_port);

#endif
