#include <stdio.h>
#include <signal.h>

int main(){
   printf("Sigset : %d\n", sizeof(sigset_t));
   printf("Struct : %d\n", sizeof(struct sigaction));
   return 0;


}
