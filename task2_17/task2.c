#include <stdio.h> /*for input og output funksjoner*/
#include <stdlib.h> /*for biblioteksfunksjoner*/
#include "task2.h" /*egen headerfil*/

int main(void) {

/*filpekere for filene*/
    FILE *inputFile = NULL;
    FILE *outputFile = NULL;
    TASK2_NUMBERS_METADATA data;

    int number = 0; /*varaibel for å lagre hvert tall lest fra filen*/
    int index = 1;

/*inputfilen blir åpnet for lesing*/
    inputFile = fopen("pgexam26_test.txt", "r");
    if (inputFile == NULL) {
        perror("Kunne ikke åpne input fil"); /*feilmelding hvis filen ikke kan åpnes*/
        return 1;
    }

/*outputfil opprettes i binær modus*/
    outputFile = fopen("pgexam26_output.bin", "wb");
    if (outputFile == NULL) {
        perror("Kunne ikke lage output fil"); /*feilmelding hvis det ikke lykkes*/
        fclose(inputFile); /*inputfilen lukkes før avslutning*/
        return 1;
    }

/*tallene fra inputfilen leses så lenge det returneres 1*/
    while (fscanf(inputFile, "%d", &number) == 1) {

/*verdien i structen settes*/
        data.iIndex = index;
        data.iNumber = number;

/*de ulike funksjonene kalles for å sjekke egenskapene til tallet*/
        data.bIsFibonacci = isFibonacci(number);
        data.bIsPrimeNumber = isPrime(number);
        data.bIsSquareNumber = isSquareNumber(number);
        data.bIsCubeNumber = isCubeNumber(number);
        data.bIsPerfectNumber = isPerfectNumber(number);
        data.bIsAbundantNumber = isAbundantNumber(number);
        data.bIsDeficientNumber = isDeficientNumber(number);
        data.bIsOddNumber = isOdd(number);

/*structen skrives til outputfilen i binær format*/
        fwrite(&data, sizeof(TASK2_NUMBERS_METADATA), 1, outputFile);

/*sekvensnummeret økes for neste tall*/
        index++;
    }

/*filene lukkes ett bruk*/
    fclose(inputFile);
    fclose(outputFile);

    printf("pgexam26_output.bin opprettet\n");

    return 0;
}
