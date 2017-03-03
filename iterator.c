#include "keystore.h"

void keystore_iterator_init(struct keystore_iterator *inst, const struct keystore *ms)
{
	inst->assignment = ms->assignment;
	inst->delimiter = ms->delimiter;
	inst->begin = fstr_get(&ms->buffer);
	inst->end = fstr_get(&ms->buffer) + fstr_len(&ms->buffer);
	inst->cursor = NULL;
}

static const char *find_next(const char *begin, const char *end, const char *cursor, char pre, bool can_begin)
{
	if (begin == NULL || end == NULL) {
		return end;
	} else if (cursor == end) {
		return end;
	} else if (cursor == NULL) {
		if (can_begin) {
			return begin;
		} else {
			return find_next(begin, end, begin, pre, false);
		}
	} else if (*cursor == pre) {
		return cursor;
	} else {
		cursor = memchr(cursor, pre, end - cursor);
		if (cursor == NULL) {
			return end;
		}
		return cursor;
	}
}

void keystore_iterator_rewind(struct keystore_iterator *inst)
{
	inst->cursor = NULL;
}

static const char *find_range(struct keystore_iterator *inst, size_t *length, bool can_begin, char begin, char end)
{
	const char *start = find_next(inst->begin, inst->end, inst->cursor, begin, can_begin);
	if (start == inst->end) {
		goto eof;
	}
	if (*start == begin) {
		start++;
		if (start == inst->end) {
			goto eof;
		}
	}
	const char *finish = find_next(inst->begin, inst->end, start, end, false);
	if (finish == inst->end) {
		goto eof;
	}
	inst->cursor = finish;
	*length = finish - start;
	return start;
eof:
	*length = 0;
	inst->cursor = inst->end;
	return NULL;
}

const char *keystore_iterator_next_key(struct keystore_iterator *inst, size_t *length)
{
	return find_range(inst, length, true, inst->delimiter, inst->assignment);
}

const char *keystore_iterator_next_value(struct keystore_iterator *inst, size_t *length)
{
	return find_range(inst, length, false, inst->assignment, inst->delimiter);
}

bool keystore_iterator_next_key_f(struct keystore_iterator *inst, struct fstr *out)
{
	size_t len;
	const char *buf = keystore_iterator_next_key(inst, &len);
	if (buf) {
		fstr_set_ref_n(out, buf, len);
		return true;
	} else {
		fstr_clear(out);
		return false;
	}
}

bool keystore_iterator_next_value_f(struct keystore_iterator *inst, struct fstr *out)
{
	size_t len;
	const char *buf = keystore_iterator_next_value(inst, &len);
	if (buf) {
		fstr_set_ref_n(out, buf, len);
		return true;
	} else {
		fstr_clear(out);
		return false;
	}
}

bool keystore_iterator_next_pair_f(struct keystore_iterator *inst, struct fstr *key, struct fstr *val)
{
	return keystore_iterator_next_key_f(inst, key) && keystore_iterator_next_value_f(inst, val);
}

bool keystore_iterator_end(struct keystore_iterator *inst)
{
	return inst->cursor == inst->end;
}

void keystore_iterator_destroy(struct keystore_iterator *inst)
{
	memset(inst, 0, sizeof(*inst));
}
