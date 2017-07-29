/* generate srank input */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "srank.h"

#define LEN(a)		(sizeof(a) / sizeof((a)[0]))

/* read a line and split it at tab characters */
static int onemoreline(FILE *fp, char **cols, int sz)
{
	int c = fgetc(fp);
	int ncols = 0;
	struct sbuf *sb;
	memset(cols, 0, sz * sizeof(cols[0]));
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
	return ncols;
}

/* convert university data */
static void conv_univs(FILE *fp)
{
	char *cols[32];
	int n;
	while ((n = onemoreline(fp, cols, LEN(cols))) >= 0) {
		if (cols[0] && cols[1] && cols[2] && isdigit((unsigned char) cols[0][0])) {
			printf("university\t%s\n", cols[1]);
			printf("university_grade\t%s\t%s\n", cols[1], cols[2]);
		}
	}
}

static void minor_reqs(char *min, char *req)
{
	struct sbuf *sb = sbuf_make();
	while (*req) {
		if (*req == ',') {
			printf("minor_req\t%s\t\"%s\"\n", min, sbuf_buf(sb));
			sbuf_cut(sb, 0);
		}
		if (*req != ',' && *req != '"')
			sbuf_chr(sb, (unsigned char) *req);
		req++;
	}
	printf("minor_req\t%s\t\"%s\"\n", min, sbuf_buf(sb));
	sbuf_free(sb);
}

/* convert minor data; requirements are split with a comma sign */
static void conv_minors(FILE *fp)
{
	char *cols[32];
	int n;
	while ((n = onemoreline(fp, cols, LEN(cols))) >= 0) {
		if (cols[0] && cols[1] && cols[2] && isdigit((unsigned char) cols[2][0])) {
			printf("minor\t%s\n", cols[0]);
			printf("minor_msc\t%s\t%s\n", cols[0], cols[2]);
			minor_reqs(cols[0], cols[1]);
		}
	}
}

/* convert student data */
static void conv_studs(FILE *fp)
{
	char *cols[128];
	char stud[128] = "";
	int n;
	while ((n = onemoreline(fp, cols, LEN(cols))) >= 0) {
		if (cols[0] && isdigit((unsigned char) cols[0][0])) {
			snprintf(stud, sizeof(stud), "%s", cols[1]);
			printf("student\t%s\n", stud);
			printf("student_name\t%s\t%s\t%s\n", stud, cols[7], cols[6]);
			printf("student_bsc\t%s\t%s\n", stud, cols[21]);
			printf("student_bscuni\t%s\t%s\n", stud, cols[20]);
			printf("student_bscgpa\t%s\t%s\n", stud, cols[27]);
			printf("student_pref\t%s\t%s\n", stud, cols[5]);
		}
		if (stud[0] && cols[0] && cols[5] && !cols[0][0] && cols[5][0])
			printf("student_pref\t%s\t%s\n", stud, cols[5]);
	}
}

int main(int argc, char *argv[])
{
	FILE *fp;
	if (argc < 3) {
		fprintf(stderr, "Usage: sconv universities minors students\n");
		return 1;
	}
	fp = fopen(argv[1], "r");
	if (!fp) {
		fprintf(stderr, "sconv: failed to open <%s>\n", argv[1]);
		return 1;
	}
	conv_univs(fp);
	fclose(fp);
	fp = fopen(argv[2], "r");
	if (!fp) {
		fprintf(stderr, "sconv: failed to open <%s>\n", argv[2]);
		return 1;
	}
	conv_minors(fp);
	fclose(fp);
	fp = fopen(argv[3], "r");
	if (!fp) {
		fprintf(stderr, "sconv: failed to open <%s>\n", argv[3]);
		return 1;
	}
	conv_studs(fp);
	fclose(fp);
	return 0;
}
