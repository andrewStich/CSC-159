// k-lib.c, 159

#include "k-include.h"
#include "k-type.h"
#include "k-data.h"

// clear DRAM data block, zero-fill it +++++++++++
void Bzero(char *p, int bytes) {
	int i;
	for(i = 0; i < bytes; i++){
		*(p + i) = 0;
	}
}

int QisEmpty(q_t *p) { // return 1 if empty, else 0 +++++++
   	if(p->tail == 0){
		return 1;
	}else{ 
		return 0;
	}
}

int QisFull(q_t *p) { // return 1 if full, else 0 +++++++
	if(p->tail == Q_SIZE){
		return 1;
	}else{
		return 0;
	}
}

// dequeue, 1st # in queue; if queue empty, return -1 ++++++++
// move rest to front by a notch, set empty spaces -1 ++++++++
int DeQ(q_t *p) { // return -1 if q[] is empty	
	int i, value;

	if(QisEmpty(p)) {
		return -1;
	}else{
		value = p->q[0]; 			// save head of queue
		p->tail--;			        // decrement tail
		for(i = 0; i < Q_SIZE-1; i++){		// loop through entire queue
			if( i < p->tail )		// check if i hasnt reached tail
        			p->q[i]=p->q[i+1]; 	// if true, q[i] gets previous value in queue
			else				// else tail has been reached
				p->q[i] = -1;		// replace q[i] with -1 (empty value
    		}
		return value;				// return saved queue head
	}
}

// if not full, enqueue # to tail slot in queue +++++++++
void EnQ(int to_add, q_t *p) {
	if(QisFull(p)) {
   		cons_printf("Panic: queue is full, cannot EnQ!\n");
		return;
	}else{
		p->q[p->tail] = to_add;
		(p->tail)++;       		 //tail++
		return; 		
	}
}

void MemCpy(char *dst, char *src, int size) {
   int i;
   for(i = 0; i<size; i++ ) {
      dst[i] = src[i];
   }
}

int StrCmp(char *str1, char *str2){ 

   while(1) {
      if(*str1 != *str2) {
         return FALSE;
      }
       
      if(*str2 == '\0' && *str1 == '\0') { //added str1 condition
         return TRUE;
      }else if(*str2 == '\0' || *str1 == '\0'){
	 return FALSE;
      }
      str1++;
      str2++;
   }
}

void Itoa(char *str, int x){  

   int i;
   int n = 0;
   int temp_x = x;

   if(x >= 100000)
      return;

   if(x <= 9){
      n = 1;
   }else{
      while(temp_x != 0){
	 temp_x /= 10;
	 n++;
      }
   }

   for(i=0; i < n; i++) {
      str[n-1-i] = x % 10 + '0';
      x /= 10;
   }

   str[n] = '\0'; //add null character to end of string
   return;
}
