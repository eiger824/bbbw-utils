#ifndef STRUTILS_H_
#define STRUTILS_H_

#include <string.h>
#include <stdlib.h>

char* msg_app_int(const char* msg1, int n)
{
	char* p = malloc(200);
	int b = sprintf(p, msg1, n);
	return p;
}

char* msg_err(const char* msg, int errnum)
{
	char *p = malloc(200);
	strcpy(p, msg);
	strcat(p, ": ");
	strcat(p, strerror(errnum));
	return p;
}

char *msg_app_int_int(const char* msg1, int n1, int n2)
{
	char *p = malloc(200);
	int b = sprintf(p, msg1, n1, n2);
	return p;
}

char* msg_app_flt(const char* msg1, float n)
{
	char* p = malloc(200);
	int b = sprintf(p, msg1, n);
	return p;
}

char* msg_app_flt_flt(const char* msg1, float n1, float n2)
{
	char* p = malloc(200);
	int b = sprintf(p, msg1, n1, n2);
	return p;
}

char* msg_app_str(const char* msg1, const char* msg2)
{
	char *p = malloc(200);
	int b = sprintf(p, msg1, msg2);
	return p;
}

#endif /*STRUTILS_H_*/
