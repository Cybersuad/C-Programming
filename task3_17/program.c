#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "include/exam.h"

/*her initialiseres det en tom list, før det tas i bruk*/
void  initList(ExamList* list) {
   list->head = NULL;
   list->tail = NULL;
   list->count = 0;
}

/*oppretting av en nu kandidat og setter startverdier for alle oppgaver*/
static ExamCandidate* createCandidate(const char* id) {
   ExamCandidate* c;
   int i;

   c = malloc(sizeof(ExamCandidate));

   if (c == NULL) {
      printf("Minneallokering feilet.\n");
      return NULL;
   }

   strcpy(c->candidateID, id);

/*-1 brukes for å vise at oppgaven ikke er vurdert ennå*/
   for (i=0; i < NUM_TASKS; i++) {
      c->points[i]= -1;
      strcpy(c->justification[i], "");
   }

   c->finalScore = 0;
   c->next = NULL;
   c->prev = NULL;

   return c;
}

/*her legges en ny kandidat bakerst i den dobbeltlenkede listen*/
void addCandidateManual(ExamList* list, const char* id) {
   ExamCandidate* c;

   c = createCandidate(id);

   if (c == NULL) {
      return;
   }

   if (list->head == NULL) {
      list->head = c;
      list->tail = c;
   }

   else {
      list->tail->next = c;
      c->prev = list->tail;
      list->tail = c;
   }

   list->count++;
}

/*her leses kandidatIDene fra filen og legger dem inn i listen*/
void addCandidatesFromFile(ExamList* list, const char* filename) {
   FILE* fp;
   char id[64];

   fp = fopen(filename, "r");

   if (!fp) {
      printf("Kunne ikke åpne fil.\n");
      return;
   }

   while (fscanf(fp, "%s", id) == 1) {
      addCandidateManual(list, id);
   }

   fclose(fp);
}

/*det søkes gjennom listen og returnerer kandidaten med riktig ID.*/
static ExamCandidate* find(ExamList* list, const char* id) {
   ExamCandidate* c;

   c = list->head;

   while (c) {
      if (strcmp(c->candidateID, id) == 0) {
         return c;
      }

      c = c->next;

   }

   return NULL;
}

/*her settes poeng og begrunnelse for en oppgave og oppdaterer totalscoren*/
void gradeTask(ExamList* list, const char* id, int task, int points, const char* just) {
   ExamCandidate* c;
   int sum;
   int i;

   c = find(list, id);

   if (!c) {
      printf("Kandidat ikke funnet.\n");
      return;
   }

   if (task < 0 || task >= NUM_TASKS) {
      printf("Oppgave finnes ikke.\n");
      return;
   }

   c->points[task] = points;
   strcpy(c->justification[task], just);

/*totalscore blir regnet ut på nytt etter vurdering*/
   sum = 0;

   for (i = 0; i<NUM_TASKS; i++) {
      if (c->points[i] >= 0) {
         sum += c->points[i];
      }
   }

   c->finalScore = sum;

}

/*det beregnes karakter basert på totalscoren*/
char calculateGrade(int score) {
   if (score >= 90) return 'A';
   if (score >= 80) return 'B';
   if (score >= 70) return 'C';
   if (score >= 60) return 'D';
   if (score >= 50) return 'E';

   return 'F';
}

/*skriver ut alle kandidatene med totalscore og beregner karakter*/
void printAll(ExamList* list) {
   ExamCandidate* c;
   char g;

   c = list->head;

   while (c) {
      g = calculateGrade(c->finalScore);
      printf("%s  Score: %d   Karakter: %c\n", c->candidateID, c->finalScore, g);
      c = c->next;
   
   }
}

/*her skrives det ut kandidater som har den karaktern brukeren søker etter*/
void printByGrade(ExamList* list, char grade) {
   ExamCandidate* c;

   if (grade >= 'a' && grade <= 'z') {
      grade = grade - 32;
   }

   c = list->head;

   while (c) {
      if (calculateGrade(c->finalScore) == grade) {
         printf("%s  Score: %d\n", c->candidateID, c->finalScore);
      }

      c = c->next;
   }
}

/*her skrives ut kandidater som mangler poeng eller begrunnelse på minst en oppgave*/
void printUngraded(ExamList* list) {
   ExamCandidate* c;
   int incomplete;
   int i;

   c= list->head;

   while (c) {
      incomplete = 0;

      for (i = 0; i < NUM_TASKS; i++) {
         if (c->points[i] < 0 || strlen(c->justification[i]) == 0) {
            incomplete = 1;
         }
      }

      if (incomplete) {
         printf("%s ikke vurdert.\n", c->candidateID);
      }

      c = c->next;
   }
}

/*her skrives det ut en detaljert vurdering for en bestemt kandidat*/
void printCandidate(ExamList* list, const char* id) {
   ExamCandidate* c;
   int i;

   c = find(list, id);

   if (!c) {
      printf("Kandidat ikke funnet.\n");
      return;
   }

   printf("Kandidat %s\n", c->candidateID);
   printf("Total score: %d   Karakter: %c\n", c->finalScore, calculateGrade(c->finalScore));

   for (i = 0; i<NUM_TASKS; i++) {
      printf("Oppgave %d: %d Poeng\n", i + 1, c->points[i]);
      printf("Begrunnelse: %s\n", c->justification[i]);
   }
}

/*her frigis alle noder i listen før programmet avsluttes*/
void freeList(ExamList* list) {
   ExamCandidate* c;
   ExamCandidate* next;

   c = list->head;

   while (c) {
      next = c->next;
      free(c);
      c = next;
   }

   list->head = NULL;
   list->tail = NULL;
   list->count = 0;
}
