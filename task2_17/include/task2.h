#ifndef TASK2_H
#define TASK2_H

/*structen pakkes uten padding slik at binærfilen får riktig format*/
#pragma pack(1)
typedef struct TASK2_NUMBERS_METADATA {
    int iIndex;                   /*rekkefølgen, starte på 1*/
    int iNumber;                  /*selve tallet*/
    int bIsFibonacci;
    int bIsPrimeNumber;
    int bIsSquareNumber;
    int bIsCubeNumber;
    int bIsPerfectNumber;
    int bIsAbundantNumber;
    int bIsDeficientNumber;
    int bIsOddNumber;
} TASK2_NUMBERS_METADATA;
#pragma pack()

/*Funksjoner som returnerer 1 (true) eller 0 (false)*/
int isFibonacci(int n);
int isPrime(int n);
int isSquareNumber(int n);
int isCubeNumber(int n);
int isPerfectNumber(int n);
int isAbundantNumber(int n);
int isDeficientNumber(int n);
int isOdd(int n);

#endif
