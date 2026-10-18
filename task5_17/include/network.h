#ifndef NETWORK_H
#define NETWORK_H

#include "ewpdef.h"

/*dette er størrelsen på bufferen som brukes til nettverksdata*/
#define BUF_SIZE 12000

/*settes opp header for EWA protokollen(magic number, størrelse og delimiter*/
void setHeader(struct EWA_EXAM25_TASK5_PROTOCOL_SIZEHEADER* h, int size);

/*sender hele datastrømmen over socket (sikrer at alt blir sendt)*/
int sendAll(int s, void* data, int size);

/*sender et standard svar til klienten med statuskode og melding */
void sendReply(int s, char* code, char* text);

/*leser en komplett pakke fra socket basert på headerens størrelse*/
int readPacket(int s, char* buf, int max);

/*valideres at filnavnet er gyldig og ikke inneholder ulovlige tegn*/
int validFilename(char* name);

/*mottar filinnhold fra kleint og lagrer det til disk*/
void receiveFile(int s, char* filename);

#endif
