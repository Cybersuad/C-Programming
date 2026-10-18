/* This file has been created by EWA, and is part of task 4 on the exam for PG3401 2026*/
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "task4_prime.h"

void tean(unsigned int *const v, unsigned int *const w, const unsigned int *const k, int N);
unsigned int tcp_checksum(const unsigned char * data, size_t length);

/*Her legges det til en struktur som brukes til å lagre primtall i en lenket liste*/
struct PGPRIMENUMBER {
   unsigned int uiPrime;
   struct PGPRIMENUMBER* next;
};

/*Her brukes det til å sende delt data inn til trådene. nødvendigheten av dette er at filen og primtallslisten brukes av begge trådene. */
struct THREADDATA {
   FILE* fInputFile;
   char* codeFile;
   struct PGPRIMENUMBER** ppPrimeList;

/*Dette er en mutex som beskytter lesing fra filen */
   pthread_mutex_t* pFileMutex;

/*Dette er en mutex som beskytter endringer av primtallslisten*/
   pthread_mutex_t* pListMutex;
};

/*Denne funksjonen forsøker å dekryptere en kryptert fil ved å bruke et primtall*/
int tryDecrypt(unsigned int prime, char* codeFile) {
   FILE* in = NULL;
   FILE* out = NULL;
   FILE* hashFile = NULL;

   unsigned char* enc = NULL; /*buffer for kryptert data*/
   unsigned char* plain = NULL; /*buffer for dekryptert data*/

   long size = 0;
   long i = 0;

   unsigned int key[4]; /*nøkkel til kryptering/dekryptering*/
   unsigned int v[2];
   unsigned int w[2];

/*her åpnes den krypterte filen*/
   in = fopen(codeFile, "rb");
   if (in == NULL) {
      return 0;
   }

/*her finnes størrelsen på filen*/
   fseek(in, 0, SEEK_END);
   size = ftell(in);
   rewind(in);

/*her allokerer minne for kryptert og dekryptert data*/
   enc = (unsigned char*)malloc(size);
   plain = (unsigned char*)malloc(size);

   if (enc == NULL || plain == NULL) {
      fclose(in);
      free(enc);
      free(plain);
      return 0;
   }

/*her lese hele filen inn i buffer*/
   fread(enc, 1, size, in);
   fclose(in);

/*samme primtall settes som nøkkel*/
   key[0] = prime;
   key[1] = prime;
   key[2] = prime;
   key[3] = prime;

/*her dekrypteres data i blokker på 8 bytes*/
   for (i = 0; i < size; i += 8) {
      memcpy(v, enc + i, 8);
      tean(v, w, key, -32); /*dekrypteringsalgoritmen*/
      memcpy(plain + i, w, 8);
   }

/*her sjekkes om dekryptert tekst som starter med "BENGT" */
   if (memcmp(plain, "BENGT", 5) == 0) {

/*dekryptert teksten skrives til en fil*/
      out = fopen("task4_plain.txt", "wb");
      if (out != NULL) {
         fwrite(plain, 1, size, out);
         fclose(out);
      }

/*lages hash av dekryptert data*/
      hashFile = fopen("task4_plain.hash", "w");
      if (hashFile != NULL) {
         fprintf(hashFile, "%u\n", tcp_checksum(plain, size));
         fclose(hashFile);
      }

      printf("Riktig nøkkel funnet: %u\n", prime);

      free(enc);
      free(plain);
      return 1;
   }

/*minnet frigjøres hvis nøkkelen ikke var riktig*/
   free(enc);
   free(plain);
   return 0;
}

/* Denne funksjonen kjøres av begge trådene. Her leses det tall fra samme fil, finner primtallene og legger dem i samme liste.*/
void* threadFunction(void* arg){
   struct THREADDATA* pData = (struct THREADDATA*)arg;

   unsigned int auiNumbers[10] = {0};
   int iNumbersRead = 0;
   int iIndex = 0;
   struct PGPRIMENUMBER* newNode = NULL;

   while (1) {
      iNumbersRead = 0;

/*Her låses fil-mutex før lesing fra filen. Uten denne kan begge trådene lese fra samme fil samtidig noe som kan føre til feil i lesingen*/
      pthread_mutex_lock(pData->pFileMutex);

      while (iNumbersRead < 10) {
         if (fscanf(pData->fInputFile, "%u", &(auiNumbers[iNumbersRead])) == 1) {
            iNumbersRead++;
         }
         else {
            break;
         }
      }

/*Her låses fil-mutex etter lesing. den andre tråden kan lese trygt fra filen*/
      pthread_mutex_unlock(pData->pFileMutex);

      if (iNumbersRead == 0) {
         break;
      }

      for (iIndex = 0; iIndex < iNumbersRead; iIndex++) {
         if (isPrime(auiNumbers[iIndex])) {
            newNode = (struct PGPRIMENUMBER*)malloc(sizeof(struct PGPRIMENUMBER));
            if (newNode != NULL) {
               newNode->uiPrime = auiNumbers[iIndex];

               pthread_mutex_lock(pData->pListMutex);

               newNode->next = *(pData->ppPrimeList);
               *(pData->ppPrimeList) = newNode;
               
               pthread_mutex_unlock(pData->pListMutex);

               newNode = NULL;

               if (tryDecrypt(auiNumbers[iIndex], pData->codeFile)) {
                  return NULL;
              }  
            }
         }
      }
   }

   return NULL;
}

int main(int argc, char* argv[]){
   pthread_t thread1;
   pthread_t thread2;

/*Primtallslisten er lokal i mainen og ikke global. tilgang blir gitt til listen gjennom THREADDATA*/
   struct PGPRIMENUMBER* pPrimeList = NULL;
   struct PGPRIMENUMBER* pPtr = NULL;

   FILE* fInputFile = NULL;

/*Mutexene her brukes for å beskytte delt data. fileMutex beskytter filen mens listMutex beskytter primtalsslisten*/
   pthread_mutex_t fileMutex;
   pthread_mutex_t listMutex;

   struct THREADDATA threadData;
   int rc;


   if (argc != 2) {
      printf("File usage %s<filename>\n", argv[0]);
      return 1;
   }

   fInputFile = fopen(argv[1], "r");
   if (fInputFile == NULL) {
      return 1;
   }

/*Her initialiseres mutexene før trådene starter*/
   if (pthread_mutex_init(&fileMutex, NULL) != 0) {
      fclose(fInputFile);
      return 1;
   }

   if (pthread_mutex_init(&listMutex, NULL) != 0) {
      pthread_mutex_destroy(&fileMutex);
      fclose(fInputFile);
      return 1;
   }

/*legger filen, listen og mutexene inn i THREADDATA, slik at begge trådene får tilgang til samme data.*/
   threadData.fInputFile = fInputFile;
   threadData.codeFile = "task4_code.bin";
   threadData.ppPrimeList = &pPrimeList;
   threadData.pFileMutex = &fileMutex;
   threadData.pListMutex = &listMutex;

/*opretter første tråd*/
   rc = pthread_create(&thread1, NULL, threadFunction, &threadData);
   if (rc != 0) {
      pthread_mutex_destroy(&fileMutex);
      pthread_mutex_destroy(&fileMutex);
      fclose(fInputFile);
      return 1;
   }

/*her opprettes andre tråd, begge trådene bruker samme fil og liste men mutexene sørger for at de ikke bruker dem samtidig på en farlig måte.*/
   rc = pthread_create(&thread2, NULL, threadFunction, &threadData);
   if (rc != 0) {
      pthread_join(thread1, NULL);
      pthread_mutex_destroy(&fileMutex);
      pthread_mutex_destroy(&listMutex);
      fclose(fInputFile);
      return 1;
   }

/*her ventes det til at begge trådene er ferdige*/
   pthread_join(thread1, NULL);
   pthread_join(thread2, NULL);

   fclose(fInputFile);

   printf("\r\nPrime numbers found : \r\n");
   pPtr = pPrimeList;
   while (pPtr != NULL) {
      printf("%u\r\n", pPtr->uiPrime);
      pPtr = pPtr->next;
   }

/*minnet blir firgjort som ble brukt til den lenkede listen*/
   while (pPrimeList != NULL) {
      pPtr = pPrimeList;
      pPrimeList = pPrimeList->next;
      free(pPtr); pPtr = NULL;
   }

/*her ødelegges mutexene etter at trådene er ferdige og mutexene ikke lenger skal brukes. */
   pthread_mutex_destroy(&fileMutex);
   pthread_mutex_destroy(&listMutex);
   
   return 0;
}
