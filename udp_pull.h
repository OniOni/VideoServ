#ifndef UPD_PULL
#define UDP_PULL

int get_fragment(char image[], int len, int start, int end, char ** frag);

int send_image_udp(int sock, int image, int frag_size);

#endif
