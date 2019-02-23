#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/wait.h>

#define PORT 2025  // portul folosit
extern int errno;  // pentru returnarea mesajelor de eroare

void sighandler(int sig)
{
  if(sig==SIGCHLD)
  while(waitpid(-1,0,WNOHANG) > 0) { }
}

int main ()
{
  struct sockaddr_in server;  // structura folosita de server
  struct sockaddr_in from;
  char m[100];                //mesajul primit de la client
  char m_tocl[100]=" ";       //mesaj raspuns catre client
  int sd;                     //socket descriptor
  int child;

  // CREARE SOCKET
  if ((sd = socket (AF_INET, SOCK_STREAM, 0)) == -1)
  {
    perror ("[server]ERR socket().\n");
    return errno;
  }
  
  // Pregatirea structurilor de date
  bzero (&server, sizeof (server));
  bzero (&from, sizeof (from));

  // Umplem structura folosita de server
  server.sin_family = AF_INET;                 //stabilirea familiei de socket-uri
  server.sin_addr.s_addr = htonl (INADDR_ANY); // acceptam orice adresa
  server.sin_port = htons (PORT);              // utilizam un port utilizator

  // Atasam socketul
  if (bind (sd, (struct sockaddr *) &server, sizeof (struct sockaddr)) == -1)
  {
    perror ("[server]ERR bind().\n");
    return errno;
  }
  
  if(signal(SIGCHLD,sighandler) == SIG_ERR)  printf("It's ok"); sleep(5);
  
  // punem serverul sa asculte daca vin clienti sa se conecteze
  if (listen (sd, 5) == -1)
  {
    perror ("[server]ERR listen().\n");
    return errno;
  }

  // Servim in mod CONCURENT clientii
  while (1)
  {
    int client;
    socklen_t length = sizeof (from);
    printf ("[server]Asteptam la portul %d...\n",PORT);
    fflush (stdout);

    // Acceptam un client (stare blocanta pana la realizarea conexiunii)
    client = accept (sd, (struct sockaddr *) &from, &length);
    
    if ((child = fork()) == -1)
      perror("Err...fork");
    else if (child==0)        //copil
    {
      // ERR la acceptarea conexiunii de la un client
      if (client < 0)
      {
        perror ("[server]ERR la accept().\n");
        continue;
      }
      // S-a realizat conexiunea, se astepta mesajul
      bzero (m, 100);
      printf ("[server]Asteptam mesajul...\n");
      fflush (stdout);
      // Citirea mesajului
      if (read (client, m, 100) <= 0)
      {
        perror ("[server]ERR la read() de la client.\n");
        close (client);         // inchidem conexiunea
        continue;               // continuam sa ascultam
	  }
	  printf ("[server]RECEIVED: %s\n", m);
	  
	  // Pregatim mesajul de raspuns pentru client
	  bzero(m_tocl,100);
	  strcat(m_tocl,"Serverul va executa comanda 1, functia de login.\n ");
	  printf("[server]SENT: %s\n",m_tocl);

      // Returnam mesajul clientului
      if (write (client, m_tocl, 100) <= 0)
      {
        perror ("[server]ERR la write() catre client.\n");
        continue;
      }
      close (client);
      exit(0);
   } // if child
  } // while
}
