#include <stdio.h>

void vuln(){
   char buffer[32];
   gets(buffer);
}

int main(){
   puts("Inicio");
   vuln();
   return 0;
}
