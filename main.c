// SERBOI FLOREA-DAN 335CB

#include "utils.h"
#include "master.h"
#include "worker.h"

// denumirea fisierului de intrare
char* in_file = NULL;
// denumirea fisierului de iesire
char* out_file = NULL;

// retinem in MASTER, pentru fiecare topic,
// cate o lista cu paragrafele procesate
list* proc_paras = NULL;

// retinem in workeri, pentru ca thread-urile sa aiba acces la nivel global
// urmatoarele variabile:
// nr de fire de executie disponibile pe sistemul pe care ruleaza nodul
int P = 0;
// bariera pentru a sincroniza
// thread-ul care primeste paragraful de la master si pe care-l trimite procesat
// cu celelalte P-1 thread-uri care proceseaza liniile repartizate lor
pthread_barrier_t barrier;
// informatiile legate de un paragraf primit
int recv_para_length = 0;
char* recv_para = NULL;
int recv_para_lines = 0;
int lines_beg[MAX_NO_LINES];
// in cazul topicului HORROR, liniile sunt calculate in alta zona de memorie
char* proc_lines[MAX_NO_LINES];
// pentru celelalte topic-uri, liniile sunt calculate direct in zona
// paragrafului procesat care va fi trimis catre MASTER
char* proc_para = NULL;
// variabila marcheaza ca nu mai exista paragrafe de procesat pentru worker
int proc_ended = 0;

// creeaza un anumit numar de thread-uri care realizeaza o functie data
// si pentru care se asteapta terminarea lor folosind join
void create_threads(int no_threads, void *(*func) (void *)) {
    int i, r;
    void *status;
    pthread_t* threads = malloc(sizeof(pthread_t) * no_threads);
    DIE(threads == NULL, "Failed to allocate!\n");
    int* arguments = malloc(sizeof(int) * no_threads);
    DIE(arguments == NULL, "Failed to allocate!\n");

    for (i = 0; i < no_threads; i++) {
        arguments[i] = i + 1;
        r = pthread_create(&threads[i], NULL, func, &arguments[i]);

        if (r) {
            printf("Eroare la crearea thread-ului %d\n", arguments[i]);
            exit(-1);
        }
    }

    for (i = 0; i < no_threads; i++) {
        r = pthread_join(threads[i], &status);

        if (r) {
            printf("Eroare la asteptarea thread-ului %d\n", arguments[i]);
            exit(-1);
        }
    }

    free(threads);
    free(arguments);
}

int main (int argc, char *argv[]) {
    P = sysconf(_SC_NPROCESSORS_CONF);
    int numtasks, rank;
    int provided;

    MPI_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE, &provided);
    MPI_Comm_size(MPI_COMM_WORLD, &numtasks);
    MPI_Comm_rank(MPI_COMM_WORLD,&rank);

    in_file = argv[1];

    // construim fisierul de iesire
    out_file = malloc(strlen(argv[1]) + 1);
    DIE(out_file == NULL, "Failed to allocate!\n");
    strcpy(out_file, in_file);
    strcpy(out_file + strlen(out_file) - 3, "out");

    // pentru fiecare nod cream numarul de thread-uri dorit
    // care vor efectua operatiile caracteristice lor
    if(rank == MASTER) {
        // vom retine pentru fiecare topic cate o lista cu paragrafele procesate
        proc_paras = calloc(sizeof(list *), (NUM_THREADS + 1));
        DIE(proc_paras == NULL, "Failed to allocate!\n");
        create_threads(NUM_THREADS, master_func);
        // scriem rezultatul
        write_result();
    } else if(rank == HORROR) {
        pthread_barrier_init(&barrier, NULL, P);
        create_threads(P, horror_func);
        pthread_barrier_destroy(&barrier);
    } else if(rank == COMEDY) {
        pthread_barrier_init(&barrier, NULL, P);
        create_threads(P, comedy_func);
        pthread_barrier_destroy(&barrier);
    } else if(rank == FANTASY) {
        pthread_barrier_init(&barrier, NULL, P);
        create_threads(P, fantasy_func);
        pthread_barrier_destroy(&barrier);
    } else if(rank == SCIFI) {
        pthread_barrier_init(&barrier, NULL, P);
        create_threads(P, scifi_func);
        pthread_barrier_destroy(&barrier);
    }

    free(out_file);

    MPI_Finalize();
}
