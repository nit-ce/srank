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
	int ncols = 0;
	int c = fgetc(fp);
	int quote = 0;
	struct sbuf *sb;
	memset(cols, 0, sz * sizeof(cols[0]));
	if (c == EOF)
		return -1;
	sb = sbuf_make();
	while (c != EOF) {
		if (!quote && c == '\n')
			break;
		if ((quote && c == '"') || (!quote && c == '\t')) {
			quote = 0;
			if (ncols + 1 >= sz)
				break;
			cols[ncols++] = sbuf_done(sb);
			sb = sbuf_make();
			if (c == '"')
				c = fgetc(fp);
			if (c == '\n')
				ungetc(c, fp);
		} else {
			if (c == '\n')
				sbuf_chr(sb, ',');
			else if (c == '\t')
				sbuf_chr(sb, ' ');
			else if (c == '"')
				quote = 1;
			else if (c != '\r')
				sbuf_chr(sb, c);
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

static int hdrs_find(char **hdrs, int hdrs_n, char *hdr, int def)
{
	int i;
	for (i = 0; i < hdrs_n; i++)
		if (!strcmp(hdr, hdrs[i]))
			return i;
	fprintf(stderr, "bad header <%s>!\n", hdr);
	exit(1);
}

/* convert student data */
static void conv_studs(FILE *fp, int group)
{
	char *cols[128];
	char stud[128] = "";
	char *hdrs[128];
	int n;
	int hdrs_n = onemoreline(fp, hdrs, LEN(hdrs));		/* the headers */
	int h_id = hdrs_find(hdrs, hdrs_n, "شماره پرونده داوطلب", 1);
	int h_name1 = hdrs_find(hdrs, hdrs_n, "نام", 7);
	int h_name2 = hdrs_find(hdrs, hdrs_n, "نام خانوادگی", 6);
	int h_bsc = hdrs_find(hdrs, hdrs_n, "رشته تحصیلی کارشناسی", 21);
	int h_uni = hdrs_find(hdrs, hdrs_n, "دانشگاه محل اخذ مدرک کارشناسی", 20);
	int h_gpa = hdrs_find(hdrs, hdrs_n, "معدل تا پایان نیمسال ششم", 27);
	int h_pref = hdrs_find(hdrs, hdrs_n, "گرایش (های) انتخابی", 5);
	while ((n = onemoreline(fp, cols, LEN(cols))) >= 0) {
		if (cols[0] && isdigit((unsigned char) cols[0][0])) {
			snprintf(stud, sizeof(stud), "%s", cols[h_id]);
			printf("student\t%s\n", stud);
			printf("student_name\t%s\t%s\t%s\n", stud, cols[h_name1], cols[h_name2]);
			printf("student_bsc\t%s\t%s\n", stud, cols[h_bsc]);
			printf("student_bscuni\t%s\t%s\n", stud, cols[h_uni]);
			printf("student_bscgpa\t%s\t%s\n", stud, cols[h_gpa]);
			printf("student_pref\t%s\t%s\n", stud, cols[h_pref]);
			printf("student_grp\t%s\t%d\n", stud, group);
		}
		if (stud[0] && cols[0] && cols[h_pref] && !cols[0][0] && cols[h_pref][0])
			printf("student_pref\t%s\t%s\n", stud, cols[h_pref]);
	}
}

int main(int argc, char *argv[])
{
	FILE *fp;
	int i;
	if (argc < 3) {
		fprintf(stderr, "Usage: sconv universities minors students [students2]\n");
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
	for (i = 0; 3 + i < argc; i++) {
		fp = fopen(argv[3 + i], "r");
		if (!fp) {
			fprintf(stderr, "sconv: failed to open <%s>\n", argv[3]);
			return 1;
		}
		conv_studs(fp, i);
		fclose(fp);
	}
	return 0;
}
