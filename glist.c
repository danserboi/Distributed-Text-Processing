// SERBOI FLOREA-DAN 335CB

#include "glist.h"
#include <stdlib.h>
#include <stdio.h>

// insereaza la sfarsitul listei un element
void cons(void *element, list* l) {
	list temp = NULL;
	temp = calloc(1, sizeof(struct cell));
	if(temp == NULL) {
		fprintf(stderr, "Failed to allocate memory for new element!\n");
		exit(0);
	}
	temp->element = element;
	// conventia e ca ultimul element din lista sa nu aiba succesor
	temp->next = NULL;

	// daca lista era goala, elementul nou creat devine capul listei
	if(*l == NULL) {
		temp->prev = NULL;
		*l = temp;
	} else {
		// adaugam elementul la sfarsit
		temp->prev = (*l)->prev;
		if((*l)->prev != NULL) {
			(*l)->prev->next = temp;
		} else {
			(*l)->next = temp;
		}
		// primul element din lista are ultimul element ca predecesor
		(*l)->prev = temp;
	}
}

// sterge primul element
void del(list* l) {
	if(*l != NULL) {
		list temp = (*l)->next;
		// intai eliberam spatiul alocat elementului
		free((*l)->element);
		free(*l);
		*l = temp;
	} else {
		fprintf(stderr, "List is already empty! Cannot delete!\n");
	}
}

// elibereaza memoria alocata unei liste de elemente
void free_list(list* l){
	list temp = NULL;
	while(*l){
		temp = (*l)->next;
		// intai eliberam spatiul alocat elementului
		free((*l)->element);
		free(*l);
		*l = temp;
	}
}
