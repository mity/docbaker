/*
 * DocBaker
 * (http://github.com/mity/docbaker)
 *
 * Copyright (c) 2017 Martin Mitas
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include "dict.h"
#include "fnv1a.h"


static inline uint32_t
dict_hash(const char* keyname, size_t key_len)
{
    return fnv1a_32(FNV1A_BASE_32, keyname, key_len);
}


/************************
 *** Global key store ***
 ************************/

/* To prevent wasting memory by frequent reusing of some keys in various
 * dictionaries, we store all the keys in use in the single storage and
 * all dictionaries just store pointers into this storage. */

typedef struct DICT_KEY {
    struct DICT_KEY* next;
    uint32_t n_refs;
    uint32_t hash;
    char keyname[1];
} DICT_KEY;

typedef struct DICT_KEY_STORE {
    DICT_KEY** buckets;
    size_t n_buckets;
    size_t n_members;
} DICT_KEY_STORE;


static DICT_KEY_STORE dict_key_store = { 0 };


static DICT_KEY*
dict_key_register(uint32_t hash, const char* keyname, size_t key_len)
{
    DICT_KEY* key;

    /* Grow the key store if there are too many keys per bucket. */
    if(dict_key_store.n_members >= dict_key_store.n_buckets * 4) {
        size_t new_n_buckets = dict_key_store.n_members * 2;
        DICT_KEY** new_buckets;
        size_t i;

        if(new_n_buckets == 0)
            new_n_buckets = 4;

        new_buckets = (DICT_KEY**) malloc(sizeof(DICT_KEY*) * new_n_buckets);
        memset(new_buckets, 0, sizeof(DICT_KEY*) * new_n_buckets);

        for(i = 0; i < dict_key_store.n_buckets; i++) {
            key = dict_key_store.buckets[i];
            while(key != NULL) {
                DICT_KEY* tmp = key->next;

                key->next = new_buckets[key->hash % new_n_buckets];
                new_buckets[key->hash % new_n_buckets] = key;

                key = tmp;
            }
        }
        free(dict_key_store.buckets);
        dict_key_store.buckets = new_buckets;
        dict_key_store.n_buckets = new_n_buckets;
    }

    /* Try to reuse an existing key. */
    key = dict_key_store.buckets[hash % dict_key_store.n_buckets];
    while(key != NULL) {
        if(hash == key->hash  &&  strcmp(keyname, key->keyname) == 0) {
            key->n_refs++;
            return key;
        }

        key = key->next;
    }

    /* Create new key. */
    key = (DICT_KEY*) malloc(offsetof(DICT_KEY, keyname) + key_len + 1);
    key->next = dict_key_store.buckets[hash % dict_key_store.n_buckets];
    key->n_refs = 1;
    key->hash = hash;
    memcpy(key->keyname, keyname, key_len + 1);

    dict_key_store.buckets[hash % dict_key_store.n_buckets] = key;
    dict_key_store.n_members++;

    return key;
}

static void
dict_key_unregister(DICT_KEY* key)
{
    DICT_KEY* key_prev;

    key->n_refs--;

    if(key->n_refs > 0)
        return;

    /* Find previous key. */
    if(key == dict_key_store.buckets[key->hash % dict_key_store.n_buckets]) {
        key_prev = NULL;
    } else {
        key_prev = dict_key_store.buckets[key->hash % dict_key_store.n_buckets];
        while(key_prev->next != key)
            key_prev = key_prev->next;
    }

    /* Detach the key from the bucket. */
    if(key_prev != NULL)
        key_prev->next = key->next;
    else
        dict_key_store.buckets[key->hash % dict_key_store.n_buckets] = key->next;

    free(key);
    dict_key_store.n_members--;

    if(dict_key_store.n_members == 0) {
        free(dict_key_store.buckets);
        dict_key_store.n_buckets = 0;
    }
}


/******************
 *** Dictionary ***
 ******************/


typedef struct DICT_BUCKET {
    struct DICT_BUCKET* next;
    DICT_KEY* key;
    void* value;
} DICT_BUCKET;


void
dict_init(DICT* dict)
{
    dict->buckets = NULL;
    dict->n_buckets = 0;
    dict->n_members = 0;
}

void
dict_fini(DICT* dict, DICT_DTORFUNC dtor_func)
{
    size_t i;

    for(i = 0; i < dict->n_buckets; i++) {
        DICT_BUCKET* b = dict->buckets[i];

        while(b != NULL) {
            DICT_BUCKET* tmp = b->next;

            dict_key_unregister(b->key);
            if(dtor_func != NULL)
                dtor_func(b->value);
            free(b);

            b = tmp;
        }
    }
}


static DICT_BUCKET*
dict_find(DICT* dict, uint32_t hash, const char* key, DICT_BUCKET** p_prev)
{
    DICT_BUCKET* b;
    DICT_BUCKET* prev = NULL;

    if(dict->n_buckets == 0)
        return NULL;

    b = dict->buckets[hash % dict->n_buckets];

    while(b != NULL) {
        if(hash == b->key->hash  &&  strcmp(key, b->key->keyname) == 0) {
            if(p_prev != NULL)
                *p_prev = prev;
            return b;
        }
        prev = b;
        b = b->next;
    }

    return NULL;
}

void
dict_set(DICT* dict, const char* key, void* value, DICT_DTORFUNC dtor_func)
{
    size_t key_len;
    uint32_t hash;
    DICT_BUCKET* b;

    key_len = strlen(key);
    hash = dict_hash(key, key_len);

    /* Try to reset an existing value. */
    b = dict_find(dict, hash, key, NULL);
    if(b != NULL) {
        if(dtor_func != NULL)
            dtor_func(b->value);
        b->value = value;
        return;
    }

    /* Grow the key store if there are too many keys per bucket. */
    dict->n_members++;
    if(dict->n_members > dict->n_buckets * 4) {
        size_t new_n_buckets = dict->n_members * 2;
        DICT_BUCKET** new_buckets;
        size_t i;

        new_buckets = (DICT_BUCKET**) malloc(sizeof(DICT_BUCKET*) * new_n_buckets);
        memset(new_buckets, 0, sizeof(DICT_BUCKET*) * new_n_buckets);

        for(i = 0; i < dict->n_buckets; i++) {
            b = dict->buckets[i];
            while(b != NULL) {
                DICT_BUCKET* tmp = b->next;

                b->next = new_buckets[b->key->hash % new_n_buckets];
                new_buckets[b->key->hash % new_n_buckets] = b;

                b = tmp;
            }
        }

        free(dict->buckets);
        dict->buckets = new_buckets;
        dict->n_buckets = new_n_buckets;
    }

    /* Add new value. */
    b = (DICT_BUCKET*) malloc(sizeof(DICT_BUCKET));
    b->next = dict->buckets[hash % dict->n_buckets];
    b->key = dict_key_register(hash, key, key_len);
    b->value = value;
    dict->buckets[hash % dict->n_buckets] = b;
}

void*
dict_get(DICT* dict, const char* key)
{
    size_t key_len;
    uint32_t hash;
    DICT_BUCKET* b;

    key_len = strlen(key);
    hash = dict_hash(key, key_len);
    b = dict_find(dict, hash, key, NULL);
    if(b != NULL)
        return b->value;
    return NULL;
}

void
dict_collect_keys(DICT* dict, ARRAY* array)
{
    size_t i;

    for(i = 0; i < dict->n_buckets; i++) {
        DICT_BUCKET* b = dict->buckets[i];
        while(b != NULL) {
            array_append(array, b->key->keyname);
            b = b->next;
        }
    }
}
