#ifndef NVME_H
#define NVME_H

#include <stdint.h>
#include <pthread.h>

#define QUEUE_SIZE 8 // 큐의 크기를 8로 제한

// 1. NVMe 명령어 코드 (Opcode)
typedef enum{
    NVME_OP_READ = 0x01, 
    NVME_OP_WRITE = 0x02
} nvme_opcode_t;

// 2. SQE (Submission Queue Entry): 호스트가 SSD에 내리는 명령 엔트리
typedef struct{
    nvme_opcode_t opcode;   // 읽기/쓰기
    uint16_t cid;           // Command ID
    uint32_t lba;           // 논리적 블록 주소
    uint32_t length;        // 데이터 길이
} nvme_sq_entry_t;

// 3. CQE (Completion Queue Entry): SSD가 호스트에게 주는 결과 엔트리
typedef struct{
    uint16_t cid;           // 어떤 명령어에 대한 결과인지
    uint16_t status;        // 0: 성공, 1: 실패
    uint16_t sq_head;       // SSD가 SQ를 어디까지 처리했는지 호스트에게 알려줌
} nvme_cq_entry_t;

// 4. Queue 관리 구조체 (원형 큐 - Circular Queue)
typedef struct {
    nvme_sq_entry_t sq[QUEUE_SIZE];    // 메모리 상의 SQ 공간
    nvme_cq_entry_t cq[QUEUE_SIZE];    // 메모리 상의 CQ 공간

    // DoorBell Registers (실제로는 하드웨어 레지스터)
    int sq_tail;    // 호스트가 명령을 넣고 업데이트 하는 꼬리
    int sq_head;    // 컨트롤러가 명령을 빼가고 업데이트 하는 머리 (DoorBell)

    int cq_tail;    // 컨트롤러가 결과를 넣고 업데이트 하는 꼬리
    int cq_head;    // 호스트카 결과를 확인하고 업데이트 하는 머리 (DoorBell)

    pthread_mutex_t lock;
    pthread_cond_t host_bell;       // 컨트롤러->호스트 인터럽트 (CQ 업데이트 완료)
    pthread_cond_t controller_bell; // 호스트->컨트롤러 도어벨 (SQ 업데이트 완료)
} nvme_queue_t;

// 전역 큐 인스턴스 (main.c에서 초기화 예정)
extern nvme_queue_t admin_q;

#endif