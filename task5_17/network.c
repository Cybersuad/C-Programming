#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include "network.h"

/*her settes opp EWA-header med magic number, størrelse og delimiter*/
void setHeader(struct EWA_EXAM25_TASK5_PROTOCOL_SIZEHEADER* h, int size) {
	memcpy(h->acMagicNumber, "EWP", 3);
	sprintf(h->acDataSize, "%04d", size);
	h->acDelimeter[0] = '|';
}

/*her sendes hele pakken også hvis send() ikke sender alt på første forsøk*/
int sendAll(int s, void* data, int size) {
   int sent;
   int n;

   sent = 0;

   while (sent < size) {
      n = send(s, (char*)data + sent, size -sent, 0);
      
         if (n <= 0) {
         return 0;
      }

      sent += n;
   }

     return 1;
   }

/*her lages og sendes det et standard serversvar med statuskode og tekst*/
void sendReply(int s, char* code, char* text) {
   struct EWA_EXAM25_TASK5_PROTOCOL_SERVERREPLY r;

   memset(&r, 0, sizeof(r));
   setHeader(&r.stHead, sizeof(r));

   memcpy(r.acStatusCode, code, 3);
   r.acHardSpace[0] = ' ';
   strncpy(r.acFormattedString, text, sizeof(r.acFormattedString) - 1);

   sendAll(s, &r, sizeof(r));
}

/*denne funksjonen leser en komplett EWA-pakke basert på størrelsen i headeren*/
int readPacket(int s, char* buf, int max) {
   int got;
   int n;
   int size;
   char sizeText[5];

   memset(buf, 0, max);

/*først leses headeren på 8 bytes*/
   got = recv(s, buf, 8, 0);
   if (got <= 0) return got;

/*videre hentes pakkestørrelsen fra headeren*/
   memset(sizeText, 0, sizeof(sizeText));
   memcpy(sizeText, buf + 3, 4);
   size = atoi(sizeText);

/*her stoppes ugyldige eller for store pakker*/
   if (size <= 0 || size > max) {
      return -1;
   }

/*resten av pakken leses hvis alt ikke kom i første recv()*/
   while (got < size) {
      n= recv(s, buf + got, size - got, 0);
      if (n <= 0) {
         return n;
      }

      got += n;
   }
   
   return got;
}

/*det sjekkes for om filnavnet ikke inneholder farlige eller ugyldige tegn*/
int validFilename(char* name) {
   int i;

   if(name == NULL || name [0] == '\0') {
      return 0;
   }

   i = 0;
   while (name[i] != '\0') {
      if (name[i] == '/' || name[i] == '\\' || name [i] == ':' ||
          name[i] == '*' || name[i] == '?' || name [i] == '"' ||
          name[i] == '<' || name[i] == '>' || name [i] == '|') {
         return 0;

       }
       i++;
   }

   return 1;
}

/*videre mottas det her filinnholdet fra kleint og lagrer det til en fil*/
void receiveFile(int s, char* filename) {
   FILE* f;
   char buf[BUF_SIZE];
   int n;
   char* start;
   char* end;

   f= fopen(filename, "wb");
   if (f == NULL) {
      printf("Kunne ikke opprette filen.\n");
      return;
   }

   while (1) {
      n = readPacket(s, buf, sizeof(buf));
      if (n <= 0) {
         break;
      }

      start = buf + 8;
      end = strstr(start, "\r\n.\r\n");

/*stopper så når sluttmarkøren kommer, og lagrer kun data før markøren*/
      if (end != NULL) {
         fwrite(start, 1, end - start, f);
         break;

      } 

      fwrite(start, 1, n - 8, f);
   }
   
   fclose(f);
}


