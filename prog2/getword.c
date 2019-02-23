#include "getword.h"
#include <stdlib.h>

/*
    Armando Perez
    CS570, John Carroll
    This program is due on Sept. 17th at 11 PM.
*/


int getword(char *w) {

  int iochar = 0;       // A buffer that hold each character returned from successive calls to getchar()
  
  int size = 0;         // The number of characters written into where w points in memory
  
  int shellcmd = 0;     // A boolan value set to 1, if a '$' is read as the first character of a token
  
  int temp = 0;         /* temporary character used to hold the return value from a
   						   second call to getchar(), this is declared outside of any if blocks to avoid 						   failure upon running splint command from terminal
                        */
  int spaces = 0;
  
  extern int bslash;
  bslash = 0;
  
  while( ( iochar = getchar() ) != EOF) {
    
    /* This is a check to see if thestring buffer in main.c gets overloaded.
       If so(more than 254 characters read from stdin), then ungetc(iochar, stdin)
       is called so that the next call to getword() will begin at the very next  
       character on the input stream.
    */
    if(iochar == '$' && size == 0 && !shellcmd) {
      shellcmd = 1;
      continue;  
    }
    
    if(size == STORAGE-1) {
      *w = '\0';
      w++;
      ungetc(iochar,stdin);
      
      if(shellcmd == 1) {
        return -size;
      }
      else {
        return size;
      }
    }

    else if(iochar == 0x5C) {
    
      //Testing three outcomes when a backslash character is read into iochar
      bslash = 0;
      temp = getchar();
      
      /* if the character following a backslash(on the input stream) is a backslash as well,
         write one '\' character to where w points
      */

      if (temp == 0x5C) {
        *w = temp;
        bslash++;
        size++;
        w++;
      }
      
      
      /*  if the character following a backslash is a newline, ignore that character
          by reiterating while loop, otherwise treat ANY character following a backslash is 
          where w points, as a non-meta character and write it into memory.
      */
      
      else {
        if(temp != '\n') { 
          bslash++;
          *w = temp;
          w++;
          size++;
        }
      }
     
    }
    
    
    /* Spaces, newline, and semicolons are treated as delimiters in this program.  
       To achieve the desired behavior, the preceding character written into w are null terminated. 
       Then, the next call to getword will have size = 0, and return an empty string 
       to main.c.
    */
    
    else if (iochar == ' '){
      if(bslash > 0 && size == spaces + 1) {
        spaces++;
        *w++ = iochar;
        size++;
      }
      else if(size > 0) {
    
    	  *w = '\0';
        if(shellcmd == 1) {
          return -size;
        }
        else {
          return size;
        }
      }
      
    }

    else if ((iochar == '\n' || iochar == ';' )) {
      
      *w = '\0';
      if (size == 0 && !shellcmd) {
        return size;
      }
      ungetc(iochar, stdin);      
      if(shellcmd == 1) {
        return -size;
      }
      else {
        return size;
      }
    }
    
    /* It required that the following routine 'look ahead' of the input stream to determine the
       metacharacter when a '<' is read into iochar, as it could very well be a '<<'. To do so, the
       routine uses a second call to getchar() stored into a temporary character buffer(temp). 
       If temp IS the character '<', then there are two '<' characters in succession and are treated
       as the metacharacter '<<'. If temp is NOT '<', then the character read into temp must be
       put back on the input stream, which is done using the call ungetc(temp, iochar). The previous
       logic is ignored if iochar is any other meta character included in the OR'ed term of the
       boolean else-if statement ('|','>','&'). Rather, the preceding string is null-terminated
       and the call to ungetc(iochar,stdin) is used to re-read the metacharacter into iochar on the
       next call to getword(), THIS time with size equal to zero, so that the meta character is read
       independently.
    */

    else if((iochar == '>' || iochar == '|' ||
            iochar == '<'  || iochar == '&') && !shellcmd) 
    {
      
      *w = '\0';
      if (size == 0) {
        temp = getchar();
        *w = iochar;
        w++;
        size++;
        if(temp == '<') {   
          *w = temp;
          w++;
          size++;
          *w = '\0';
          w++;
          return size;  
        }
        *w = '\0';
        w++;
        ungetc(temp,stdin);
        return size;
      }
      
      w++;
      ungetc(iochar, stdin);
      return size;
    }
      
    /* If the tilde character ('~') is encountered, then first the program
       must verify it's the first character of a token. This routine takes advantage of
       the fact that pointers are treated as arrays in memory. So the pointer 'home',
       points to the first character of the return value from the call to getenv("HOME").
       Every subsequent character is read into a character buffer(buff) then written into where w 
       currently points, until a null character is read into the buffer.
    */

    else if(iochar == '~' && !shellcmd && size == 0) {
      
      temp = getchar();
      ungetc(temp, stdin);
      
      if(temp == ';' || temp == ' ' || temp == '\n') {
		    
		    char *home = getenv("HOME");
		    int i = 0;
		    char buff = home[i];
		    
		    while(buff != '\0') {
		      *w = buff;
		      i++;
		      w++;
		      buff = home[i];
		      size++;
		    }
		  }
		  else {
		    *w = iochar;
		    w++;
		    size++;
		  }
		}

    /* Any other non meta characters read into iochar are written to where w points into memory
    */

    else {
      *w = iochar;
      w++;
      size++;
    }
  }   //while
  
  /* If EOF is encountered in the middle of an input stream, then the characters written into
     w must be null terminated, so that the last call to getword() does not produce gibberish output.
  */
  if(iochar == EOF) {*w = '\0';}
  if (size == 0 && !shellcmd) {
    return -255;
  }
     
  return size;
}
