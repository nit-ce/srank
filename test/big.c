#include <stdio.h>

#define NUNIV		100
#define NMINOR		34
#define NSTUD		1000
#define UNAME		"The Test University %d"
#define MNAME		"The Test Minor %d"
#define SNAME		"The Test Student %d"

static void univs(void)
{
	int i;
	char name[128];
	for (i = 0; i < NUNIV; i++) {
		sprintf(name, UNAME, i);
		printf("university\t%s\n", name);
		printf("university_grade\t%s\t%d.%02d\n", name, i % 4, i % 100);
	}
}

static void minors(void)
{
	int i;
	char name[128];
	char req[128];
	for (i = 0; i < NMINOR; i++) {
		sprintf(name, MNAME, i);
		printf("minor\t%s\n", name);
		printf("minor_msc\t%s\t%d\n", name, 5 + i % 7);
		sprintf(req, MNAME, (i + 0) % NMINOR);
		printf("minor_req\t%s\t%s\n", name, req);
		sprintf(req, MNAME, (i + NMINOR - 1) % NMINOR);
		printf("minor_req\t%s\t%s\n", name, req);
		sprintf(req, MNAME, (i + NMINOR - 2) % NMINOR);
		printf("minor_req\t%s\t%s\n", name, req);
	}
}

static void students(void)
{
	char name[128];
	char pref[128];
	char uname[128];
	char mname[128];
	int i;
	for (i = 0; i < NSTUD; i++) {
		sprintf(name, SNAME, i);
		sprintf(uname, UNAME, i % NUNIV);
		sprintf(mname, MNAME, i % NMINOR);
		printf("student\t%s\n", name);
		printf("student_name\t%s\t%s\t%s\n", name, name, name);
		printf("student_bsc\t%s\t%s\n", name, mname);
		printf("student_bscuni\t%s\t%s\n", name, uname);
		printf("student_bscgpa\t%s\t%d.%02d\n", name, 16 + i % 4, i % 100);
		sprintf(pref, MNAME, (i + 0) % NMINOR);
		printf("student_pref\t%s\t%s\n", name, pref);
		sprintf(pref, MNAME, (i + 1) % NMINOR);
		printf("student_pref\t%s\t%s\n", name, pref);
		sprintf(pref, MNAME, (i + 2) % NMINOR);
		printf("student_pref\t%s\t%s\n", name, pref);
	}
}

int main(void)
{
	univs();
	minors();
	students();
	return 0;
}
