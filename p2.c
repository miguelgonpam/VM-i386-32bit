#include <stdio.h>

int main(){
   char buffer[32];
   fgets(buffer, 32, stdin);
   puts(buffer);
   return 0;
}
