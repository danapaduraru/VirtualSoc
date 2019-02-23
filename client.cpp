#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>

extern int errno;
int port;

int main (int argc, char *argv[])
{
  int sd;
  struct sockaddr_in server;
  char m_tosv[100];

  if (argc != 3)
  {
    printf ("Sintaxa: %s <adresa_server> <port>\n", argv[0]);
    return -1;
  }
  port = atoi (argv[2]);   // stabilim portul

  // Cream socketul
  if ((sd = socket (AF_INET, SOCK_STREAM, 0)) == -1)
  {
    perror ("ERR socket().\n");
    return errno;
  }

  // Umplem structura folosita pentru realizarea conexiunii cu serverul
  server.sin_family = AF_INET;                  // familia socket-ului
  server.sin_addr.s_addr = inet_addr(argv[1]);  // adresa IP a serverului
  server.sin_port = htons (port);               // portul de conectare

  // CONNECT TO SERVER
  if (connect (sd, (struct sockaddr *) &server,sizeof (struct sockaddr)) == -1)
  {
    perror ("[client]ERR connect().\n");
    return errno;
  }

  // Read message
  bzero (m_tosv, 100);
  printf ("[client]Welcome to VirtualSoc! Choose one of the following:\n 1 Login\n 2 See all public posts\n 3 Create new account\n 4 Find out more about the application\n 5 Exit\n ");
  fflush (stdout);
  read (0, m_tosv, 100); // CITESTE DE LA TASTATURA

  // SEND TO server
  if (write (sd, m_tosv, 100) <= 0)
  {
    perror ("[client]ERR write() spre server.\n");
    return errno;
  }

  // Citirea raspunsului dat de server
  // (apel blocant pina cind serverul raspunde)
  if (read (sd, m_tosv, 100) < 0)
  {
    perror ("[client]ERR read() de la server.\n");
    return errno;
  }
  printf ("[client]RECEIVED: %s\n", m_tosv);
  close (sd);
}
