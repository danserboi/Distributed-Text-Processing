// SERBOI FLOREA-DAN 335CB

#ifndef _WORKER_
#define _WORKER_

extern char* in_file;

extern int P;
extern pthread_barrier_t barrier;
extern int recv_para_length;
extern char* recv_para;
extern int recv_para_lines;
extern int lines_beg[MAX_NO_LINES];
extern char* proc_lines[MAX_NO_LINES];
extern char* proc_para;
extern int proc_ended;

int is_letter(char ch);

int is_consonant(char ch);

char lowercase(char letter);

char uppercase(char letter);

void paras_receiving(int topic);

void process_horror_line(int line);

void process_comedy_line(int line);

void process_fantasy_line(int line);

void process_scifi_line(int line);

void process_lines(int thread_id, int topic);

void *horror_func(void *arg);

void *comedy_func(void *arg);

void *fantasy_func(void *arg);

void *scifi_func(void *arg);

#endif