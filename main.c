#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <errno.h>
#include <sys/epoll.h>

#define MAX_EVENTS 10
#define MAX_HEADER 200
#define MAX_STR 32
#define MAX_IMAGE 10*1024

char * build_date()
{
  return "00 / 00 / 00";
}

char * build_http_header(char * type, int size)
{
  char * header = malloc(MAX_HEADER * sizeof(char));
  memset(header, '\0', MAX_HEADER);
  strcat(header, "HTTP/1.1 200 OK\nDate: ");
  
  /*Insertion de la date actuelle*/
  strcat(header, build_date());

  strcat(header, "\nServer: ServLib (Unix) (Ubuntu/Linux)\nAccept-Ranges: bytes\n\
Content-Length: ");

  /*Insertion de la taille*/
  char tmp[MAX_STR] = {'\0'};
  sprintf(tmp, "%d", size);
  strcat(header, tmp);

  strcat(header, "\nConnection: close\nContent-Type: ");

  /*Insertion du type*/
  strcat(header, type);

  strcat(header, "; charset=UTF-8\n\n");

  return header;
}

void file_to_buffer(char * nomFic, char ** buff, int * size)
{
  /*Ouverture du fichier*/
  FILE * f = fopen(nomFic, "r");
  printf("fopen : %s\n", strerror(errno));

  /*Deplacement du curseur à la fin du fichier*/
  fseek(f, 0,SEEK_END);
  printf("fseek : %s\n", strerror(errno));

  /*On recupere le nombre de caractere*/
  *size = ftell(f);
  printf("fopen : %s\n", strerror(errno));

  int c;
  int i;
  /*On alloue un espace memoire de la taille du fichier*/
  *buff = malloc(*size * sizeof(char));

  /*On se replace au debut du fichier*/
  fseek(f, 0, SEEK_SET);
  printf("fseek : %s\n", strerror(errno));

  /*On recupere tout les caracteres*/
  for(i = 0; i < *size; i++)
  {
    c = fgetc(f);
    (*buff)[i] = c;
  }

  /*Et on referme le fichier*/
  fclose(f);
}

void send_get_answer(int fd)
{
  int size;
  char * buf = NULL;
  /*On recuper le fichier sous forme de chaine de cara*/
  file_to_buffer("catalogue.txt", &buf, &size);
  /*On construit l'entete HTML aproprié*/
  char * header = build_http_header("text/plain", size);

  /*Et on envoie les données*/
  puts("Going to send");
  send(fd, header, strlen(header), MSG_MORE);
  printf("send : %s\n", strerror(errno));
  send(fd, buf, size, 0);
  printf("send : %s\n", strerror(errno));

  /*On libere les ressources alloué*/
  free(header);
  free(buf);
}

void prepare_sock(int port, char * addr, int * sock, struct sockaddr_in * saddr)
{
  *sock = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
  printf("socket : %s\n", strerror(errno));
  
  saddr->sin_addr.s_addr = inet_addr(addr);
  saddr->sin_family = AF_INET;
  saddr->sin_port = htons(port);
}


int mk_sock(int port, char * addr)
{
  int sock;
  struct sockaddr_in saddr;

  prepare_sock(port, addr, &sock, &saddr);

  bind(sock, (struct sockaddr *)&saddr, sizeof(saddr));
  printf("bind : %s\n", strerror(errno));

  listen(sock, 10);
  printf("listen : %s\n", strerror(errno));
  
  return sock;
}

void central(int port)
{
  struct epoll_event ev, events[MAX_EVENTS];
  int nfds, epollfd;
  struct sockaddr_in saddr_client;
  
  int sock = mk_sock(port, "127.0.0.1");
  
  epollfd = epoll_create(10);
  if (epollfd == -1) {
    perror("epoll_create");
    exit(EXIT_FAILURE);
  }
  
  ev.events = EPOLLIN;
  ev.data.fd = sock;
  if (epoll_ctl(epollfd, EPOLL_CTL_ADD, sock, &ev) == -1) {
    perror("epoll_ctl: sock");
    exit(EXIT_FAILURE);
  }

  for (;;) {
    nfds = epoll_wait(epollfd, events, MAX_EVENTS, -1);
    puts("working ...");
    if (nfds == -1) {
      perror("epoll_pwait");
      exit(EXIT_FAILURE);
    }
    int n;
    for (n = 0; n < nfds; ++n) {
      if (events[n].data.fd == sock) {
	socklen_t size_addr = sizeof(struct sockaddr_in);
	int csock = accept(sock, (struct sockaddr *)&saddr_client, &size_addr);
	printf("accept : %s\n", strerror(errno));

	printf("Connection de %s :: %d\n", inet_ntoa(saddr_client.sin_addr), 
	       htons(saddr_client.sin_port));
	if (csock == -1) {
	  perror("accept");
	  exit(EXIT_FAILURE);
	}
	/*setnonblocking(csock);*/
	ev.events = EPOLLIN | EPOLLET;
	ev.data.fd = csock;
	if (epoll_ctl(epollfd, EPOLL_CTL_ADD, csock,
		      &ev) == -1) {
	  perror("epoll_ctl: csock");
	  exit(EXIT_FAILURE);
	}
      } 
      else{
	printf("%d\n", events[n].events == 1);
	if (events[n].events == 1)
	  send_get_answer(events[n].data.fd);
      }
    }
  }
}

void read_init(int sock, int * id, int * port_c)
{
  int i, fini = 0, b_get = 1, b_end = 1;
  char buff, buff_pre;
  char * get = "GET ", * end = "END ";

  *id = 0;
  *port_c = 0;

  for(i = 0; i < 4; i++)
  {
    do{}
    while(recv(sock, &buff, 1, 0) != 1);
      
    printf(":%c", buff);
    if (buff != get[i])
      b_get = 0;
    if (buff != end[i])
      b_end = 0;
  }

  printf("get : %d  end : %d\n", b_get, b_end);

  if(b_get)
  {
    fini = 0;
    while(!fini)
    {
      do{}
      while(recv(sock, &buff, 1, 0) != 1);
      if (buff == ' ' || buff == '\n' || buff == '\r')
	fini = 1;
      else
	*id = (*id * 10) + atoi(&buff); 	
    }
    printf("id %d\n", *id);
    
    /*fini = 0;
      for(i = 0; i < 12; i++)
      {
      recv(sock, &buff, 1, 0);
      if (buff != listen_port[i])
      cont = 0;
      }*/
    
    fini = 0;
    while(!fini)
      {
	do{}
	while(recv(sock, &buff, 1, 0) != 1);
	if (buff == ' ')
	  fini = 1;
      }
    puts("espace\n");
  
    fini = 0;
    while(!fini)
    {
      do{}
      while(recv(sock, &buff, 1, 0) != 1);
      if (buff == ' ' || buff == '\n' || buff == '\r')
	fini = 1;
      else
	*port_c = (*port_c * 10) + atoi(&buff); 	
    }
    printf("port : %d\n", *port_c);    
  }
  else
  {
    close(sock);
  }
  int nb_nl = 0;
  while(nb_nl < 2)
  {
    do{}
    while(recv(sock, &buff, 1, 0) != 1);
    
    if (buff == '\n')
      nb_nl++;
  }
}

void read_get(int sock, int * id)
{
  int i, fini = 0;
  char buff, buff_pre;
  char * get = "GET ";

  *id = 0;

  while(!fini)
  {
    do{}
    while(recv(sock, &buff, 1, 0) != 1);
    if (buff == ' ')
      fini = 1;
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
	*id = (*id * 10) + atoi(&buff); 	
    }


  int nb_nl = 0;
  while(nb_nl < 2)
  {
    do{}
    while(recv(sock, &buff, 1, 0) != 1);
    
    if (buff == '\n')
      nb_nl++;
  }
}

int send_image(int sock, int image)
{
  int len, sent;
  char str[12], *buff;
  sprintf(str, "%d.jpg", image);
  file_to_buffer(str, &buff, &len);
  sent = len;

  puts(buff);
  puts("Going to send image\n");
  do{
    sent -= send(sock, buff, len, 0);
  }
  while(sent > 0);
}

int connect_to(char * addr, int c_port)
{
  int sock;
  struct sockaddr_in saddr;
  prepare_sock(c_port, addr, &sock, &saddr);

  connect(sock, (struct sockaddr*)&saddr, sizeof(saddr));
  
  return sock;
}

void tcp_pull(int port, char * file)
{
  int sock = mk_sock(port, "127.0.0.1");

  int id, c_port;
  struct epoll_event ev, events[MAX_EVENTS];
  int nfds, epollfd;
  struct sockaddr_in saddr_client;
  

  epollfd = epoll_create(10);
  if (epollfd == -1) {
    perror("epoll_create");
    exit(EXIT_FAILURE);
  }
  
  ev.events = EPOLLIN;
  ev.data.fd = sock;
  if (epoll_ctl(epollfd, EPOLL_CTL_ADD, sock, &ev) == -1) {
    perror("epoll_ctl: sock");
    exit(EXIT_FAILURE);
  }

  for (;;) {
    nfds = epoll_wait(epollfd, events, MAX_EVENTS, -1);
    puts("working ...");
    if (nfds == -1) {
      perror("epoll_pwait");
      exit(EXIT_FAILURE);
    }
    int n;
    for (n = 0; n < nfds; ++n) {
      if (events[n].data.fd == sock) {
	socklen_t size_addr = sizeof(struct sockaddr_in);
	int csock = accept(sock, (struct sockaddr *)&saddr_client, &size_addr);
	printf("accept : %s\n", strerror(errno));

	printf("Connection de %s :: %d\n", inet_ntoa(saddr_client.sin_addr), 
	       htons(saddr_client.sin_port));
	if (csock == -1) {
	  perror("accept");
	  exit(EXIT_FAILURE);
	}
	/*setnonblocking(csock);*/
	ev.events = EPOLLIN | EPOLLET;
	ev.data.fd = csock;
	if (epoll_ctl(epollfd, EPOLL_CTL_ADD, csock,
		      &ev) == -1) {
	  perror("epoll_ctl: csock");
	  exit(EXIT_FAILURE);
	}
      }
      else{
	printf("%d\n", events[n].events == 1);
	if (events[n].events == 1)
	{
	  struct sockaddr_in addr;
	  int id, len;
	  read_init(events[n].data.fd, &id, &c_port);
	  printf("%d::%d\n", id, c_port);
	  getsockname(events[n].data.fd, (struct sockaddr*)&addr, &len); 
	  int data_sock = connect_to(inet_ntoa(addr.sin_addr), c_port);
	  read_get(events[n].data.fd, &id);
	  printf("%d\n", id);

	  send_image(data_sock, id);
	}
      }
    }
  }
}


int main(int argc, char ** argv)
{
  pid_t p_central = fork();
  if (p_central == 0)
    central(8082);
    
  pid_t p_tcp_pull = fork();
  if (p_tcp_pull == 0)
    tcp_pull(8083, NULL);

  wait(-1);
  wait(-1);
  return 0;
}
