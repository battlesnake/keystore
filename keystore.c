#if 0
(
set -euo pipefail
declare -r out="$(mktemp)"
trap 'rm -f -- "$out"' EXIT ERR
gcc -Wall -Wextra -Werror -DTEST_keystore_c -std=gnu11 -I./c_modules -O3 -o "$out" $(find . -name '*.c')
valgrind --leak-check=full --track-origins=yes --quiet "$out"
)
exit 0
#endif
#include "keystore_defs.h"
#include "keystore.h"

static void set_length(struct keystore *inst, size_t length)
{
	fstr_resize(&inst->buffer, length);
}

void keystore_initcustom(struct keystore *inst, size_t capacity, size_t allocby, char assignment, char delimiter)
{
	(void) capacity; /* TODO capacity for fixedstr */
	(void) allocby; /* TODO allocby for fixedstr - use "buffer" as backing for fstr? */
	fstr_init(&inst->buffer);
	inst->assignment = assignment;
	inst->delimiter = delimiter;
}

void keystore_initcustom_from(struct keystore *inst, const char *data, size_t length, size_t allocby, char assignment, char delimiter)
{
	keystore_initcustom(inst, length, allocby, assignment, delimiter);
	set_length(inst, length);
	memcpy(fstr_get_mutable(&inst->buffer), data, length);
}

void keystore_init(struct keystore *inst, size_t capacity, size_t allocby)
{
	keystore_initcustom(inst, capacity, allocby, kv_delim, line_delim);
}

void keystore_init_from(struct keystore *inst, size_t allocby, const char *data, size_t length)
{
	(void) allocby;
	keystore_initcustom_from(inst, data, length, allocby, kv_delim, line_delim);
}

void keystore_append(struct keystore *inst, const char *key, const char *value)
{
	fstr_format_append(&inst->buffer, "%s%c%s%c", key, inst->assignment, value, inst->delimiter);
}

void keystore_write(struct keystore *inst, const char *data, size_t length)
{
	fstr_append_n(&inst->buffer, data, length);
}

const char *keystore_data(const struct keystore *inst, size_t *length)
{
	/* No instance passed */
	if (inst == NULL) {
		if (length) {
			*length = 0;
		}
		return NULL;
	}
	/* Return buffer */
	if (length) {
		*length = fstr_len(&inst->buffer);
	}
	return fstr_get(&inst->buffer);
}

const char *keystore_lookup_key(const struct keystore *inst, const char *key)
{
	const size_t klen = strlen(key);
	const size_t length = fstr_len(&inst->buffer);
	if (klen + 2 > length) {
		return NULL;
	}
	const char *begin = fstr_get(&inst->buffer);
	const char *end = begin + length - 2 - klen;
	bool start = true;
	for (const char *it = begin; it < end; it++) {
		if (start && memcmp(it, key, klen) == 0 && it[klen] == inst->assignment) {
			return it;
		}
		start = *it == inst->delimiter;
	}
	return NULL;
}

const char *keystore_lookup(const struct keystore *inst, const char *key, size_t *length)
{
	if (length) {
		*length = 0;
	}
	const size_t klen = strlen(key);
	const char *key_pos = keystore_lookup_key(inst, key);
	if (key_pos == NULL) {
		return NULL;
	}
	const char *value_pos = key_pos + klen + 1;
	if (length) {
		size_t max_len = fstr_get(&inst->buffer) + fstr_len(&inst->buffer) - value_pos;
		const char *end = memchr(value_pos, inst->delimiter, max_len);
		if (end == NULL) {
			return NULL;
		}
		*length = end - value_pos;
	}
	return value_pos;
}

bool keystore_lookup_f(const struct keystore *inst, const struct fstr *key, struct fstr *value)
{
	size_t length;
	const char *p = keystore_lookup(inst, fstr_get(key), &length);
	if (!p) {
		return false;
	}
	fstr_set_ref_n(value, p, length);
	return true;
}

bool keystore_data_f(const struct keystore *inst, struct fstr *out)
{
	size_t len;
	const char *buf = keystore_data(inst, &len);
	if (buf) {
		fstr_set_ref_n(out, buf, len);
		return true;
	} else {
		fstr_clear(out);
		return false;
	}
}

bool keystore_lookup_key_cf(const struct keystore *inst, const char *key, struct fstr *out)
{
	size_t len = strlen(key);
	const char *buf = keystore_lookup_key(inst, key);
	if (buf) {
		fstr_set_ref_n(out, buf, len);
		return true;
	} else {
		fstr_clear(out);
		return false;
	}
}

bool keystore_lookup_cf(const struct keystore *inst, const char *key, struct fstr *out)
{
	size_t len;
	const char *buf = keystore_lookup(inst, key, &len);
	if (buf) {
		fstr_set_ref_n(out, buf, len);
		return true;
	} else {
		fstr_clear(out);
		return false;
	}
}

void keystore_clear(struct keystore *inst)
{
	fstr_resize(&inst->buffer, 0);
}

void keystore_destroy(struct keystore *inst)
{
	fstr_destroy(&inst->buffer);
}

#if defined TEST_keystore_c
int main(int argc, char *argv[])
{
	(void) argc;
	(void) argv;
	struct keystore i, o;
	keystore_init(&i, 256, 256);
	keystore_append(&i, "key", "value");
	keystore_append(&i, "vegetable", "potato");
	keystore_append(&i, "color", "purple");
	size_t len;
	const char *buf = keystore_data(&i, &len);
	printf("\n");

	printf("Buffer: ");
	for (size_t i = 0; i < len; i++) {
		if (buf[i]) {
			printf("%c", buf[i]);
		} else {
			printf("; ");
		}
	}
	printf("\n");
	keystore_init_from(&o, 256, buf, len);
	keystore_destroy(&i);
	printf("\n");

	printf("Lookup:\n");
	printf(" * key/val: %s\n", keystore_lookup_key(&o, "key"));
	printf(" * key: %s\n", keystore_lookup(&o, "key", NULL));
	printf(" * vegetable: %s\n", keystore_lookup(&o, "vegetable", NULL));
	printf(" * color: %s\n", keystore_lookup(&o, "color", NULL));
	printf("\n");

	{
	printf("Printer:\n");
	struct fstr prefix;
	fstr_init_ref(&prefix, " - ");
	keystore_print(&o, '=', '\n', &prefix);
	printf("\n");
	keystore_print(&o, '=', '\n', NULL);
	printf("\n");
	}

	{
	printf("Iterator:\n");
	struct keystore_iterator it;
	keystore_iterator_init(&it, &o);
	while (!keystore_iterator_end(&it)) {
		const char *k, *v;
		size_t kl, vl;
		k = keystore_iterator_next_key(&it, &kl);
		v = keystore_iterator_next_value(&it, &vl);
		if (k != NULL && v != NULL) {
			printf(" * %.*s: %.*s\n", (int) kl, k, (int) vl, v);
		}
	}
	keystore_iterator_destroy(&it);
	printf("\n");
	}

	{
	printf("Iterator2:\n");
	struct fstr fk = FSTR_INIT;
	struct fstr fv = FSTR_INIT;
	struct keystore_iterator it;
	keystore_iterator_init(&it, &o);
	while (keystore_iterator_next_pair_f(&it, &fk, &fv)) {
		printf(" * " PRIfs ": " PRIfs "\n", prifs(&fk), prifs(&fv));
	}
	keystore_iterator_destroy(&it);
	fstr_destroy(&fv);
	fstr_destroy(&fk);
	printf("\n");
	}


	keystore_destroy(&o);
}
#endif
