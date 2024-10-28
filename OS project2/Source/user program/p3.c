#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define KU_CPU 339 // RR

int main(int argc, char ** argv) {
    int jobTime;
    int delayTime;
    char name[4];

    int wait = 0;
    int response;
    int first = 0;
        

    if (argc < 4) {
        printf("\nInsufficient Arguments..\n");
        return 1;
    }

    /* first argument : job time (second)
       second argument : delay time before execution (second)
       third argument : process name
    */

    jobTime = atoi(argv[1]);
    delayTime = atoi(argv[2]);
    strcpy(name, argv[3]);

    // wait for 'delayTime' seconds before execution
    sleep(delayTime);
    printf("\nProcess %s : I will use CPU by %ds. \n", name, jobTime);
    jobTime *= 10; //execute system call in every 0.1 second

    while(jobTime){
        // system call is succeed, decrease job time
        if(!syscall(KU_CPU, name, jobTime)) {
            --jobTime;
        // if it is first time to allocate CPU
            if(!first_run) {
                first_run = 1;
                response = wait;
            }
        }
        // if request is rejected, increase wait time
        else ++wait;
        usleep (100000); // delay 0.1 second
    }

    syscall(KU_CPU, name, 0);
    printf("\nProcess %s : Finish! My response time is %ds and My total wait time is %ds. ", name, (response + 5) / 10, (wait + 5) / 10);
    return 0;
}
