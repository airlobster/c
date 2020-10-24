// dlist.c

#include "dlist.h"

typedef struct _dlist_node_t {
	struct _dlist_node_t *next, *prev;
	void* data;
} dlist_node_t;

typedef struct _dlist_context_t {
	dlist_node_t *head, *tail;
	dlist_dtor dtor;
	size_t len;
} dlist_context_t;

void dlist_init(dlist_dtor dtor, dlist_t** l) {
	dlist_context_t* x = (dlist_context_t*)malloc(sizeof(dlist_context_t));
	x->dtor = dtor;
	x->head = x->tail = 0;
	x->len = 0;
	*l = x;
}

void dlist_deinit(dlist_t* l) {
	dlist_reset(l);
	free(l);
}

void dlist_reset(dlist_t* l) {
	dlist_context_t* x = (dlist_context_t*)l;
	dlist_node_t* e = x->head;
	while( e ) {
		dlist_node_t* next = e->next;
		if( e->data && x->dtor )
			(*x->dtor)(e->data);
		free(e);
		e = next;
	}
	x->head = x->tail = 0;
	x->len = 0;
}

size_t dlist_len(dlist_t* l) {
	dlist_context_t* x = (dlist_context_t*)l;
	return x->len;
}

void dlist_push_back(dlist_t* l, void* e) {
	dlist_context_t* x = (dlist_context_t*)l;
	dlist_node_t* n = (dlist_node_t*)malloc(sizeof(dlist_node_t));
	n->next = 0;
	n->prev = x->tail;
	n->data = e;
	if( x->tail )
		x->tail->next = n;
	x->tail = n;
	if( ! x->head )
		x->head = n;
	x->len++;
}

void dlist_push_front(dlist_t* l, void* e) {
	dlist_context_t* x = (dlist_context_t*)l;
	dlist_node_t* n = (dlist_node_t*)malloc(sizeof(dlist_node_t));
	n->data = e;
	n->prev = 0;
	n->next = x->head;
	if( x->head )
		x->head->prev = n;
	x->head = n;
	if( ! x->tail )
		x->tail = n;
	++x->len;
}


int dlist_enum(dlist_t* l, int(*cb)(void* data, void* cookie), void* cookie) {
	dlist_context_t* x = (dlist_context_t*)l;
	int rc = 0;
	dlist_node_t* e = x->head;
	while( e && ! rc ) {
		dlist_node_t* next = e->next;
		rc = cb(e->data, cookie);
		e = next;
	}
	return rc;
}

void dlist_detach(dlist_t* l, size_t* n, void*** a) {
	int i=0;
	dlist_context_t* x = (dlist_context_t*)l;
	size_t _n = dlist_len(l);
	void** _a = (void**)malloc(_n * sizeof(void*));
	dlist_node_t* e = x->head;
	while( e ) {
		_a[i++] = e->data;
		e->data = 0; // detach from data
		e = e->next;
	}
	dlist_reset(l);
	*n = _n;
	*a = _a;
}

