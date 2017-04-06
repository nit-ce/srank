/* sidx: save and assign an index to each given string */
#include <stdlib.h>
#include <string.h>
#include "srank.h"

/* string indexing struct */
struct sidx {
	char **tab;
	void **dat;
	int len;
	int cnt;
};

/* allocate an sidx struct */
struct sidx *sidx_make(void)
{
	struct sidx *sidx;
	sidx = malloc(sizeof(*sidx));
	memset(sidx, 0, sizeof(*sidx));
	return sidx;
}

static void *mextend(void *old, long oldsz, long newsz, int memsz)
{
	void *new = malloc(newsz * memsz);
	memcpy(new, old, oldsz * memsz);
	memset(new + oldsz * memsz, 0, (newsz - oldsz) * memsz);
	free(old);
	return new;
}

/* free the given sidx struct */
void sidx_free(struct sidx *sidx)
{
	int i;
	for (i = 0; i < sidx->len; i++)
		free(sidx->tab[i]);
	free(sidx->tab);
	free(sidx->dat);
	free(sidx);
}

/* find the index of the given string */
int sidx_get(struct sidx *sidx, char *key)
{
	int i;
	for (i = 0; i < sidx->cnt; i++)
		if (!strcmp(sidx->tab[i], key))
			return i;
	return -1;
}

/* add the given string to sidx and return its index */
int sidx_put(struct sidx *sidx, char *key)
{
	if (sidx_get(sidx, key) >= 0)
		return sidx_get(sidx, key);
	if (sidx->cnt == sidx->len) {
		sidx->len = sidx->len ? sidx->len * 2 : 512;
		sidx->tab = mextend(sidx->tab, sidx->cnt, sidx->len, sizeof(sidx->tab[0]));
		sidx->dat = mextend(sidx->dat, sidx->cnt, sidx->len, sizeof(sidx->dat[0]));
	}
	sidx->tab[sidx->cnt] = malloc(strlen(key) + 1);
	strcpy(sidx->tab[sidx->cnt], key);
	return sidx->cnt++;;
}

/* return the string for the given index */
char *sidx_str(struct sidx *sidx, int id)
{
	if (id >= 0 && id < sidx->cnt)
		return sidx->tab[id];
	return NULL;
}

/* the number of items in sidx */
int sidx_len(struct sidx *sidx)
{
	return sidx->cnt;
}

void *sidx_datget(struct sidx *sidx, int id)
{
	if (id >= 0 && id < sidx->cnt)
		return sidx->dat[id];
	return NULL;
}

void sidx_datset(struct sidx *sidx, int id, void *dat)
{
	if (id >= 0 && id < sidx->cnt)
		sidx->dat[id] = dat;
}
