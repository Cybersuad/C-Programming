#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include "ewpdef.h"

#define BUFFER_SIZE 70000
#define OUTPUT_FILE "task6_received.bmp"
#define HEADER_SIZE 20

#define FLAG_FIN  0x01
#define FLAG_ACK  0x10
#define FLAG_NACK 0x20

/*her lese nøyaktig antall bytes fra socketen, recv() kan lese mindre enn ønsket, derfor brukes en løkke*/
int read_exact(int sock, unsigned char *buffer, int length) {
   int total = 0;

   while (total < length) {
      int received = recv(sock, buffer + total, length - total, 0);

      if (received <= 0) {
         return -1;
      }

      total += received;
   }

   return total;
}

/*her beregnes checksum for datapakken. dette brukes for å kontrollere at data ikke er ødelagt under overføringen*/
unsigned short calculate_checksum(unsigned char *data, int length) {
   unsigned int sum = 0;
   int i;
   short word;

   for (i = 0; i < length; i += 2) {
      word = (short)(data[i] << 8);

      if (i + 1 < length) {
         word = (short)(word | data[i + 1]);
      }

      sum += word;

      if (sum > 0xFFFF) {
         sum = (sum & 0xFFFF) + 1;
      }
   }

   return (unsigned short)(~sum & 0xFFFF);
}

/*denne funksjonen sender ACK eller NACK tilbake til EWA. ACK betyr pakken bli mottatt riktig. NACK betyr at pakken må sendes på nytt*/
void send_ack_or_nack(int sock, unsigned int sequence, int isNack) {
   unsigned char response[HEADER_SIZE];
   struct EWA_EXAM25_TASK4_PROTOCOL_TCP *pkt =
      (struct EWA_EXAM25_TASK4_PROTOCOL_TCP *)response;

   memset(response, 0, HEADER_SIZE);

   pkt->uiSequenceNumber = sequence;
   pkt->uiAckNumber = sequence;
   pkt->ucFlags = isNack ? FLAG_NACK : FLAG_ACK;
   pkt->usSizeOfPacket = 0;

   send(sock, response, HEADER_SIZE, 0);
}

int main(int argc, char *argv[]) {
   char *serverIp = NULL;
   int port = 0;
   int i;

/*Her leses IP-adresse og port fra kommandolinjen. disse verdiene hentes fra EWA-verktøyet*/
   for (i = 1; i < argc; i++) {
      if (strcmp(argv[i], "-server") == 0 && i + 1 < argc) {
         serverIp = argv[++i];
      } else if (strcmp(argv[i], "-port") == 0 && i + 1 < argc) {
         port = atoi(argv[++i]);
      }
   }

   if (serverIp == NULL || port == 0) {
      printf("Usage: ./task6 -server <ip> -port <port>\n");
      return 1;
   }

/*oppretting av en TCP-socket som brukes til å koble til EWA-serveren*/
   int sock = socket(AF_INET, SOCK_STREAM, 0);
   if (sock < 0) {
      perror("socket");
      return 1;
   }

/*her settes opp serveradressen med IP og port */
   struct sockaddr_in serverAddress;
   memset(&serverAddress, 0, sizeof(serverAddress));
   serverAddress.sin_family = AF_INET;
   serverAddress.sin_port = htons(port);

   if (inet_pton(AF_INET, serverIp, &serverAddress.sin_addr) <= 0) {
      perror("inet_pton");
      close(sock);
      return 1;
   }

/*kobler klienten til EWA-serveren*/
   if (connect(sock, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) {
      perror("connect");
      close(sock);
      return 1;
   }

   printf("Connected to EWA server %s:%d\n", serverIp, port);

/*her åpnes output-filen i binær skrivemodus. og bildet fra EWA lagres som en fil*/
   FILE *file = fopen(OUTPUT_FILE, "wb");
   if (file == NULL) {
      perror("fopen");
      close(sock);
      return 1;
   }

/*bufferen brukes til lagre både header og data for hver pakke*/
   unsigned char *buffer = malloc(BUFFER_SIZE);
   if (buffer == NULL) {
      perror("malloc");
      fclose(file);
      close(sock);
      return 1;
   }

/*her holdes det styr på hvilket sekvensnummer som forventes neste gang*/
   unsigned int expectedSequence = 0;

/*denne funksjonen leser pakker fra EWA helt til FIN-flagget mottas*/
   while (1) {
      memset(buffer, 0, BUFFER_SIZE);

/*først lese headeren på 20 bytes. headeren inneholder sekvensenummer, størrelse, checksum og flagg*/
      if (read_exact(sock, buffer, HEADER_SIZE) < 0) {
         printf("Connection closed while reading header.\n");
         break;
      }

/*tolker starten av bufferet som en EWA TCP-header*/
      struct EWA_EXAM25_TASK4_PROTOCOL_TCP *pkt =
         (struct EWA_EXAM25_TASK4_PROTOCOL_TCP *)buffer;

      unsigned int sequence = pkt->uiSequenceNumber;
      unsigned short dataSize = pkt->usSizeOfPacket;
      unsigned short receivedChecksum = pkt->usChecksum;
      unsigned char flags = pkt->ucFlags;

      printf("Packet seq=%u dataSize=%u flags=0x%02X expected=%u\n",
         sequence, dataSize, flags, expectedSequence);

/*avviser pakker som er større enn bufferet*/
      if ((int)dataSize > BUFFER_SIZE - HEADER_SIZE) {
         printf("Packet too large. Sending NACK.\n");
         send_ack_or_nack(sock, sequence, 1);
         continue;
      }

/*dataene starter rett etter headeren i bufferet*/
      unsigned char *dataPtr = buffer + HEADER_SIZE;

/*leser selve datadelen dersom pakken inneholder data*/
      if (dataSize > 0) {
         if (read_exact(sock, dataPtr, dataSize) < 0) {
            printf("Connection closed while reading data.\n");
            break;
         }
      }

/*kontrollerer checksum for å oppdage ødelagte pakker. ved feil sendes NACK slik at EWA kan sende pakken på nytt*/
      if (receivedChecksum != 0 && dataSize > 0) {
         unsigned short calculatedChecksum =
            calculate_checksum(dataPtr, dataSize);

         if (calculatedChecksum != receivedChecksum) {
            printf("Checksum mismatch on packet %u. Sending NACK.\n", sequence);
            send_ack_or_nack(sock, sequence, 1);
            continue;
         }
      }
/*hvis pakken kommer i feil rekkefølge, brukes sekvensnummeret til å skrive dataene på riktig plass i filen */
      if (sequence != expectedSequence) {
         printf("Out-of-sequence: got %u, expected %u. Writing at correct position.\n",
            sequence, expectedSequence);
      }

/*skriver dataene til riktig posisjon i output-filen*/
      if (dataSize > 0) {
         fseek(file, sequence, SEEK_SET);
         fwrite(dataPtr, 1, dataSize, file);
      }

      printf("Packet %u received and written (%u bytes). Sending ACK.\n",
         sequence, dataSize);

/*sender ACK etter at pakken er validert og skrevet til fil*/
      send_ack_or_nack(sock, sequence, 0);

/*oppdaterer forventet sekvensnummer når riktig pakke er mottatt*/
      if (sequence == expectedSequence) {
         expectedSequence = sequence + dataSize;
      }

/*FIN-flagget betyr at filoverføringen er ferdig*/
      if (flags & FLAG_FIN) {
         printf("FIN received. File transfer complete.\n");
         break;
      }
   }

/*rydder før programmet avsluttes*/
   free(buffer);
   fclose(file);
   close(sock);

   printf("File saved as %s\n", OUTPUT_FILE);
   return 0;
}
