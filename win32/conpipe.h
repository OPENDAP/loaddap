#ifndef WIN32_CONSOLE_PIPE
#define WIN32_CONSOLE_PIPE 1

#include <stdio.h>

int		pclose(FILE *);
FILE	*openpipe(char *, unsigned short);

#endif
