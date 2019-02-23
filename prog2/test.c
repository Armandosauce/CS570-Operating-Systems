#include <stdio.h>

int main() {

  char buff[256];
  
  fgets(buff, 256, stdin);
  
  printf("\n%s\n\n", buff);
  
  return 0;
}
