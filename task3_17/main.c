#include <stdio.h>
#include <string.h>
#include "include/exam.h"

int main(void) {
	ExamList list; /*dobbelt lenket liste med kandidater*/
	int choice; /*menyvalg for brukeren*/
   char id[32]; /*kandidat id*/
   char file[128]; /*filnavn*/
   char just[256]; /*begrunnelsen for vurdering*/
   char g; /*karakter (A-F)*/
   char answer; /*Ja/nei valget*/
   int task; /*oppgavenummer*/
   int points; /*poeng for oppgavene*/

   choice = 0;
   initList(&list); /*her initialiseres en tom liste før bruk*/

/*dette er hovedmeny som kjører til brukeren velger å avslutte den*/
	while (choice != 8) {
		printf("\n--- Eksamenssystem meny---\n");
		printf("1. Legg til kandidat manuelt\n");
		printf("2. Legg til kandidater fra fil\n");
		printf("3. Vurder en oppgave\n");
		printf("4. Skriv ut alle kandidater med score og karakter\n");
		printf("5. Skriv ut kandidater med valgt karakter\n");
		printf("6. Skriv ut kandidater som ikke er ferdig vurdert\n");
		printf("7. Skriv ut full informasjon om en kandidate\n");
		printf("8. Avslutt\n");
		printf("Velg: ");
		scanf("%d", &choice);

		if (choice == 1) {
			printf("Kandidat ID: ");
			scanf("%s", id);
			addCandidateManual(&list, id); /*legger kandidaten inn bakerst i listen*/
		}

		else if (choice == 2) {

/*her leses kandidatene automatisk fra fil*/
         addCandidatesFromFile(&list, "candidate.txt");

			printf("Kandidater lagt til fra fil.\n");

/*her gis brukeren valg om å vise resultatet direkte*/
         printf("Vil du vise kandidatene? (y/n): ");
			scanf("%c", &answer);

			if (answer == 'y' || answer == 'Y') {
            printAll(&list);
         }
		}

		else if (choice == 3) {
			printf("Kandidat ID: ");
			scanf("%s", id);

			printf("Oppgavenummer (1-%d): ", NUM_TASKS);
			scanf("%d", &task);

			printf("Poeng:  ");
			scanf("%d", &points);

			printf("Begrunnelse: ");
			getchar(); /*her fjernes newline fra buffer før fgets*/
			fgets(just, 256, stdin);
			just[strcspn(just, "\n")] = 0; /*fjerner linjeskrift fra input*/

/*her oppdateres valgt oppgave og recalculerer total scoren*/
			gradeTask(&list, id, task - 1, points, just);
		}

		else if (choice == 4) {
			printAll(&list); /*alle kandidatene med karakter blir vist*/
		}

		else if (choice == 5) {
			printf("Karakter: ");
			scanf(" %c", &g);
			printByGrade(&list, g); /*her filtreres kandidater basert på karakter*/
		}

		else if (choice == 6) {

/*her vises kandidater som mangler vurdering (når ikke alle oppagver er satt)*/
			printUngraded(&list);
		}

		else if (choice == 7) {
			printf("Kandidat ID: ");
			scanf("%s", id);
			printCandidate(&list, id); /*detaljert visning av en kandidat*/
		}
	}

	freeList(&list); /*her frigis alt dynamisk minne før avslutning*/
	return 0;
}

