#ifndef EXAM_H
#define EXAM_H

#define NUM_TASKS 4
#define MAX_JUST 256
#define MAX_ID 32

/*dette er dtrukturent for en kandidat*/
typedef struct ExamCandidate {
    char candidateID[MAX_ID];
    int points[NUM_TASKS];
    char justification[NUM_TASKS][MAX_JUST];
    int finalScore;
    struct ExamCandidate* next;
    struct ExamCandidate* prev;
} ExamCandidate;

/*dette er struktur for listen*/
typedef struct {
    ExamCandidate* head;
    ExamCandidate* tail;
    int count;
} ExamList;

/*dette er de ulike funksjonsprototypene*/
void initList(ExamList* list);
void addCandidateManual(ExamList* list, const char* id);
void addCandidatesFromFile(ExamList* list, const char* filename);
void gradeTask(ExamList* list, const char* id, int task, int points, const char* just);
void printAll(ExamList* list);
void printByGrade(ExamList* list, char grade);
void printUngraded(ExamList* list);
void printCandidate(ExamList* list, const char* id);
char calculateGrade(int score);
void freeList(ExamList* list);

#endif
