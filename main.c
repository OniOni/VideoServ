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

void file_to_buffer(char ** buff, int * size)
{
  /*Ouverture du fichier*/
  FILE * f = fopen("catalogue.txt", "r");
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
  file_to_buffer(&buf, &size);
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

void central(int port)
{
  struct epoll_event ev, events[MAX_EVENTS];
  int nfds, epollfd;
  
  int sock = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
  printf("socket : %s\n", strerror(errno));
  
  struct sockaddr_in saddr, saddr_client;;
 
  saddr.sin_addr.s_addr = htonl(INADDR_ANY);
  saddr.sin_family = AF_INET;
  saddr.sin_port = htons(port);

  bind(sock, (struct sockaddr *)&saddr, sizeof(saddr));
  printf("bind : %s\n", strerror(errno));

  listen(sock, 10);
  printf("listen : %s\n", strerror(errno));

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

int main(int argc, char ** argv)
{
  pid_t p_central = fork();
  if (p_central == 0)
    central(8082);
    
  wait(-1);
  return 0;
}
