#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "network.h"

int main(int argc, char* argv[]) {
   int port;
   int server;
   int client;
   int running;
   int n;
   int opt;

   char id[52]; /*server ID som sendes til klient*/
   char buf[BUF_SIZE]; /*buffer for mottatt data*/
   char filename[60]; /*filnavn mottatt fra klient*/
   char acceptText[60]; /*tekst i 220 server accept*/

   struct sockaddr_in addr;
   struct sockaddr_in clientAddr;
   unsigned int clientLen;

   running = 1;
   opt = 1;

/*her sjekkes at programmet får riktige argumenter fra kommandolinjen*/
   if (argc != 5) {
      printf("Bruk: %s -port <port> -id <id>\n", argv[0]);
      return 1;
   }

   port = atoi(argv[2]);

/*her lagres server-ID*/
   memset(id, 0, sizeof(id));
   strncpy(id, argv[4], sizeof(id) - 1);

/*her opprettes TCP socket*/
   server = socket(AF_INET, SOCK_STREAM, 0);
   if (server < 0) {
      printf("Kunne ikke opprette socket.\n");   
      return 1;
   }

/*her tillates gjenbruk av porten (hindrer at det kommer dte er i bruk*/
   setsockopt(server, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

/*settes opp serveradresse*/
   memset(&addr, 0, sizeof(addr));
   addr.sin_family = AF_INET;
   addr.sin_addr.s_addr = inet_addr("127.0.0.1"); /*localhost*/
   addr.sin_port = htons(port);

/*her bindes socket til IP og port*/
   if (bind(server, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
      printf("Kunne ikke binde socket til porten.\n");
      close(server);   
      return 1;
   }

/*her starter det å lytte etter klienter*/
   if (listen(server, 1) < 0) {
      printf("Kunne ikke starte lytting på porten.\n");
      close(server);
      return 1;
   }

   printf("Lytter på 127.0.0.1:%d\n", port);

/*klienten ventes på*/
   clientLen = sizeof(clientAddr);
   client = accept(server, (struct sockaddr*)&clientAddr, &clientLen);
   if (client < 0) {
      printf("Kunne ikke akseptere klienttilkobling.\n");
      close(server);
      return 1;
   }

   printf("Klient koblet til.\n");

/*serveren er klar, sendes 220*/
   sprintf(acceptText, "127.0.0.1 SMTP %s", id);
   sendReply(client, "220", acceptText);
   printf("Sendte 220 server accept.\n");

/*hovedløkken for kommunikasjon med klienten*/
   while (running) {
      n = readPacket(client, buf, sizeof(buf));
      if (n <= 0) {
         break;
      }

/*HELO kommando*/
      if (strncmp(buf + 8, "HELO", 4) == 0) {
         printf("Mottok HELO.\n");
         sendReply(client, "250", "127.0.0.1 Hei klient");
         printf("Sendte 250 HELO svar.\n");
      } 
/*MAIL FROM kommando*/
      else if (strncmp(buf + 8, "MAIL FROM:", 10) == 0) {
         printf("Mottok MAIL FROM.\n");
         sendReply(client, "250", "Avsender ok");
         printf("Sendte 250 avsender godkjent\n");
      } 
/*RCPT TO kommando*/
      else if (strncmp(buf + 8, "RCPT TO:", 8) == 0) {
         printf("Morrok RCPT TO.\n");
         sendReply(client, "250", "Mottaker ok");
         printf("Sendte 250 mottaker ok\n");
      } 
/*DATA kommando, filoverføringen starter*/
      else if (strncmp(buf + 8, "DATA", 4) == 0) {
         memset(filename, 0, sizeof(filename));
         strncpy(filename, buf + 13, sizeof(filename) - 1);

         printf("Mottok DATA. Filnavn: %s\n", filename);

/*her sjekkes at filnavnet er gyldig før lagring*/
         if (validFilename(filename)) {
            sendReply(client, "354", "Klar til å motta data");
            printf("Sendte 354 klar til å motta data\n");

/*det leses og lagres fil fra klienten*/
            receiveFile(client, filename);

            sendReply(client, "250", "Fil lagret");
            printf("Sendte 250 fil lagret\n");
         } 
         else {
            sendReply(client, "501", "Ugyldig filnavn");
            printf("Sendte 501 ugyldig filnavn.\n");
         }
      } 
/*QUIT kommando*/
      else if (strncmp(buf + 8, "QUIT", 4) == 0) {
            printf("Mottok QUIT.\n");
            sendReply(client, "221", "Lukker tilkobling");
            printf("Sendte 221 lukker tilkobling\n");
            running = 0;
      } 
/*hvis det er en ukjent kommando som sendes*/
      else {
            printf("Mottok ukjent kommando\n");
            sendReply(client, "500", "Ukjent kommando");
      }
   }

/*forbindelsen lukkes her*/
   close(client);
   close(server);

   return 0;

}
