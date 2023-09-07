#include "C/ctype.h"
#include "C/stdint.h"
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
#define PREKERNEL_ADDRESS 0x50000
#define PREKERNEL_SIZE_ADDRESS 0x8000
#define KERNEL_ADDRESS 0x100000
#define HHKERNEL_ADDRESS 0xC0000000
#define KERNEL_IMAGE "KRNL    SYS"

typedef struct SMAP_entry {
  uint64_t base;
  uint64_t size;
  uint32_t type;
  uint32_t acpi;
} __attribute__((packed)) SMAP_entry_t;

extern void jump_usermode();

void hal_init();
void setup_interrupt_handlers();

__attribute__((section("prekernel_setup"))) void pkmain(void) {
  clear_screen();

  // initialize hardware abstraction layer (GDT, IDT, PIC)
  hal_init();

  // setup exception, software & hardware interrupt handlers
  setup_interrupt_handlers();

  // initialize TSS
  tss_init(5, 0x10, 0);

  uint32_t prekernel_size = *(uint32_t *)PREKERNEL_SIZE_ADDRESS;
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

  // deinitialize memory region where the memory map is in
  pmmngr_deinit_region(MEMMAP_ADDRESS,
                       pmmngr_get_block_count() / BLOCKS_PER_BYTE);

  // deinitialize memory region where the prekernel is in
  pmmngr_deinit_region(PREKERNEL_ADDRESS, prekernel_size * 512);

  // initialize floppy disk controller
  fd_init(0);

  // initialize file system
  fat_init();

  // load kernel into physical address 0x100000
  FILE kernel = vol_open_file((uint8_t *)KERNEL_IMAGE);
  uint8_t *buffer = (uint8_t *)KERNEL_ADDRESS;
  uint32_t kernel_size = 0;

  while (!kernel.end) {
    vol_read_file(&kernel, &buffer[kernel_size]);
    kernel_size += SECTOR_SIZE;
  }

  // deinitialize memory region below 0x16000 for BIOS & Bootloader
  pmmngr_deinit_region(0x1000, 0x15000);

  // deinitialize memory region where the kernel is in
  pmmngr_deinit_region((physical_addr)(uint32_t *)KERNEL_ADDRESS, kernel_size);

  pmmngr_display_blocks();

  // initialize virtual memory manager & enable paging
  vmmngr_init();

  int32_t stack = 0;
  __asm__ __volatile__("movl %%ESP, %0" : "=r"(stack));
  tss_set_stack(0x10, stack);
  jump_usermode();
}

void usermode(void) {
  clear_screen();

  // execute higher half kernel
  ((void (*)(void))HHKERNEL_ADDRESS)();
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
