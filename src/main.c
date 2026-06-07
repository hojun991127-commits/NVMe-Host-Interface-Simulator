#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include "../include/nvme.h"

nvme_queue_t admin_q;

extern void* host_thread_func(void* arg);
extern void* controller_thread_func(void* arg);

void init_nvme_queue(){
    admin_q.sq_head = 0;
    admin_q.sq_tail = 0;
    admin_q.cq_head = 0;
    admin_q.cq_tail = 0;

    pthread_mutex_init(&admin_q.lock, NULL);
    pthread_cond_init(&admin_q.host_bell, NULL);
    pthread_cond_init(&admin_q.controller_bell, NULL);
}

int main(){
    printf("=== NVMe Host Interface Simulator (Interrupt Mode) ===\n\n");

    init_nvme_queue();

    pthread_t host_tid, ctrl_tid;

    // 호스트 스레드(OS)와 컨트롤러 스레드(SSD) 동시 실행
    pthread_create(&ctrl_tid, NULL, controller_thread_func, NULL);
    sleep(1);
    pthread_create(&host_tid, NULL, host_thread_func, NULL);

    //호스트의 일이 끝날 때까지 대기
    pthread_join(host_tid, NULL);

    //호스트가 종료되면 강제로 컨트롤러도 종료
    pthread_cancel(ctrl_tid);
    pthread_join(ctrl_tid, NULL);

    printf("\n=== Simultaion Completed Successfully ===\n");
    return 0;
}