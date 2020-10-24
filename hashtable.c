

#include <stdlib.h>
#include <string.h>
#include <regex.h>
#include "utils.h"
#include "hashtable.h"

#define	HASHTABLE_SIZE		(1024)

typedef unsigned long hash_t;

typedef struct _hashtable_node_t {
	char* key;
	void* value;
	struct _hashtable_node_t *next, *prev;
	hash_t hash;
} hashtable_node_t;

typedef struct _hashtable_impl_t {
	hashtable_node_t** a;
	void(*dtor)(void*);
	size_t n;
} hashtable_impl_t;

static hash_t _hash(const char *str) {
	hash_t hash = 5381;
	int c;
	while( (c=*str++) )
		hash = ((hash << 5) + hash) + (unsigned char)c;
	return hash;
}

static hashtable_node_t* _find(hashtable_impl_t* ht, const char* key) {
	hash_t hash = _hash(key);
	hashtable_node_t* e = ht->a[hash % HASHTABLE_SIZE];
	while( e ) {
		if( e->hash == hash )
			return e;
		e = e->next;
	}
	return 0;
}

static hashtable_node_t* _add(hashtable_impl_t* ht, const char* key, void* value) {
	hash_t hash = _hash(key);
	hashtable_node_t* eFirst = ht->a[hash % HASHTABLE_SIZE];
	hashtable_node_t* e = ALLOC(hashtable_node_t);
	e->key = strdup(key);
	e->value = value;
	e->hash = hash;
	e->next = eFirst;
	e->prev = 0;
	if( eFirst )
		eFirst->prev = e;
	ht->a[hash % HASHTABLE_SIZE] = e;
	ht->n++;
	return e;
}

int hashtable_init(void(*dtor)(void*), hashtable_t** ht) {
	int i;
	hashtable_impl_t* t = ALLOC(hashtable_impl_t);
	t->a = ALLOCA(hashtable_node_t*, HASHTABLE_SIZE);
	t->dtor = dtor;
	t->n = 0;
	for(i=0; i < HASHTABLE_SIZE; i++)
		t->a[i] = 0;
	*ht = t;
	return 0;
}

void hashtable_deinit(hashtable_t* ht) {
	hashtable_impl_t* t = (hashtable_impl_t*)ht;
	hashtable_reset(ht);
	free(t->a);
	free(t);
}

void hashtable_put(hashtable_t* ht, const char* key, void* value) {
	hashtable_impl_t* t = (hashtable_impl_t*)ht;
	hashtable_node_t* e = _find(t, key);
	if( e ) {
		if( e->value && t->dtor )
			(*t->dtor)(e->value);
		e->value = value;
	} else {
		e = _add(t, key, value);
	}
}

void* hashtable_get(hashtable_t* ht, const char* key) {
	hashtable_impl_t* t = (hashtable_impl_t*)ht;
	hashtable_node_t* e = _find(t, key);
	return e ? e->value : 0;
}

void hashtable_remove(hashtable_t* ht, const char* key) {
	hashtable_impl_t* t = (hashtable_impl_t*)ht;
	hashtable_node_t* e = _find(t, key);
	if( e ) {
		if( e->prev )
			e->prev->next = e->next;
		else
			t->a[e->hash % HASHTABLE_SIZE] = e->next;
		if( e->next )
			e->next->prev = e->prev;
		if( t->dtor )
			(*t->dtor)(e->value);
		FREE(e);
		t->n--;
	}
}

int hashtable_enum(hashtable_t* ht, const char* pat, hashtable_enum_callback_t cb, void* cookie) {
	hashtable_impl_t* t = (hashtable_impl_t*)ht;
	int i, err;
	regex_t re;
	if( (err=regcomp(&re, pat ? pat : ".*", REG_EXTENDED | REG_ICASE)) != 0 )
		return err;
	for(i=0; i < HASHTABLE_SIZE; i++) {
		hashtable_node_t* e = t->a[i];
		while( e ) {
			hashtable_node_t* next = e->next;
			if( regexec(&re, e->key, 0, 0, 0)==0 && cb(e->key, e->value, cookie) )
				break;
			e = next;
		}
	}
	regfree(&re);
	return 0;
}

size_t hashtable_size(hashtable_t* ht) {
	hashtable_impl_t* t = (hashtable_impl_t*)ht;
	return t->n;
}

static int _cb_keys(const char* key, void* value, void* cookie) {
	const char*** k = (const char***)cookie;
	**k = key;
	(*k)++;
	return 0;
}
static int _cb_cmp_key(const void* p1, const void* p2) {
	const char* k1 = *(const char**)p1;
	const char* k2 = *(const char**)p2;
	return strcasecmp(k1, k2);
}
int hashtable_enum_sorted(hashtable_t* ht, const char* pat, hashtable_enum_callback_t cb, void* cookie) {
	hashtable_impl_t* t = (hashtable_impl_t*)ht;
	int nout = 0, i;

	if( ! t->n )
		return 0;

	const char** keys = ALLOCA(const char*, t->n);

	// collect keys
	const char** k = keys;
	hashtable_enum(ht, pat, _cb_keys, &k);
	nout = k - keys;

	qsort(keys, nout, sizeof(char*), _cb_cmp_key);

	for(i=0; i < nout; i++) {
		hashtable_node_t* e = _find(t, keys[i]);
		cb(keys[i], e->value, cookie);
	}

	FREE(keys);
	return 0;
}

void hashtable_reset(hashtable_t* ht) {
	hashtable_impl_t* t = (hashtable_impl_t*)ht;
	int i;
	for(i=0; i < HASHTABLE_SIZE; i++) {
		hashtable_node_t* e = t->a[i];
		while( e ) {
			if( e->value && t->dtor )
				(*t->dtor)(e->value);
			hashtable_node_t* next = e->next;
			FREE(e);
			e = next;
		}
		t->a[i] = 0;
	}
	t->n = 0;
}

