
#ifndef _HASHTABLE_H_
#define _HASHTABLE_H_

typedef void hashtable_t;

typedef int(*hashtable_enum_callback_t)(const char* key, void* value, void* cookie);

int hashtable_init(void(*dtor)(void*), hashtable_t** ht);
void hashtable_deinit(hashtable_t* ht);
void hashtable_reset(hashtable_t* ht);
size_t hashtable_size(hashtable_t* ht);
void hashtable_put(hashtable_t* ht, const char* key, void* value);
void* hashtable_get(hashtable_t* ht, const char* key);
void hashtable_remove(hashtable_t* ht, const char* key);
int hashtable_enum(hashtable_t* ht, const char* pat, hashtable_enum_callback_t cb, void* cookie);
int hashtable_enum_sorted(hashtable_t* ht, const char* pat, hashtable_enum_callback_t cb, void* cookie);

#endif
