#include <stdio.h>
#include <unistd.h>
#include "../include/nvme.h"

#define TOTAL_CMDS 5    // SQ에 쌓일 명령어 수

void* host_thread_func(void* arg){
    // 1. 호스트가 5개의 명령을 생성하여 SQ에 넣음
    for(int i = 0; i < TOTAL_CMDS; ++i){
        pthread_mutex_lock(&admin_q.lock);   // 큐 자물쇠 잠금

        // SQ가 꽉 찼는지 확인 (원형 큐)
        while(((admin_q.sq_tail + 1) % QUEUE_SIZE) == admin_q.sq_head){
            printf("[Host] SQ is Full. Waiting for controller...\n");
            pthread_cond_wait(&admin_q.host_bell, &admin_q.lock);
        }
        
        // [Step 1] SQ에 명령어 작성
        admin_q.sq[admin_q.sq_tail].opcode = NVME_OP_WRITE;
        admin_q.sq[admin_q.sq_tail].cid = i + 1; // 명령어 ID (1~5);
        admin_q.sq[admin_q.sq_tail].lba = 100 * i;
        admin_q.sq[admin_q.sq_tail].length = 4096;

        admin_q.sq_tail = (admin_q.sq_tail + 1) % QUEUE_SIZE;

        printf("[Host] -> SQ 작성 완료 (CID: %d) | SQ Doorbell 울림! (Tail: %d)\n", i + 1, admin_q.sq_tail);

        // 도어벨 울림
        pthread_cond_signal(&admin_q.controller_bell);

        pthread_mutex_unlock(&admin_q.lock); // 자물쇠 풀기
        usleep(500000); // 0.5초마다 명령 전송 (OS가 다른 일 하는 시간)
    }

    // 2. 호스트가 CQ 결과를 기다림 (인터럽트 대기)
    int completed = 0;
    while(completed < TOTAL_CMDS){
        pthread_mutex_lock(&admin_q.lock);

        // CQ가 비어있으면 인터럽트가 올 때까지 대기
        while(admin_q.cq_head == admin_q.cq_tail){
            pthread_cond_wait(&admin_q.host_bell, &admin_q.lock);
        }

        // [Step 4] CQ 결과 확인 및 도어벨 갱신
        nvme_cq_entry_t cqe = admin_q.cq[admin_q.cq_head];
        admin_q.cq_head = (admin_q.cq_head + 1) % QUEUE_SIZE;
        completed++;

        printf("[Host] <- CQ 확인 완료 (CID: %d, Status: %d) | CQ Doorbell 울림 (Head: %d)\n", cqe.cid, cqe.status, admin_q.cq_head);
        pthread_mutex_unlock(&admin_q.lock);
    }
    return NULL;
}