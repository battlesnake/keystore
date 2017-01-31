#include "keystore.h"

size_t keystore_foreach(const struct keystore *ks, keystore_foreach_cb *cb, void *arg)
{
	struct keystore_iterator it;
	keystore_iterator_init(&it, ks);
	struct fstr key = FSTR_INIT;
	struct fstr val = FSTR_INIT;
	size_t count = 0;
	while (keystore_iterator_next_key_f(&it, &key) && keystore_iterator_next_value_f(&it, &val)) {
		if (!cb(arg, &key, &val)) {
			break;
		}
		count++;
	}
	keystore_iterator_destroy(&it);
	return count;
}

struct print_closure {
	char assignment;
	char delim;
	const struct fstr *prefix;
};

static bool print_cb(void *arg, const struct fstr *key, const struct fstr *val)
{
	const struct print_closure *pc = arg;
	printf(PRIfs PRIfs "%c" PRIfs "%c", prifs(pc->prefix), prifs(key), pc->assignment, prifs(val), pc->delim);
	return true;
}

size_t keystore_print(const struct keystore *ks, char assignment, char delim, const struct fstr *prefix)
{
	struct print_closure pc = { .assignment = assignment, .delim = delim, .prefix = prefix };
	if (!pc.prefix) {
		pc.prefix = &FSTR_INIT;
	}
	return keystore_foreach(ks, print_cb, &pc);
}
