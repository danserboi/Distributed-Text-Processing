// SERBOI FLOREA-DAN 335CB

#ifndef _MASTER_
#define _MASTER_

extern char* in_file;
extern char* out_file;

extern list* proc_paras;

char* send_para(char* para, int para_lines, int worker_id, int* lines_beg);

void read_topic(int worker_id, char* topic);

void *master_func(void *arg);

void write_result();

#endif