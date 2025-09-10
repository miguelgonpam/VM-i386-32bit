#include <stdio.h>
#include <stdint.h>


void sort(uint32_t *arr, size_t s) {
    for (int i = 0; i < s - 1; i++) {
        int min_idx = i;
        for (int j = i + 1; j < s; j++) {
            if (arr[j] < arr[min_idx]) {
                min_idx = j;
            }
        }
        // Intercambiar
        uint32_t temp = arr[i];
        arr[i] = arr[min_idx];
        arr[min_idx] = temp;
    }
}

void print_arr(uint32_t *arr, size_t s){
   for (int i=0; i<s; i++){
	printf("%08x\n", arr[i]);
   }

}

void check(uint32_t *arr, size_t s){
   char f = 0;
   for(int i=1; i<s; i++){
      if (arr[i-1] > arr[i]){
         printf("NO: Error in index %u-%u \n", i-1, i);
	 f = 1;
      }

   }
   if (!f){
	printf("CORRECT \n");
   }

}

int main(){
   uint32_t arr[] = { 0x0804847e,0x0804847f,0x08048184,0x080481b0,0x080481e8,0x08048224,0x0804828f,0x08048290,0x080483ff,0x08048421, 0x08048627, 0x080488e9, 0x08048c64, 0x08048291,0x08048c3d, 0x0804892b, 0x08048dd5, 0x08048836, 0x08048cf4, 0x0804897a, 0x08048b1e, 0x08048dbf, 0x08048ea7, 0x080484a3, 0x08048879, 0x08048b94, 0x080485c1, 0x0804891b, 0x080487ad, 0x08048ef0, 0x08048bec, 0x08048785, 0x080489bb, 0x08048627, 0x0804847e, 0x080489c0, 0x0804815d, 0x080483ff, 0x0804855e, 0x08048290, 0x080489ee, 0x08048261, 0x08048cb7, 0x08048de6, 0x080488e9, 0x0804847f, 0x08048a97, 0x08048a97, 0x080488ee, 0x08048ef0, 0x08048124, 0x08048cb7, 0x08048447, 0x08048b58, 0x080489c0};
   sort(arr, 55);
   print_arr(arr, 55);
   check(arr, 55);

   return 0;
}

