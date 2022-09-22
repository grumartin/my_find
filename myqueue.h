#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/wait.h>

#define KEY 3454354345 /* unique key e.g., matriculation number */
#define PERM 0660 /* starts with 0 to declare it as octal */

#define MAX_DATA 255

typedef struct
{
	long mType;
	char filename[MAX_DATA];
} message_t;
