// SERBOI FLOREA-DAN 335CB

#ifndef _GLIST_
#define _GLIST_

typedef struct cell *list;

struct cell {
  void *element;
  list next, prev;
};

void cons(void *element, list* l);
void del(list* l);
void free_list(list* l);

#endif