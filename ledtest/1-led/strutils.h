#ifndef STRUTILS_H_
#define STRUTILS_H_

char* msg_app_int(const char* msg1, int n);
char* msg_err(const char* msg, int errnum);
char *msg_app_int_int(const char* msg1, int n1, int n2);
char* msg_app_flt(const char* msg1, float n);
char* msg_app_flt_flt(const char* msg1, float n1, float n2);
char* msg_app_flt3(const char* msg1, float n1, float n2, float n3);
char* msg_app_str(const char* msg1, const char* msg2);

#endif /*STRUTILS_H_*/
