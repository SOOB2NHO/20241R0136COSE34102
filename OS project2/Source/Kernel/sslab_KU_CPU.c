#include <linux/syscalls.h>
#include <linux/kernel.h>
#include <linux/linkage.h>
#include <linux/sched.h>
#include <linux/string.h>
#include <linux/slab.h>

#define QUEUE_SIZE 100
#define IDLE -1

typedef struct {
    pid_t pid;
    int jobTime;
    int priority;
} job_t;

static int i;
static int rear = -1;
static int timeslice;
static int nowjobTime = 100;
static int nowPriority = 100;
static job_t wq[QUEUE_SIZE];
static pid_t now = IDLE;

static int ku_is_full(void) {
    return rear == QUEUE_SIZE - 1;
}

static int ku_is_empty(void) {
    return rear == -1;
}

static int ku_is_new_id(pid_t check) {
    for (i = 0; i <= rear; ++i) {
        if (wq[i].pid == check) return 0;
    }
    return 1;
}

static void ku_push(job_t data) {
    if (ku_is_full()) {
        printk("error: Queue is full.\n");
        return;
    }
    wq[++rear] = data;
}

static job_t ku_SRTF_pop(void) {
    int idx = 0;
    job_t popjob = wq[0];


    if (ku_is_empty()) {
        printk("error: Queue is empty.\n");
    }

    for (i = 1; i <= rear; i++) {
        if (wq[i].jobTime < popjob.jobTime) {
            idx = i;
            popjob = wq[i];
        }
    }

    rear--;

    for (i = idx; i <= rear; i++) {
        wq[i] = wq[i + 1];
    }

    return popjob;
}

static job_t ku_priority_pop(void) {
    int idx = 0;
    job_t popjob = wq[0];

    if (ku_is_empty()) {
        printk("error: Queue is empty.\n");
    }

    for (i = 1; i <= rear; i++) {
        if (wq[i].priority < popjob.priority) {
            idx = i;
            popjob = wq[i];
        }
    }

    rear--;

    for (i = idx; i <= rear; i++) {
        wq[i] = wq[i + 1];
    }

    return popjob;
}


static job_t ku_pop(void) {
    job_t popjob;
    if (ku_is_empty()) {
        printk("error: Queue is empty.\n");
    }
    else {
        popjob = wq[0];
        // remove first element and move forward
        for (i = 0; i < rear; i++) {
            wq[i] = wq[i + 1];
        }
        // Clear last element
        wq[rear].pid = 0;
        wq[rear].jobTime = 0;
        wq[rear].priority = 0;
        rear--;
    }
    return popjob;
}

SYSCALL_DEFINE2(os2024_ku_cpu_fcfs, char*, name, int, jobTime) {
    job_t newJob = {current->pid, jobTime, 0};

    if (now == IDLE) now = newJob.pid;

    if (now == newJob.pid) {
        // if job finished
        if (jobTime == 0) {
            printk("Process Finished: %s\n", name);
            // if queue is empty
            if (ku_is_empty()) now = IDLE;
            else now = ku_pop().pid;
        }
        else printk("Working: %s\n", name);
        // request accepted
        return 0;
    }

    if (ku_is_new_id(newJob.pid)) ku_push(newJob);
    printk("Working Denied: %s\n", name);
    // request rejected
    return 1;
}

// short remainig time first scheduling
SYSCALL_DEFINE2(os2024_ku_cpu_srtf, char*, name, int, jobTime) {
    job_t newJob = {current->pid, jobTime, 0};

    if (now == IDLE || newJob.jobTime < nowjobTime) {
        if (now == IDLE && ku_is_new_id(newJob.pid)) {
            job_t next = {now, nowjobTime, 0};
            ku_push(next);
        }
        now = newJob.pid;
        nowjobTime = newJob.jobTime;
    }

    if (now == newJob.pid) {
        if (jobTime == 0) {
            printk("Process Finished: %s\n", name);

            if (!ku_is_empty()) {
                job_t next = ku_SRTF_pop();
                now = next.pid;
                nowjobTime = next.jobTime;
            }

            else now = IDLE;
        }

        else {
            printk("Working: %s\n", name);
            nowjobTime--;
        }
        return 0;
    }
    if (ku_is_new_id(newJob.pid)) ku_push(newJob);
    printk("Working Denied: %s\n", name);
    return 1;
}

// round robin scheduling
SYSCALL_DEFINE2(os2024_ku_cpu_rr, char*, name, int, jobTime) {
    // store pid of current process
    job_t newJob = {current->pid, jobTime};

    // if process finished
    if (jobTime == 0) {
        printk("Process Finished: %s\n", name);
        if (ku_is_empty()) now = IDLE;
        else now = ku_pop().pid;
        timeslice = 0;
        return 0;
    }

    if (now == IDLE) now = newJob.pid;

    if (now == newJob.pid) {
        if (timeslice == 10) {
            printk("----> Turn Over: %s\n", name);

            if (ku_is_empty()) now = IDLE;

            else {
                now = ku_pop().pid;
                ku_push(newJob);
            }

            timeslice = 0;
            return 1;
        }
        else {
            printk("Working: %s\n", name);
            timeslice++;
        }
        return 0;
    }

    if (ku_is_new_id(newJob.pid)) ku_push(newJob);
    printk("Working Denied: %s\n", name);
    return 1;
}

//priority scheduling
SYSCALL_DEFINE3(os2024_ku_cpu_priority, char*, name, int, jobTime, int, priority) {
    job_t newJob = {current->pid, jobTime, priority};

    if (now == IDLE || newJob.priority < nowPriority) {

        if (now != IDLE) {
            job_t next = {now, nowjobTime, nowPriority};
            ku_push(next);
        }
        now = newJob.pid;
        nowjobTime = newJob.jobTime;
        nowPriority = newJob.priority;
    }


    if (now == newJob.pid) {

        if (jobTime == 0) {
            printk("Process Finished: %s\n", name);

            if (!ku_is_empty()) {
                job_t next = ku_priority_pop();
                now = next.pid;
                nowjobTime = next.jobTime;
                nowPriority = next.priority;
            }

            else  {
                now = IDLE;
                nowjobTime = 0;
                nowPriority = 100;
            }
        }

        else {
            printk("Working: %s\n", name);
            nowjobTime--;
        }
        return 0;
    }

    if(ku_is_new_id(newJob.pid)) ku_push(newJob);
    printk("Working Denied: %s\n", name);
    return 1;
}
