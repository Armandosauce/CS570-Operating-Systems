
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdbool.h>
#include <fcntl.h>
#include <errno.h>

#define  MAXITEM 100 /*max number of words per line */
#define  MAXPIPES 10

/* flags */
#define  STDIN_FD 0
#define  STDOUT_FD 1
#define  STDERR_FD 2

#define  ENVIRON 1
#define  CD      2

int bslash;

int OUT_RDIR;
int IN_RDIR;
int NO_WAIT;

int parse();
char* getCurrDir(char*, int);
void sighandler(int signum);
int hasBuiltIn();
bool dontWait();
void execPipes(int, int);
int userLookUp(char*, char*);
void hereis();
