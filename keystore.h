#pragma once
/* C codec for key/value list */
#include <cstd/std.h>
#include <fixedstr/fixedstr.h>

struct keystore {
	struct fstr buffer;
	char assignment;
	char delimiter;
};

struct keystore_iterator {
	char assignment;
	char delimiter;
	const char *begin;
	const char *end;
	const char *cursor;
};

/* Key store */

void keystore_init(struct keystore *inst, size_t capacity, size_t allocby);
void keystore_init_from(struct keystore *inst, size_t allocby, const char *data, size_t length);

void keystore_initcustom(struct keystore *inst, size_t capacity, size_t allocby, char assignment, char delimiter);
void keystore_initcustom_from(struct keystore *inst, const char *data, size_t length, size_t allocby, char assignment, char delimiter);

void keystore_append(struct keystore *inst, const char *key, const char *value);
void keystore_write(struct keystore *inst, const char *data, size_t length);

void keystore_clear(struct keystore *inst);

const char *keystore_data(const struct keystore *inst, size_t *length);
const char *keystore_lookup_key(const struct keystore *inst, const char *key);
const char *keystore_lookup(const struct keystore *inst, const char *key, size_t *length);

bool keystore_lookup_f(const struct keystore *inst, const struct fstr *key, struct fstr *value);
bool keystore_data_f(const struct keystore *inst, struct fstr *out);

bool keystore_lookup_key_cf(const struct keystore *inst, const char *key, struct fstr *out);
bool keystore_lookup_cf(const struct keystore *inst, const char *key, struct fstr *out);

void keystore_destroy(struct keystore *inst);

/* Iterator */
void keystore_iterator_init(struct keystore_iterator *inst, const struct keystore *ms);
void keystore_iterator_rewind(struct keystore_iterator *inst);

const char *keystore_iterator_next_key(struct keystore_iterator *inst, size_t *length);
const char *keystore_iterator_next_value(struct keystore_iterator *inst, size_t *length);

bool keystore_iterator_next_key_f(struct keystore_iterator *inst, struct fstr *out);
bool keystore_iterator_next_value_f(struct keystore_iterator *inst, struct fstr *out);
bool keystore_iterator_next_pair_f(struct keystore_iterator *inst, struct fstr *key, struct fstr *val);

bool keystore_iterator_end(struct keystore_iterator *inst);
void keystore_iterator_destroy(struct keystore_iterator *inst);

/* Utility */
typedef bool keystore_foreach_cb(void *arg, const struct fstr *key, const struct fstr *val);
size_t keystore_foreach(const struct keystore *ks, keystore_foreach_cb *cb, void *arg);

size_t keystore_print(const struct keystore *ks, char assignment, char delim, const struct fstr *prefix);
