#include <linux/syscalls.h>
#include <linux/kernel.h>
#include <linux/linkage.h>

#define QUEUE_SIZE 100

int queue[QUEUE_SIZE];
int rear = -1;
int front = 0;
int i = 0;
int temp = 0;

SYSCALL_DEFINE1(os2024_enqueue, int, a) {
    // if queue is full
    if (rear == QUEUE_SIZE - 1) {
        printk("error : queue is full!\n");
        return -1;
    }

    // if queue already has the value
    for (i = front; i <= rear; i++) {
        if (queue[i] == a) {
            printk("error : value %d is already in queue\n", a);
            return -1;
        }
    }

    // update queue
    queue[rear] = a;

    // print queue
    printk(KERN_INFO "[System call] os2024_enqueue()\n");
    printk(KERN_INFO "Queue Front --------------------\n");
    for (i = front; i <= rear; i++) {
        printk(KERN_INFO "%d\n", queue[i]);
    }
    printk(KERN_INFO "Queue Rear ---------------------\n");
    
    //queue rear update
    rear++;

    // return enqueued value
    return a;
}

SYSCALL_DEFINE0(os2024_dequeue) {

    if (rear == -1) {
        printk("error : queue is empty!\n");
        return -1;
    }

    // store dequeued value
    temp = queue[front];

    // Shift elements to the left
    for (i = front; i < rear; i++) {
        queue[i] = queue[i + 1];
    }
    
    queue[rear] = 0; // clear the last element
    rear--; // update queue's rear

    // print queue
    printk(KERN_INFO "[System call] os2024_dequeue()\n");
    printk(KERN_INFO "Queue Front --------------------\n");
    for (i = front; i <= rear; i++) {
        printk(KERN_INFO "%d\n", queue[i]);
    }
    printk(KERN_INFO "Queue Rear ---------------------\n");

    // return dequeued value
    return temp;
}
