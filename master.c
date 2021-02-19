// SERBOI FLOREA-DAN 335CB

#include "utils.h"
#include "master.h"

// trmite un paragraf catre worker si il primeste inapoi procesat
char* send_para(char* para, int para_lines, int worker_id, int* lines_beg) {
    // trimitere paragraf catre worker impreuna cu informatii legate de el
    int para_length = lines_beg[para_lines] + 1;
    MPI_Send(&para_length, 1, MPI_INT, worker_id, 0, MPI_COMM_WORLD);
    MPI_Send(&para_lines, 1, MPI_INT, worker_id, 0, MPI_COMM_WORLD);
    MPI_Send(para, para_length, MPI_CHAR, worker_id, 0, MPI_COMM_WORLD);
    MPI_Send(lines_beg, para_lines + 1, MPI_INT, worker_id, 0, MPI_COMM_WORLD);

    // primire paragraf modificat
    MPI_Status status;
    int new_para_length = 0;
    MPI_Recv(&new_para_length, 1, MPI_INT, worker_id, 0, MPI_COMM_WORLD,
             &status);
    char* proc_para = malloc(new_para_length);
    DIE(proc_para == NULL, "Failed to allocate!\n");
    MPI_Recv(proc_para, new_para_length, MPI_CHAR, worker_id, 0, MPI_COMM_WORLD,
             &status);

    return proc_para;
}

// functia citeste din fisierul de intrare si
// trimite paragrafele apartinand unui anumit topic spre procesare
// pe care le primeste procesate si le adauga in lista topicului respectiv
void read_topic(int worker_id, char* topic) {
    char line[CHUNK];
    char* para = NULL;
    int reading_para = 0;
    int para_lines = 0;
    // vom retine indexul unde incepe fiecare linie in timp ce citim paragraful
    // pentru ca un worker sa nu faca aceasta munca inca o data
    int lines_beg[MAX_NO_LINES];

    // citim fisierul si trimitem paragrafele cu topicul cerut catre worker
    FILE *file = fopen(in_file, "r");
    if (file == NULL) {
        perror("Error ");
    }

    while (fgets(line, sizeof(line), file)) {
        // ne aflam in modul de citire al unui paragraf cu topicul cerut
        if(reading_para == 1) {
            // am terminat de parcurs paragraful
            if(strcmp(line, "\n") == 0) {
                char* proc_para = send_para(para, para_lines, worker_id,
                                            lines_beg);
                // adaug paragraful in lista celor procesate
                cons(proc_para, &proc_paras[worker_id]);
                free(para);
                para = NULL;
                reading_para = 0;
                para_lines = 0;
            } else { // inca parcurgem paragraful
                // adaugam linia curenta la ce avem deja
                if(para == NULL) {
                    para = malloc(strlen(line) + 1);
                    DIE(para == NULL, "Failed to allocate!\n");
                    strcpy(para, line);
                } else {
                    para = realloc(para, (strlen(para) + strlen(line) + 1));
                    DIE(para == NULL, "Failed to allocate!\n");
                    strcat(para, line);
                }
                // incrementam contorul de linii pentru paragraful curent
                para_lines++;
                // marcam pozitia de inceput a linii curente
                lines_beg[para_lines] = strlen(para);
            }
        }

        // urmeaza un paragraf cu topicul cerut
        if(strcmp(line, topic) == 0) {
            reading_para = 1;
            // prima linie incepe de la pozitia 0
            lines_beg[para_lines] = 0;
        }
    }

    // daca ultimul paragraf a ramas netrimis
    // si ne aflam in timpul citirii unui paragraf, il trimitem catre worker
    if(reading_para == 1) {
        lines_beg[para_lines] = strlen(para);
        char* proc_para = send_para(para, para_lines, worker_id, lines_beg);
        cons(proc_para, &proc_paras[worker_id]);
        free(para);
    }


    // trimitem un mesaj prin care specificam ca am terminat procesarea
    int processing = 0;
    MPI_Send(&processing, 1, MPI_INT, worker_id, 0, MPI_COMM_WORLD);

    fclose(file);
}

// functie executata de fiecare thread pornit din MASTER
// ea apeleaza functia care citeste paragrafele pentru un anumit topic
void *master_func(void *arg) {
	int thread_id = *(int *)arg;

    if(thread_id == HORROR) {
        read_topic(HORROR, "horror\n");
    } else if(thread_id == COMEDY) {
        read_topic(COMEDY, "comedy\n");
    } else if(thread_id == FANTASY) {
        read_topic(FANTASY, "fantasy\n");
    } else if(thread_id == SCIFI) {
        read_topic(SCIFI, "science-fiction\n");
    }

	pthread_exit(NULL);
}

// citeste paragrafele din fisierul de intrare
// si scrie varianta lor modificata in fisierul de iesire
void write_result() {
    char line[CHUNK];
    FILE *in = fopen(in_file, "r");
    if (in == NULL) {
        perror("Error ");
    }

    FILE *out = fopen(out_file, "w");
    if (out == NULL) {
        perror("Error ");
    }

    while (fgets(line, sizeof(line), in)) {
        if(strcmp(line, "horror\n") == 0) {
            fputs(line, out);
            fputs((char*)proc_paras[HORROR]->element, out);
            del(&proc_paras[HORROR]);
            fputs("\n", out);
        } else if(strcmp(line, "comedy\n") == 0) {
            fputs(line, out);
            fputs((char*)proc_paras[COMEDY]->element, out);
            del(&proc_paras[COMEDY]);
            fputs("\n", out);
        } else if(strcmp(line, "fantasy\n") == 0) {
            fputs(line, out);
            fputs((char*)proc_paras[FANTASY]->element, out);
            del(&proc_paras[FANTASY]);
            fputs("\n", out);
        } else if(strcmp(line, "science-fiction\n") == 0) {
            fputs(line, out);
            fputs((char*)proc_paras[SCIFI]->element, out);
            del(&proc_paras[SCIFI]);
            fputs("\n", out);
        }
    }

    fclose(in);
    fclose(out);

    free(proc_paras[HORROR]);
    free(proc_paras[COMEDY]);
    free(proc_paras[FANTASY]);
    free(proc_paras[SCIFI]);

    free(proc_paras);
}