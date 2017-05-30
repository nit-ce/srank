/* srank for ranking students */
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "srank.h"

#define NLEN	512		/* name length */
#define MLEN	512		/* major/minor name length */
#define RCNT	32		/* number of required majors */
#define PCNT	3		/* number of student preferences */
#define UDEF	100		/* default university score */

#define LEN(a)		(sizeof(a) / sizeof((a)[0]))

struct univ {
	char name[NLEN];	/* university name */
	int grade;		/* university grade (0-400) */
};
struct minor {
	char name[NLEN];	/* minor name */
	int mscmax;		/* maximum MSc admissions */
	int phdmax;		/* maximum PhD admissions */
	int msccnt;
	int phdcnt;
	int reqs[RCNT];		/* required majors */
	int reqs_cnt;		/* the number of items in reqs[] */
};
struct stud {
	char name[NLEN];	/* student name */
	int bsc;		/* BSc major name */
	int bscgpa;		/* BSc GPA (0-2000) */
	int bscuni;		/* BSc university */
	int prefs[PCNT];	/* student preferences */
	int prefs_cnt;		/* number of items in prefs[] */
	int score;
	int mapped;
};

/* string indices for storing univ, minor, and stud structs */
static struct sidx *univs;
static struct sidx *minors;
static struct sidx *studs;
/* BSc majors; just for comparison */
static struct sidx *bscs;
/* current line number */
static int lineno;

/* return the value in hundredths; e.g., 1430 for 14.3 */
static int hundredths(char *s)
{
	int n = 0;
	int f = 0;
	for (; isdigit((unsigned char) *s); s++)
		n = n * 10 + *s - '0';
	if (s[0] == '.' && isdigit((unsigned char) s[1])) {
		f = (s[1] - '0') * 10;
		if (isdigit((unsigned char) s[2]))
			f += s[2] - '0';
	}
	return n * 100 + f;
}

/* unify the given string; e.g., remove double spaces */
static char *sunify(char *s)
{
	struct sbuf *sb;
	int sp = 1;
	sb = sbuf_make();
	while (*s) {
		int c = (unsigned char) *s++;
		if (c < 128 && ispunct(c))
			c = ' ';
		if (c != ' ' || !sp)
			sbuf_chr(sb, c);
		sp = c == ' ';
	}
	/* stripping end-of-word spaces */
	if (sp && sbuf_len(sb) > 0)
		sbuf_cut(sb, sbuf_len(sb) - 1);
	return sbuf_done(sb);
}

/* like sidx_get(), but use sunify(key) */
static int sidx_uget(struct sidx *sidx, char *key)
{
	char *ukey = sunify(key);
	int idx = sidx_get(sidx, ukey);
	free(ukey);
	return idx;
}

/* like sidx_put(), but use sunify(key) */
static int sidx_uput(struct sidx *sidx, char *key)
{
	char *ukey = sunify(key);
	int idx = sidx_put(sidx, ukey);
	free(ukey);
	return idx;
}

static void stud_register(char *name)
{
	struct stud *st = malloc(sizeof(*st));
	int idx = sidx_uput(studs, name);
	memset(st, 0, sizeof(*st));
	strcpy(st->name, name);
	st->mapped = -1;
	st->score = -1;
	sidx_datset(studs, idx, st);
}

static struct stud *stud_find(char *name)
{
	int idx = sidx_uget(studs, name);
	if (idx >= 0)
		return sidx_datget(studs, idx);
	return NULL;
}

static void univ_register(char *name)
{
	struct univ *un = malloc(sizeof(*un));
	int idx = sidx_uput(univs, name);
	memset(un, 0, sizeof(*un));
	strcpy(un->name, name);
	sidx_datset(univs, idx, un);
}

static struct univ *univ_find(char *name)
{
	int idx = sidx_uget(univs, name);
	if (idx >= 0)
		return sidx_datget(univs, idx);
	return NULL;
}

static void minor_register(char *name)
{
	struct minor *mi = malloc(sizeof(*mi));
	int idx = sidx_uput(minors, name);
	memset(mi, 0, sizeof(*mi));
	strcpy(mi->name, name);
	sidx_datset(minors, idx, mi);
}

static struct minor *minor_find(char *name)
{
	int idx = sidx_uget(minors, name);
	if (idx >= 0)
		return sidx_datget(minors, idx);
	return NULL;
}

/* read a line and split it at tab characters */
static int onemoreline(FILE *fp, char **cols, int sz)
{
	int c = fgetc(fp);
	int ncols = 0;
	struct sbuf *sb;
	if (c == EOF)
		return -1;
	sb = sbuf_make();
	while (c != '\n' && c != EOF) {
		if (c != '\t') {
			sbuf_chr(sb, c);
		} else {
			if (ncols + 1 >= sz)
				break;
			cols[ncols++] = sbuf_done(sb);
			sb = sbuf_make();
		}
		c = fgetc(fp);
	}
	cols[ncols++] = sbuf_done(sb);
	lineno++;
	return ncols;
}

static void warn(char *fmt, ...)
{
	va_list ap;
	char msg[512];
	va_start(ap, fmt);
	vsprintf(msg, fmt, ap);
	va_end(ap);
	fprintf(stderr, "srank:%d: %s\n", lineno, msg);
}

static void srank_enrol(char *sname, char *mname)
{
	struct stud *st = stud_find(sname);
	if (st) {
		int midx = sidx_uget(minors, mname);
		if (midx >= 0) {
			struct minor *mi = sidx_datget(minors, midx);
			st->mapped = midx;
			mi->msccnt++;
		} else {
			warn("unknown minor <%s>", mname);
		}
	}
	if (!st)
		warn("unknown student <%s>", sname);
}

/* read input commands */
static void srank_input(FILE *fp)
{
	char *cols[64];
	int ncols;
	int i;
	while ((ncols = onemoreline(fp, cols, LEN(cols))) >= 0) {
		char *cmd = cols[0];
		if (!cmd || !cmd[0] || cmd[0] == '#') {
			continue;
		} else if (!strcmp("student", cmd) && cols[1]) {
			if (sidx_uget(studs, cols[1]) < 0)
				stud_register(cols[1]);
			else
				warn("student <%s> redefined", cols[1]);
		} else if (!strcmp("student_bsc", cmd) && cols[1]) {
			struct stud *s = stud_find(cols[1]);
			if (s && cols[2])
				s->bsc = sidx_uput(bscs, cols[2]);
			if (!s)
				warn("unknown student <%s>", cols[1]);
		} else if (!strcmp("student_bscgpa", cmd) && cols[1]) {
			struct stud *s = stud_find(cols[1]);
			if (s && cols[2])
				s->bscgpa = hundredths(cols[2]);
			if (!s)
				warn("unknown student <%s>", cols[1]);
		} else if (!strcmp("student_bscuni", cmd)) {
			struct stud *s = stud_find(cols[1]);
			if (s && cols[2])
				s->bscuni = sidx_uget(univs, cols[2]);
			if (!s)
				warn("unknown student <%s>", cols[1]);
		} else if (!strcmp("student_pref", cmd)) {
			struct stud *s = stud_find(cols[1]);
			if (s && s->prefs_cnt < PCNT && cols[2]) {
				int pref = sidx_uget(minors, cols[2]);
				if (pref >= 0)
					s->prefs[s->prefs_cnt++] = pref;
				else
					warn("unknown minor <%s>", cols[2]);
			}
			if (!s)
				warn("unknown student <%s>", cols[1]);
		} else if (!strcmp("university", cmd)) {
			if (sidx_uget(univs, cols[1]) < 0)
				univ_register(cols[1]);
			else
				warn("university <%s> redefined", cols[1]);
		} else if (!strcmp("university_grade", cmd)) {
			struct univ *u = univ_find(cols[1]);
			if (u && cols[2])
				u->grade = hundredths(cols[2]);
			if (!u)
				warn("unknown university <%s>", cols[1]);
		} else if (!strcmp("minor", cmd)) {
			if (sidx_uget(univs, cols[1]) < 0)
				minor_register(cols[1]);
			else
				warn("minor <%s> redefined", cols[1]);
		} else if (!strcmp("minor_msc", cmd)) {
			struct minor *m = minor_find(cols[1]);
			if (m && cols[2])
				m->mscmax = atoi(cols[2]);
			if (!m)
				warn("unknown minor <%s>", cols[1]);
		} else if (!strcmp("minor_phd", cmd)) {
			struct minor *m = minor_find(cols[1]);
			if (m && cols[2])
				m->phdmax = atoi(cols[2]);
			if (!m)
				warn("unknown minor <%s>", cols[1]);
		} else if (!strcmp("minor_req", cmd)) {
			struct minor *m = minor_find(cols[1]);
			if (m && m->reqs_cnt < RCNT && cols[2])
				m->reqs[m->reqs_cnt++] = sidx_uput(bscs, cols[2]);
			if (!m)
				warn("unknown minor <%s>", cols[1]);
		} else if (!strcmp("enrol", cmd)) {
			if (cols[1] && cols[2])
				srank_enrol(cols[1], cols[2]);
		} else {
			fprintf(stderr, "srank: unknown command <%s>\n", cmd);
		}
		for (i = 0; i < ncols; i++)
			free(cols[i]);
	}
}

/* compute student scores */
static void srank_scores(void)
{
	int i;
	for (i = 0; i < sidx_len(studs); i++) {
		struct stud *st = sidx_datget(studs, i);
		struct univ *un = sidx_datget(univs, st->bscuni);
		st->score = st->bscgpa + (un ? un->grade : UDEF);
	}
}

/* compare two students based on their score */
static int studcmp(void *v1, void *v2)
{
	struct stud *s1 = sidx_datget(studs, *(int *) v1);
	struct stud *s2 = sidx_datget(studs, *(int *) v2);
	return s2->score - s1->score;
}

/* rank students; the main algorithm */
static void srank_rank(void)
{
	int n = sidx_len(studs);
	int *sorted = malloc(n * sizeof(sorted[0]));
	int i, j, k;
	for (i = 0; i < n; i++)
		sorted[i] = i;
	qsort(sorted, n, sizeof(sorted[0]), (void *) studcmp);
	for (i = 0; i < n; i++) {
		struct stud *st = sidx_datget(studs, sorted[i]);
		if (st->mapped >= 0)
			continue;
		for (j = 0; j < st->prefs_cnt; j++) {
			struct minor *mi;
			if (st->prefs[j] < 0)		/* unknown minor */
				continue;
			mi = sidx_datget(minors, st->prefs[j]);
			if (!mi)			/* unknown minor */
				continue;
			for (k = 0; k < mi->reqs_cnt; k++)
				if (mi->reqs[k] == st->bsc)
					break;
			if (k == mi->reqs_cnt) {	/* unmet requirement */
				warn("unmet requirement %s:%s", st->name, mi->name);
				continue;
			}
			if (mi->msccnt >= mi->mscmax)	/* no room left */
				continue;
			mi->msccnt++;
			st->mapped = st->prefs[j];
			break;
		}
	}
}

/* print student admissions */
static void srank_print(FILE *fp)
{
	int i;
	for (i = 0; i < sidx_len(studs); i++) {
		struct stud *st = sidx_datget(studs, i);
		if (st->mapped >= 0) {
			struct minor *mi = sidx_datget(minors, st->mapped);
			fprintf(fp, "%s\t%s\n", st->name, mi->name);
		}
	}
}

/* print student grades */
static void srank_printgrades(FILE *fp)
{
	int i;
	for (i = 0; i < sidx_len(studs); i++) {
		struct stud *st = sidx_datget(studs, i);
		fprintf(fp, "%s\t%d.%02d\n", st->name, st->score / 100, st->score % 100);
	}
}

static void sidx_done(struct sidx *sidx)
{
	int i;
	for (i = 0; i < sidx_len(sidx); i++)
		free(sidx_datget(sidx, i));
	sidx_free(sidx);
}

int main(int argc, char *argv[])
{
	FILE *ifp = NULL;
	FILE *ofp = NULL;
	int grades = 0;
	int i;
	for (i = 1; i < argc && argv[i][0] == '-'; i++) {
		switch (argv[i][1]) {
		case 'i':
			ifp = fopen(argv[i][2] ? argv[i] + 2 : argv[++i], "r");
			break;
		case 'o':
			ofp = fopen(argv[i][2] ? argv[i] + 2 : argv[++i], "w");
			break;
		case 'g':
			grades = 1;
			break;
		default:
			printf("Usage: srank options <input >output\n");
			printf("Options:\n");
			printf("  -i path \t read from a file instead of standard input\n");
			printf("  -o path \t write to a file instead of standard output\n");
			printf("  -g      \t only print student grades\n");
			return 1;
		}
	}
	univs = sidx_make();
	minors = sidx_make();
	studs = sidx_make();
	bscs = sidx_make();
	srank_input(ifp ? ifp : stdin);
	srank_scores();
	srank_rank();
	if (grades)
		srank_printgrades(ofp ? ofp : stdout);
	else
		srank_print(ofp ? ofp : stdout);
	if (ifp)
		fclose(ifp);
	if (ofp)
		fclose(ofp);
	sidx_done(univs);
	sidx_done(minors);
	sidx_done(studs);
	sidx_free(bscs);
	return 0;
}
