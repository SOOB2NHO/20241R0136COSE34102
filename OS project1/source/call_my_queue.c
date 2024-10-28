#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>


#define my_queue_enqueue 335 // my_queue_enqueue -> mapping 335
#define my_queue_dequeue 336 // my_queue_dequeue -> mapping 336

int main() {
    int r = 0;
    int val[3];
    int i = 0;
    int j = 0;
    srand(time(NULL));

    // enqueued value
    for(i = 0; i < 3; i++) {
        r = rand() % 100;
        syscall(my_queue_enqueue, r);

        //check value is already in queue
        for(j = 0; j < i; j++) {
            if(val[j] == r) { 
                printf("error : value is already in the queue\n");
                return -1;
            }
        }
        //store the value to check 
        val[i] = r;

        printf("Enqueued: %d\n", r);
    }

    // dequeued value
    for(i = 0; i < 3; i++) {
        r = syscall(my_queue_dequeue);
        //check queue is empty
        if(r == -1) {
            printf("error: Queue is empty\n");
            break;
        }
        printf("Dequeued: %d\n", r);
    }
    
    return 0;
}
