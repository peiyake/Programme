#include <stdio.h>
int main(int argc, const char *argv[])
{
	long a[] = {1,2,3,4,5,6,7,8,9};
	long *p = a;
	fprintf(stdout,"sizeof a  =%d\n",sizeof(a));
	fprintf(stdout,"sizeof p  =%d\n",sizeof(p));
	fprintf(stdout,"sizeof *p =%d\n",sizeof(*p));
	return 0;
}
