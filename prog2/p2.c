
#include "getword.h"
#include "p2.h"

/*
    Armando Perez
    CS570, John Carroll
    This program is due in its entirety on Dec. 5th at 11 PM.
*/

char args[MAXITEM * STORAGE];
char *newargv[STORAGE];

char currDir[STORAGE];
char startDir[STORAGE];
char *workingDir;

char *environ = NULL;
char value[STORAGE];

int offset[MAXPIPES];
int num_pipes;
int pipefd[20];

char* passwdfile = "/etc/passwd";
          
int grand_pid;
int builtin;
int numWords = 0;
int cmd_ptr;
int out_ptr;
int in_ptr;
int chars = 0;
int status = 0;
int c_wpid = 0;
int wpid;
int NEW_FILE = O_WRONLY | O_CREAT | O_EXCL;
int OPEN_FILE = O_RDONLY;
int cpid = 0;

struct stat passwd;

/* flags */
int KILLSIG;

int main()

{
  
  (void)signal(SIGTERM, sighandler);
  
  setpgid(0, 0);
  
  getcwd(startDir, STORAGE);
  getcwd(currDir, STORAGE);
  
  stat(passwdfile, &passwd);

  while (!KILLSIG) {
	
		if(strcmp(startDir, currDir) != 0) {
		  workingDir = getCurrDir(currDir, STORAGE);
		  printf("%s", workingDir);
		}
		
		printf(":570: ");
			
		numWords = parse();
	  
		if(numWords == 0 && chars == -255) { break; }  	
		if(numWords <= 0)   { continue; }
		
		if((builtin = hasBuiltIn()) > 0) {
			/* handle "cd" code */
			if(	builtin == CD) {
				if(numWords == 1) {
				  if(chdir(getenv("HOME")) < 0) {
					  perror("error");
					}
				}
			  else if(numWords == 2 || (numWords == 3 && NO_WAIT)) { 
		      if (chdir(newargv[1]) < 0) {
		        perror("error");
		      }
			  }
			  else {
			    fprintf(stderr, "cd error");
			  }
			  memset(currDir, 0, STORAGE);
			  getcwd(currDir, STORAGE);
				continue;
			}
			
			/* handle "environ" code */
			else if( builtin == ENVIRON ) {
				  
				if(numWords == 2) {
				  environ = getenv(newargv[1]);
			    printf("%s", environ);
			    printf("\n");

				}
				else if (numWords == 3) {
				  environ = newargv[1];
			    strcpy(value, newargv[2]);
			    
			    strcat(environ, "=");
			    strcat(environ, value);
			    
			    if(putenv(environ) != 0) {
			      perror("environ error");
			    }
				}
				else {
				  fprintf(stderr, "environ error");
				}
				
				continue;
			}
		}
		
    int i;
    for(i = 0; i <= numWords; i++) {
      printf("newargv[%d]: %s\n", i, newargv[i]);
    
    }
    
    fflush(stdout);
	  fflush(stderr);
		
    if((cpid = fork()) == 0) {
    
      if (num_pipes > 0) {
				 
        int j;
        for(j = 0; j < num_pipes; j++) {
          if(pipe(pipefd + j*2) < 0) {	//0 && 1
						perror("pipe error");
						exit(errno);
					}
        }
        
        for(j = 0; j < num_pipes; j++) {
		      execPipes(2* (num_pipes - j - 1), offset[num_pipes-j-1]);

		      if(grand_pid > 0) {
		      	break;
		      } 
		    }
		    
	      for(j = 0; j < 2 * num_pipes; j++) {
	        if(close(pipefd[j]) < 0) {
	          perror("close error");
	          fprintf(stderr, "failed to close pipefd[%d]\n", j);
	          exit(errno);
	        }
	      }
	      
      } //num_pipes > 0
    
      int outfd;
      int infd;
      
      //if
      if(OUT_RDIR == 1) {
		    if(num_pipes == 0 || cmd_ptr == offset[num_pipes-1]) { 
				  if((outfd = open(newargv[out_ptr], NEW_FILE, S_IRUSR | S_IWUSR)) < 0) {
		        perror("open error");
		        exit(errno);
		      }
		      if(dup2(outfd, STDOUT_FD) == -1) {
		        perror("dup2 error");
		        exit(errno);
		      }
		      if(close(outfd) < 0) {
		        perror("close error");
		        exit(errno);
		      }
		    }
      }
      
      if(IN_RDIR == 1) {
        if(num_pipes == 0 || cmd_ptr < offset[0]) { 
          if((infd = open(newargv[in_ptr], OPEN_FILE)) < 0) {
            perror("open error");
            exit(errno);
          }
          if(dup2(infd, STDIN_FD) == -1) {
            perror("dup2 error");
            exit(errno);
          }
          if(close(infd) < 0) {
            perror("close error");
            exit(errno);
          }
        }
      }
      
      if(NO_WAIT && IN_RDIR == 0) {
		    
		    int bgfd;
        
        if((bgfd = open("/dev/null", OPEN_FILE)) < 0) {
          perror("open error");
          exit(errno);
        }
        
        if(dup2(bgfd, STDIN_FD) < 0) {
        	perror("dup2 error");
        	exit(errno);
        }
        
        if(close(bgfd) < 0) {
          perror("close error");
          exit(errno);
        }
        
		  }
		  
      if(execvp(newargv[cmd_ptr], &newargv[cmd_ptr]) < 0) {
				perror("execvp error");
				exit(errno);
		  }
    	
    }  
    else if(NO_WAIT) {
      
      printf("%s [%d]\n",newargv[cmd_ptr], cpid);
    
    }
    else {
    
      for(;;) {
        
        c_wpid = wait(&status);
        
        if(cpid == c_wpid) {
          break;
        }
      }
    }
    
  }

  killpg(getpgrp(), SIGTERM);

  printf("p2 terminated.\n");
  exit(0);
}


int parse() {
    int argc = 0;
    int ptr = 0;
    int wordnum = 0;
    int i, j;


    char *curr;
    char *env;
    char buff[STORAGE];
    
    IN_RDIR = 0;
    OUT_RDIR = 0;
    NO_WAIT = 0;
    bslash = 0;
    
    cmd_ptr = -1;
    out_ptr = -1;
    in_ptr = -1;
    grand_pid = -1;
    
    num_pipes = 0;
    chars = 0;
    
    memset(args, 0, STORAGE * MAXITEM);
    
    while ( (chars = getword(args + ptr)) != 0 && chars != -255) {
      
      curr = (args + ptr);
      
      if (bslash == 0) {
		    
		    // '$' code
		    if(chars < 0) {
		      env = getenv(curr);
		      
		      if(!env) {
		        fprintf(stderr, "%s: Undefined variable\n", curr);
		        return -2;
		      }
		      curr = env;
		      chars *= -1;
		    }
		    
		    
		    if(strcmp(curr,"|") == 0) {
		      if(num_pipes >= MAXPIPES) {
		        fprintf(stderr, "parse error: only %d | allowed per input line\n", MAXPIPES);
		        return -2;
		      }
		      
		      offset[num_pipes] = argc+1;
		      num_pipes++;
		      newargv[argc++] = NULL;
		    
		    }
		    else if(strcmp(curr,"<") == 0) {
		      if (IN_RDIR == 0) { 
		        IN_RDIR = 1;
				    newargv[argc++] = NULL;
				    in_ptr = argc;
				  }
		      else {
		        fprintf(stderr, "parse error: only one < allowed per input line\n");
		        return -2;
		      }
		    }
		    else if(strcmp(curr,">") == 0) {
		      if(OUT_RDIR == 0) {
		        OUT_RDIR = 1;
			      newargv[argc++] = NULL;
			      out_ptr = argc;
			    }
		      else {
		        fprintf(stderr, "parse error: only one > allowed per input line\n");
		        return -2;
		      }
		    }
		    else if(strcmp("<<", curr) == 0) {
		      
		    }
		    else {
		      newargv[argc] = curr;
		      if(out_ptr != argc && in_ptr != argc && cmd_ptr < 0) {
				      cmd_ptr = argc;
			    }
			    argc++;		      		    
		    }
      }
      else {
      	
      	newargv[argc] = curr;
      	
	      if(out_ptr != argc && in_ptr != argc && cmd_ptr < 0) {
			      cmd_ptr = argc;
		    }
		    argc++;
      }
      
      args[ptr + chars] = '\0';
		  ptr += chars + 1;
      
      wordnum++;
      /*      
      int i;
      for (i = 0; i < chars; i++) {
			printf("curr[%d]: %c\n", i, curr[i]);
	  }
	  */
		  
    }
    
    /*
    Check the array contents, make sure it not entirely null, if it is return 0 and reissue the prompt
    */
    if(chars != -255 && wordnum > 0) {

			for(i = 0; i < wordnum; i++) {
				if(newargv[i]) {
				  break;
				}
				if(i == wordnum - 1) {
				  fprintf(stderr, "parse error: no arguments detected\n");
				  return 0;
				}
			}
		  
		  if(newargv[wordnum - 1]) {
				if(strcmp("&", newargv[wordnum-1]) == 0) {
					newargv[wordnum - 1] = NULL;
					NO_WAIT = 1;
				}
			}
			
			for(i = 0; i < wordnum; i++) {
			  if(newargv[i]) {
					if(newargv[i][0] == '~') {
					  for(j = 0; j < STORAGE; j++) {
					    if(newargv[i][j] == '/') {
					      	break;
					      }
					  }
					  if(!newargv[i][j]) {
					  	
					  	if(userLookUp(&newargv[i][1], &newargv[i][0]) < 0) {
					  	  fprintf(stderr, "Unidentified user: %s\n", &newargv[i][1]);
					  	}
					  }
					  else {
					  
					    newargv[i][j++] = '\0';
					    buff[0] = '/';
					    strcpy(&buff[1], &newargv[i][j]); 
					    
					  	if(userLookUp(&newargv[i][1], &newargv[i][0]) == 0) {
					  		strcat(newargv[i], buff);
					  	}
					  	else {
					  	  fprintf(stderr, "Unidentified user: %s\n", &newargv[i][1]);
					  	}
					  	
					  }
					} 
				}
			}
			
		}
		
    newargv[wordnum] = NULL;
    
    
    return wordnum;
}


void execPipes(int pindex, int pipe_ptr) {
  /*
  
  pipefd[pindex] = the read end of the current pipe
  pipefd[pindex + 1] = the write end of the current pipe
  
  */
  
  fflush(stdout);
  fflush(stderr);
  
  if((grand_pid = fork()) == 0) {
    //grandchild
    
    if(dup2(pipefd[pindex + 1], STDOUT_FD) < 0) {
      perror("dup2 error");
      exit(errno);
    }
  } //grandchild
  
  else if (grand_pid > 0) {
    //child
    
    if(dup2(pipefd[pindex], STDIN_FD) < 0) {
		  perror("dup2 error");
		  exit(errno);
		}

		cmd_ptr = pipe_ptr;  
     
  } //child
  
  else {
    perror("fork error");
    exit(errno);  
  } 
    
}

void sighandler(int signum) {
  KILLSIG = 1;
}

int hasBuiltIn() {
  
  if(newargv[0]) {
		if(strcmp(newargv[0], "cd") == 0) {
		  return CD;
		}
		else if(strcmp(newargv[0], "environ") == 0) {
		  return ENVIRON;
		}
  }
  return 0;
}

char* getCurrDir(char *dir, int storage) {
  int i;
  for(i = storage; i >= 0; i--) {
    if(dir[i] == '/') {
      break;
    }
  }
  
  return &dir[i+1];
}

void hereis() {

}

int userLookUp(char* name, char* dest) {

  char* curr;
  char* path;
  FILE* fp;
  char buff[passwd.st_blocks * 512];
  int i;
  int semicount = 0;
  int pathsize = 0;

  fp = fopen(passwdfile, "r");
  
  if(!fp) {
    perror("fopen error");
    exit(errno);
  }
  
  (void)fread(buff, 512, passwd.st_blocks, fp);
  
  curr = strstr(buff, name); 
  if(curr == NULL) { return -1; }
  
  for(;;) {
    
    if(*curr == ':') {
      semicount++;
    }
    
    if(semicount == 6) {
    	break;
    }
    
    if(semicount == 5) {
      pathsize++;
      curr++;
      continue;      
    }
    
    curr++;
    path = curr;
  }
  
  pathsize--;
  path++;
  
  printf("path: %s, pathsize: %d\n", path, pathsize);

  memmove(dest, path, pathsize); 
  dest[pathsize] = '\0';
  
  if(fclose(fp) == EOF) {
    perror("fclose error");
    exit(errno);
  
  }
  
  return 0;
}






