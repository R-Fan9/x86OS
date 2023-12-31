#include "C/stdint.h"

#ifndef IDT_H
#define IDT_H

#define IDT_SIZE 256
#define TRAP_GATE_FLAGS 0x8F
#define INT_GATE_FLAGS 0x8E
#define INT_GATE_USER_FLAGS 0xEE

typedef struct _idt_descriptor {
  // bits 0-16 of interrupt routine (IR) address
  uint16_t base_low;

  // code selector in gdt
  uint16_t selector;

  // reserved, should be 0
  uint8_t reserved;

  // bits flags
  uint8_t flags;

  // bits 16-32 of IR address
  uint16_t base_high;
} __attribute__((packed)) idt_descriptor;

typedef struct _idtr {
  // size of idt
  uint16_t limit;

  // base address of idt
  uint32_t base;
} __attribute__((packed)) idtr;

// Interrupt "frame" to pass to interrupt handlers/ISRs
typedef struct {
  uint32_t eip;
  uint32_t cs;
  uint32_t eflags;
  uint32_t sp;
  uint32_t ss;
} __attribute__((packed)) int_frame_t;

void idt_set_descriptor(uint32_t i, void *irq, uint16_t flags);
void idt_init();

#endif // !IDT_H
