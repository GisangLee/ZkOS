// kernel/idt.c - 링커 에러 원천 차단 버전
#include "idt.h"

struct idt_entry idt[256];
struct idt_ptr idtp;

// 키보드 스캔 코드를 아스키 코드로 바꾸는 매핑 테이블
unsigned char keyboard_map[128] = {
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
  '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',   0,
 '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/',   0, '*',
    0, ' ', 0
};

int cursor_pos = (12 * 80 + 20) * 2; // 글자가 찍힐 초기 화면 위치

// C 내부에서만 사용할 함수이므로 static inline으로 선언하여 링커 간섭을 막습니다.
static inline void set_idt_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags) {
    idt[num].base_low = (base & 0xFFFF);
    idt[num].base_high = (base >> 16) & 0xFFFF;
    idt[num].selector = sel;
    idt[num].always0 = 0;
    idt[num].flags = flags;
}

// 진짜 키보드 인터럽트 처리 함수
void keyboard_handler() {
    uint8_t scancode = inb(0x60);

    if (!(scancode & 0x80)) {
        char letter = keyboard_map[scancode];
        if (letter > 0) {
            char* video_memory = (char*) 0xb8000;
            video_memory[cursor_pos] = letter;
            video_memory[cursor_pos + 1] = 0x0A; // 밝은 초록색
            cursor_pos += 2;
        }
    }

    outb(0x20, 0x20);
    outb(0xA0, 0x20);
}

// kernel/idt.c 내의 기존 keyboard_interrupt_entry 함수를 아래 코드로 덮어씌웁니다.

// 컴파일러가 임의의 프레임 코드를 넣지 않도록 'naked' 속성을 부여합니다.
// 이제 이 함수는 오직 우리가 적은 순수 어셈블리 명령어로만 채워집니다.
__attribute__((naked)) void keyboard_interrupt_entry() {
    __asm__ __volatile__(
        "pushal\n\t"                // 1. 모든 범용 레지스터 스택에 백업
        "cld\n\t"                   // 2. C 언어 표준 방향 플래그 초기화
        "call _keyboard_handler\n\t" // 3. 실제 키보드 처리 함수 호출 (언더바 필수)
        "popal\n\t"                 // 4. 레지스터 복원
        "iretl\n\t"                 // 5. 32비트 인터럽트 전용 복귀 명령
    );
}

// PIC 초기화
static inline void pic_init() {
    outb(0x20, 0x11); outb(0xA0, 0x11);
    outb(0x21, 0x20); outb(0xA1, 0x28);
    outb(0x21, 0x04); outb(0xA1, 0x02);
    outb(0x21, 0x01); outb(0xA1, 0x01);
    outb(0x21, 0xFD); outb(0xA1, 0xFF);
}

// IDT 최종 빌드 및 로드
void init_idt() {
    idtp.limit = (sizeof(struct idt_entry) * 256) - 1;
    idtp.base = (uint32_t)&idt;

    for(int i = 0; i < 256; i++) {
        set_idt_gate(i, 0, 0, 0);
    }

    uint32_t handler_addr = (uint32_t)keyboard_interrupt_entry;
    // [수정] 플래그 값을 0x8E에서 0x8F로 변경하여 트랩 게이트 안정성을 높입니다.
    set_idt_gate(33, handler_addr, 0x08, 0x8F);

    pic_init(); 

    __asm__ __volatile__("lidt _idtp"); 
    __asm__ __volatile__("sti");
}