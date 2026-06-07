#include <stdio.h>
#include <unistd.h>
#include "../include/nvme.h"

void* controller_thread_func(void* arg){
    while(1){   //컨트롤러는 전원이 꺼질 때까지
        pthread_mutex_lock(&admin_q.lock);

        //SQ가 비어있으면 도어벨이 울릴 때까지 수면 대기
        while(admin_q.sq_head == admin_q.sq_tail){
            pthread_cond_wait(&admin_q.controller_bell, &admin_q.lock);
        }

        // [Step 2] 도어벨 소리를 듣고 깨어나 SQ에서 명령 가져오기 (Fetch)
        nvme_sq_entry_t sqe = admin_q.sq[admin_q.sq_head];
        admin_q.sq_head = (admin_q.sq_head + 1) % QUEUE_SIZE;

        printf("[Contoller] SQ에서 명령 가져옴 (CID: %d, Opcode: %d)\n", sqe.cid, sqe.opcode);
        pthread_mutex_unlock(&admin_q.lock);

        // FTL_WRITE / READ 실행

        printf("    => [FTL] NAND Flash LBA %d번지 기록 중...\n", sqe.lba);
        usleep(800000); // 낸드 기록에 걸리는 시간 (0.8초)

        // 기록 완료 후 CQ에 결과 갱신
        pthread_mutex_lock(&admin_q.lock);

        // [Step 3] CQ에 결과 보고서 작성
        admin_q.cq[admin_q.cq_tail].cid = sqe.cid;
        admin_q.cq[admin_q.cq_tail].status = 0;     // 0: SUCCESS
        admin_q.cq[admin_q.cq_tail].sq_head = admin_q.sq_head;

        admin_q.cq_tail = (admin_q.cq_tail + 1) % QUEUE_SIZE;

        printf("[Controller] CQ가 보고서 작성 완료 (CID: %d) | Host로 Interrupt 전송\n", sqe.cid);

        // 인터럽트 발생
        pthread_cond_signal(&admin_q.host_bell);
        pthread_mutex_unlock(&admin_q.lock);
    }
    return NULL;
}