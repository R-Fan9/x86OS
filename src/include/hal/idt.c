#include "idt.h"
#include "C/stdint.h"
#include "C/string.h"

static struct idt_descriptor idt[IDT_SIZE];
static struct idtr idt_ptr;

void idt_install() { __asm__ __volatile__("lidt %0" : : "m"(idt_ptr)); }

void idt_set_descriptor(uint32_t i, void *isr, uint16_t flags) {
  if (i > IDT_SIZE) {
    return;
  }

  memset((void *)&idt[0], 0, sizeof(struct idt_descriptor));

  idt[i].base_low = (uint32_t)isr & 0xFFFF;
  idt[i].selector = 0x08;
  idt[i].reserved = 0;
  idt[i].flags = flags;
  idt[i].base_high = ((uint32_t)isr >> 16) & 0xFFFF;
}

int idt_init() {
  idt_ptr.limit = (sizeof(struct idt_descriptor) * IDT_SIZE) - 1;
  idt_ptr.base = (uint32_t)&idt[0];

  memset((void *)&idt[0], 0, sizeof(struct idt_descriptor) * IDT_SIZE - 1);

  idt_install();

  return 0;
}