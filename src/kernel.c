#include "C/ctype.h"
#include "C/stdint.h"
#include "C/stdio.h"
#include "C/string.h"
#include "debug/display.h"
#include "filesystem/fat12.h"
#include "filesystem/file.h"
#include "filesystem/floppydisk.h"
#include "hal/gdt.h"
#include "hal/idt.h"
#include "hal/pic.h"
#include "hal/tss.h"
#include "interrupts/exceptions.h"
#include "interrupts/keyboard.h"
#include "interrupts/pit.h"
#include "interrupts/syscall.h"
#include "memory/physical_mmngr.h"
#include "memory/virtual_mmngr.h"

#define SMAP_ENTRY_COUNT_ADDRESS 0x1000
#define SMAP_ENTRY_ADDRESS 0x1004
#define MEMMAP_ADDRESS 0x30000
#define KERNEL_ADDRESS 0x100000
#define KERNEL_SIZE_ADDRESS 0x8000

typedef struct SMAP_entry {
  uint64_t base;
  uint64_t size;
  uint32_t type;
  uint32_t acpi;
} __attribute__((packed)) SMAP_entry_t;

extern void jmp_userspace();

void hal_init();
void setup_interrupt_handlers();

__attribute__((section("kernel_main"))) void kmain(void) {
  clear_screen();

  uint32_t kernel_size = *(uint32_t *)KERNEL_SIZE_ADDRESS;
  uint32_t entry_count = *(uint32_t *)SMAP_ENTRY_COUNT_ADDRESS;
  SMAP_entry_t *entry = (SMAP_entry_t *)SMAP_ENTRY_ADDRESS;
  SMAP_entry_t *last_entry = entry + entry_count - 1;
  uint32_t total_memory = last_entry->base + last_entry->size - 1;

  // initialize physical memory manager
  pmmngr_init(MEMMAP_ADDRESS, total_memory);

  for (uint32_t i = 0; i < entry_count; i++, entry++) {
    // entry with type 1 indicates the memory region is available
    if (entry->type == 1) {
      pmmngr_init_region(entry->base, entry->size);
    }
  }

  // initialize hardware abstraction layer (GDT, IDT, PIC, TSS)
  hal_init();

  // setup exception, software & hardware interrupt handlers
  setup_interrupt_handlers();

  // initialize floppy disk controller
  fd_init(0);

  // initialize file system
  fat_init();

  // deinitialize memory region below 0x16000 for BIOS & Bootloader
  pmmngr_deinit_region(0x1000, 0x15000);

  // deinitialize memory region where the memory map is in
  pmmngr_deinit_region(MEMMAP_ADDRESS,
                       pmmngr_get_block_count() / BLOCKS_PER_BYTE);

  // deinitialize memory region where the kernel is in
  pmmngr_deinit_region(KERNEL_ADDRESS, kernel_size * 512);

  // initialize virtual memory manager
  // enable paging
  // map kernel to address 0xC0000000 (higher half kernel)
  vmmngr_init();

  int32_t stack = 0;
  __asm__ __volatile__("movl %%ESP, %0" : "=r"(stack));
  tss_set_stack(0x10, stack);
  jmp_userspace();
}

void hal_init() {

  // set up Global Descritor Table
  gdt_init();

  // set up Interrupt Descritor Table
  idt_init();

  // mask off all hardware interrupts, disable PIC
  disable_pic();

  // remap PIC IRQ interrupts (IRQ0 starts at 0x20)
  pic_init();

  // initialize TSS
  tss_init(5, 0x10, 0);
}

void setup_interrupt_handlers() {

  // set up exception handlers (i.e, divide by 0, page fault..)
  idt_set_descriptor(14, page_fault_handler, TRAP_GATE_FLAGS);

  // set up software interrupts (system call handler)
  idt_set_descriptor(0x80, syscall_dispatcher, INT_GATE_USER_FLAGS);

  // add ISRs for PIC hardware interrupts
  idt_set_descriptor(0x20, timer_irq0_handler, INT_GATE_FLAGS);
  idt_set_descriptor(0x21, keyboard_irq1_handler, INT_GATE_FLAGS);
  idt_set_descriptor(0x26, fd_irq6_handler, INT_GATE_FLAGS);

  // enable PIC IRQ interrupts after setting up their descriptors
  clear_irq_mask(0); // enable timer - IRQ0
  clear_irq_mask(1); // enable keyboard - IRQ1
  clear_irq_mask(2); // enable PIC2 line - IRQ2
  clear_irq_mask(6); // enable floppy disk - IRQ6

  // set default PIT Timer IRQ0 rate - ~1000hz
  // 1193182 MHZ / 1193 = ~1000
  pit_set_channel_mode_frequency(0, 1000, PIT_OCW_MODE_RATEGEN);

  // enable interrupts
  __asm__ __volatile__("sti");
}
