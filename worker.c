// SERBOI FLOREA-DAN 335CB

#include "utils.h"
#include "worker.h"

// verifica daca caracterul este o litera
int is_letter(char ch) {
    if((ch >= 65 && ch <= 90) || (ch >= 97 && ch <= 122)) {
        return 1;
    } else {
        return 0;
    }
}

// verifica daca caracterul este o consoana
int is_consonant(char ch) {
    if(is_letter(ch)) {
        if(ch == 'a' || ch == 'e' || ch == 'i' || ch == 'o' || ch == 'u'
        || ch == 'A' || ch == 'E' || ch == 'I' || ch == 'O' || ch == 'U') {
            return 0;
        } else {
            return 1;
        }
    }
    return 0;
}

// returneaza minuscula unei litere din alfabet
char lowercase(char letter) {
    if(letter >= 65 && letter <= 90) {
        return letter + 32;
    } else {
        return letter;
    }
}

// returneaza majuscula unei litere din alfabet
char uppercase(char letter) {
    if(letter >= 65 && letter <= 90) {
        return letter;
    } else {
        return letter - 32;
    }
}

// functia se ocupa de receptionarea paragrafelor
// apartinand unui topic si trimiterea lor procesate inapoi catre MASTER
void paras_receiving(int topic) {
    while(1) {
        // primim lungimea paragrafului
        MPI_Status status;
        MPI_Recv(&recv_para_length, 1, MPI_INT, MASTER, 0, MPI_COMM_WORLD,
                 &status);

        // daca lungimea e 0, atunci s-a terminat procesarea
        if(recv_para_length == 0) {
            // marcam celorlalte thread-uri acest lucru
            proc_ended = 1;
            // deblocam thread-urile de procesare care asteapta un paragraf
            pthread_barrier_wait(&barrier);
            break;
        }

        // primim restul de informatii legate de paragraf
        recv_para = malloc(recv_para_length);
        DIE(recv_para == NULL, "Failed to allocate!\n");
        MPI_Recv(&recv_para_lines, 1, MPI_INT, MASTER, 0, MPI_COMM_WORLD,
                 &status);
        MPI_Recv(recv_para, recv_para_length, MPI_CHAR, MASTER, 0,
                 MPI_COMM_WORLD, &status);
        MPI_Recv(lines_beg, recv_para_lines + 1, MPI_INT, MASTER, 0,
                 MPI_COMM_WORLD, &status);

        // daca topicul este HORROR, in cel mai rau caz, pur teoretic,
        // putem avea o lungime dubla pentru paragraful procesat
        // altfel, lungimea ramane intacta
        if(topic == HORROR) {
            proc_para = malloc(2 * recv_para_length + 1);
            DIE(proc_para == NULL, "Failed to allocate!\n");
            proc_para[0] = '\0';
        } else {
            proc_para = malloc(recv_para_length + 1);
            DIE(proc_para == NULL, "Failed to allocate!\n");
            proc_para[recv_para_length - 1] = '\0';
        }

        // se trece de bariera atunci cand se poate incepe procesarea
        pthread_barrier_wait(&barrier);

        // se trece de bariera atunci cand procesarea a luat sfarsit
        pthread_barrier_wait(&barrier);

        // pentru topicul horror, liniile prelucrate au fost stocate separat
        // si le concatenam pentru a obtine rezultatul final
        // celelalte au lucrat direct pe memorie alocata paragrafului procesat
        if(topic == HORROR) {
            for(int i = 0; i < recv_para_lines; i++) {
                strcat(proc_para, proc_lines[i]);
                free(proc_lines[i]);
            }
        }

        // trimitem catre procesul MASTER paragraful procesat
        int new_length = strlen(proc_para) + 1;
        MPI_Send(&new_length, 1, MPI_INT, MASTER, 0, MPI_COMM_WORLD);
        MPI_Send(proc_para, new_length, MPI_CHAR, MASTER, 0, MPI_COMM_WORLD);

        free(recv_para);
        free(proc_para);
    }
}

// urmatoarele 4 functii proceseaza o linie dintr-un paragraf al unui topic

void process_horror_line(int line) {
    // pentru paragrafele HORROR construim separat in memorie noile linii
    proc_lines[line] = calloc(2 * (lines_beg[line + 1] - lines_beg[line]) + 1,
                           sizeof(char));
    DIE(proc_lines[line] == NULL, "Failed to allocate!\n");
    // pos reprezinta cursorul pentru noua linie
    int pos = 0;

    for(int j = lines_beg[line]; j < lines_beg[line + 1]; j++, pos++) {
        proc_lines[line][pos] = recv_para[j];
        // dublam consoanele in fiecare cuvant
        if(is_consonant(recv_para[j])) {
            pos++;
            proc_lines[line][pos] = lowercase(recv_para[j]);
        }
    }

    proc_lines[line][pos] = '\0';
}

void process_comedy_line(int line) {
    // word_pos reprezinta cursorul in cadrul cuvantului curent
    int word_pos = 1;

    for(int j = lines_beg[line]; j < lines_beg[line + 1]; j++) {
        // resetam numaratoarea caracterelor in cadrul cuvantului urmator
        if(recv_para[j] == ' ' || recv_para[j] == '\n') {
            proc_para[j] = recv_para[j];
            word_pos = 1;
        } else {
            // fiecare litera de pe pozitie para devine majuscula
            if(is_letter(recv_para[j]) && word_pos % 2 == 0) {
                proc_para[j] = uppercase(recv_para[j]);
            } else {
                proc_para[j] = recv_para[j];
            }
            word_pos++;
        }
    }
}

void process_fantasy_line(int line) {
    // word_pos reprezinta cursorul in cadrul cuvantului curent
    int word_pos = 1;

    for(int j = lines_beg[line]; j < lines_beg[line + 1]; j++) {
        // resetam numaratoarea caracterelor in cadrul cuvantului urmator
        if(recv_para[j] == ' ' || recv_para[j] == '\n') {
            proc_para[j] = recv_para[j];
            word_pos = 1;
        } else {
            // prima litera a fiecarui cuvant devine majuscula
            if(word_pos == 1 && is_letter(recv_para[j])) {
                proc_para[j] = uppercase(recv_para[j]);
            } else {
                proc_para[j] = recv_para[j];
            }
            word_pos++;
        }
    }
}

void process_scifi_line(int line) {
    // retinem numarul de cuvinte in cadrul liniei
    int word_count = 1;

    for(int j = lines_beg[line]; j < lines_beg[line + 1]; j++) {
        if(recv_para[j] == ' ') {
            proc_para[j] = recv_para[j];
            // urmeaza un cuvant nou
            word_count++;
        }
        else if(recv_para[j] == '\n') {
            proc_para[j] = recv_para[j];
        } else {
            // inversam fiecare al 7-lea cuvant de pe linie
            if(word_count % 7 == 0) {
                // determinam lungimea cuvantului
                int length = 1;
                while(j + length < recv_para_length
                      && recv_para[j + length] != ' '
                      && recv_para[j + length] != '\n') {
                    length++;
                }
                // calculam inversul cuvantului
                for(int k = 0; k < length; k++) {
                    proc_para[j + k] = recv_para[j + length - 1 - k];
                }
                // sarim peste cuvant
                j += length - 1;
            } else {
                proc_para[j] = recv_para[j];
            }
        }
    }
}

// prelucreaza liniile din paragraf repartizate thread-ului
void process_lines(int thread_id, int topic) {
    // thread-urile cu ID-ul de la 2 la P prelucreaza textul
    while(!proc_ended) {
        // marcam faptul ca suntem dispusi sa procesam un paragraf
        // trecem de bariera doar cand exista un paragraf de procesat
        pthread_barrier_wait(&barrier);
        // thread-ul intai verifica daca procesarea a luat sfarsit
        if(proc_ended) {
            break;
        }

        // vom calcula doar liniile repartizate noua
        int curr_line = (thread_id - 2) * NO_LINES_PROC;

        // procesam liniile repartizate noua
        while(curr_line < recv_para_lines) {
            for(int i = curr_line; i < recv_para_lines
                && i < curr_line + NO_LINES_PROC; i++) {
                    if(topic == HORROR) {
                        process_horror_line(i);
                    } else if(topic == COMEDY) {
                        process_comedy_line(i);
                    } else if(topic == FANTASY) {
                        process_fantasy_line(i);
                    } else if(topic == SCIFI) {
                        process_scifi_line(i);
                    }
            }
            // daca e cazul, trecem la urmatorul calup de linii repartizat
            curr_line += (P - 1) * NO_LINES_PROC;
        }

        // marcam faptul ca ne-am terminat treaba pentru acest paragraf
        pthread_barrier_wait(&barrier);
    }
}

// urmatoarele 4 functii de thread sunt specifice fiecarui WORKER(topic)
// avem un thread din worker care se ocupa de primirea paragrafului
// si trimiterea lor dupa ce au fost procesate
// iar celelalte thread-uri, cu ID-ul de la 2 la P, prelucreaza paragraful

void *horror_func(void *arg) {
	int thread_id = *(int *)arg;

    if(thread_id == RECEIVER) {
        paras_receiving(HORROR);
    } else {
        process_lines(thread_id, HORROR);
    }

	pthread_exit(NULL);
}

void *comedy_func(void *arg) {
	int thread_id = *(int *)arg;

    if(thread_id == RECEIVER) {
        paras_receiving(COMEDY);
    } else {
        process_lines(thread_id, COMEDY);
    }

	pthread_exit(NULL);
}

void *fantasy_func(void *arg) {
	int thread_id = *(int *)arg;

    if(thread_id == RECEIVER) {
        paras_receiving(FANTASY);
    } else {
       process_lines(thread_id, FANTASY);
    }

	pthread_exit(NULL);
}

void *scifi_func(void *arg) {
	int thread_id = *(int *)arg;

    if(thread_id == RECEIVER) {
        paras_receiving(SCIFI);
    } else {
        process_lines(thread_id, SCIFI);
    }

	pthread_exit(NULL);
}