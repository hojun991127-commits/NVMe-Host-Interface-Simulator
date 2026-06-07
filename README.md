# NVMe Host Interface Simulator

> C/C++ 기반의 NVMe Submission Queue (SQ) 및 Completion Queue (CQ) 비동기 통신 시뮬레이터

## Project Overview
 **운영체제(OS/Host)와 SSD 컨트롤러 간의 통신 프로토콜(NVMe)** 모사
SATA의 한계를 극복한 PCIe 기반 NVMe의 핵심 메커니즘인 다중 원형 큐(Circular Queue)와 비동기 I/O를 `pthread` 멀티스레딩으로 구현

## Core Architecture

- **Host Thread (OS/CPU):** - NVMe 명령어(`nvme_sq_entry_t`)를 생성하여 Submission Queue(SQ)에 삽입(Enqueue).
  - 도어벨(Doorbell) 레지스터를 업데이트하여 컨트롤러에 작업 지시.
- **Controller Thread (SSD 펌웨어):**
  - SQ 도어벨을 감지하고 명령어를 Fetch.
  - 가상의 낸드 쓰기 작업(FTL 모사) 수행 후 Completion Queue(CQ)에 결과 기록.
  - 호스트에 인터럽트(Interrupt) 신호를 전송하여 작업 완료 통보.

## Key Technologies
- **Multi-threading:** `pthread`를 활용하여 호스트와 디바이스의 독립적이고 비동기적인 동작 구현.
- **Synchronization:** `pthread_mutex_t`와 `pthread_cond_t`를 활용하여 Race Condition을 방지하고 하드웨어 인터럽트 및 도어벨(MMIO) 동작을 소프트웨어적으로 모사.
- **Ring Buffer:** 메모리 복사 오버헤드(Shift)가 없는 원형 큐(Circular Queue) 구조를 채택하여 고속 I/O 환경 모사.

## Build & Run
```bash
make clean
make
./nvme_sim