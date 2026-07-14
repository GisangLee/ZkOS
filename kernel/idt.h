// kernel/idt.h

#ifndef IDT_H
#define IDT_H

#include <stdint.h>

struct idt_entry {
    uint16_t base_low;
    uint16_t selector;
    uint8_t  always0;
    uint8_t  flags;
    uint16_t base_high;
} __attribute__((packed));

struct idt_ptr {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ __volatile__("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static inline void outb(uint16_t port, uint8_t val) {
    __asm__ __volatile__("outb %0, %1" : : "a"(val), "Nd"(port));
}

void init_idt();

// [수정] 이 줄을 아래처럼 'static inline'으로 변경하거나 아예 삭제해 주세요!
// void set_idt_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags);
static inline void set_idt_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags);

#endif