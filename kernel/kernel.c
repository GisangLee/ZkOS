// kernel/kernel.c
#include "idt.h"

void kernel_main() {
    char* video_memory = (char*) 0xb8000;

    // 화면 지우기
    for (int i = 0; i < 80 * 25 * 2; i += 2) {
        video_memory[i] = ' ';     
        video_memory[i+1] = 0x07;  
    }

    // 환영 메시지 출력
    char* message = "Welcome to ZkOS! Kernel Boot Successful!";
    int offset = (10 * 80 + 20) * 2;

    int j = 0;
    while (message[j] != '\0') {
        video_memory[offset] = message[j];
        video_memory[offset + 1] = 0x2F; // 초록 배경
        offset += 2;
        j++;
    }

    // [대망의 추가] 인터럽트 및 키보드 시스템 가동!!
    init_idt();

    // 커널이 종료되지 않고 키보드 신호를 계속 처리하도록 무한 대기
    while(1);
}