#ifndef UTILS
#define UTILS

void prepare_sock(int port, int addr, int * sock, struct sockaddr_in * saddr, int);

int mk_sock(int port, int addr, int);

char * build_date();

char * build_http_header(char * type, int size);

void file_to_buffer(char * nomFic, char ** buff, int * size);

int connect_to(int addr, int c_port, int);

#endif
