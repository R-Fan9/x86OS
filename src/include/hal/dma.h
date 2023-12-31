#include "C/stdint.h"

#ifndef DMA_H
#define DMA_H

enum DMA0_IO {
  DMA0_STATUS_REG = 0x08,
  DMA0_COMMAND_REG = 0x08,
  DMA0_REQUEST_REG = 0x09,
  DMA0_CHANMASK_REG = 0x0A,
  DMA0_MODE_REG = 0x0B,
  DMA0_CLEARBYTE_FLIPFLOP_REG = 0x0C,
  DMA0_TEMP_REG = 0x0D,
  DMA0_MASTER_CLEAR_REG = 0x0D,
  DMA0_CLEAR_MASK_REG = 0x0E,
  DMA0_MASK_REG = 0x0F
};

enum DMA1_IO {
  DMA1_STATUS_REG = 0xD0,
  DMA1_COMMAND_REG = 0xD0,
  DMA1_REQUEST_REG = 0xD2,
  DMA1_CHANMASK_REG = 0xD4,
  DMA1_MODE_REG = 0xD6,
  DMA1_CLEARBYTE_FLIPFLOP_REG = 0xD8,
  DMA1_INTER_REG = 0xDA,
  DMA1_UNMASK_ALL_REG = 0xDC,
  DMA1_MASK_REG = 0xDE
};

enum DMA0_CHANNEL_IO {
  DMA0_CHAN0_ADDR_REG = 0, //! Thats right, i/o port 0
  DMA0_CHAN0_COUNT_REG = 1,
  DMA0_CHAN1_ADDR_REG = 2,
  DMA0_CHAN1_COUNT_REG = 3,
  DMA0_CHAN2_ADDR_REG = 4,
  DMA0_CHAN2_COUNT_REG = 5,
  DMA0_CHAN3_ADDR_REG = 6,
  DMA0_CHAN3_COUNT_REG = 7,
};

enum DMA1_CHANNEL_IO {
  DMA1_CHAN4_ADDR_REG = 0xC0,
  DMA1_CHAN4_COUNT_REG = 0xC2,
  DMA1_CHAN5_ADDR_REG = 0xC4,
  DMA1_CHAN5_COUNT_REG = 0xC6,
  DMA1_CHAN6_ADDR_REG = 0xC8,
  DMA1_CHAN6_COUNT_REG = 0xCA,
  DMA1_CHAN7_ADDR_REG = 0xCC,
  DMA1_CHAN7_COUNT_REG = 0xCE,
};

enum DMA0_PAGE_REG {
  DMA_PAGE_EXTRA0 = 0x80, //! Also diagnostics port
  DMA_PAGE_CHAN2_ADDRBYTE2 = 0x81,
  DMA_PAGE_CHAN3_ADDRBYTE2 = 0x82,
  DMA_PAGE_CHAN1_ADDRBYTE2 = 0x83,
  DMA_PAGE_EXTRA1 = 0x84,
  DMA_PAGE_EXTRA2 = 0x85,
  DMA_PAGE_EXTRA3 = 0x86,
  DMA_PAGE_CHAN6_ADDRBYTE2 = 0x87,
  DMA_PAGE_CHAN7_ADDRBYTE2 = 0x88,
  DMA_PAGE_CHAN5_ADDRBYTE2 = 0x89,
  DMA_PAGE_EXTRA4 = 0x8C,
  DMA_PAGE_EXTRA5 = 0x8D,
  DMA_PAGE_EXTRA6 = 0x8E,
  DMA_PAGE_DRAM_REFRESH = 0x8F //! no longer used in new PCs
};

enum DMA_CMD_REG_MASK {
  DMA_CMD_MASK_MEMTOMEM = 1,
  DMA_CMD_MASK_CHAN0ADDRHOLD = 2,
  DMA_CMD_MASK_ENABLE = 4,
  DMA_CMD_MASK_TIMING = 8,
  DMA_CMD_MASK_PRIORITY = 0x10,
  DMA_CMD_MASK_WRITESEL = 0x20,
  DMA_CMD_MASK_DREQ = 0x40,
  DMA_CMD_MASK_DACK = 0x80
};

enum DMA_MODE_REG_MASK {

  DMA_MODE_MASK_SEL = 3,

  DMA_MODE_MASK_TRA = 0xC,
  DMA_MODE_SELF_TEST = 0,
  DMA_MODE_READ_TRANSFER = 4,
  DMA_MODE_WRITE_TRANSFER = 8,

  DMA_MODE_MASK_AUTO = 0x10,
  DMA_MODE_MASK_IDEC = 0x20,

  DMA_MODE_MASK = 0xC0,
  DMA_MODE_TRANSFER_ON_DEMAND = 0,
  DMA_MODE_TRANSFER_SINGLE = 0x40,
  DMA_MODE_TRANSFER_BLOCK = 0x80,
  DMA_MODE_TRANSFER_CASCADE = 0xC0
};

// set the base address of a channel
void dma_set_address(uint8_t channel, uint8_t low, uint8_t high);

// set the count register of a channel
void dma_set_count(uint8_t channel, uint8_t low, uint8_t high);

// set external page register
void dma_set_external_page_register(uint8_t reg, uint8_t val);

// set the mode of a channel
void dma_set_mode(uint8_t channel, uint8_t mode);

// set read mode
void dma_set_read(uint8_t channel);

// set write mode
void dma_set_write(uint8_t channel);

// mask (disable) a channel
void dma_mask_channel(uint8_t channel);

// unmask (enable) a channel
void dma_unmask_channel(uint8_t channel);

// clear byte pointer flip-flop
void dma_reset_flipflop(uint8_t dma);

// reset DMA
void dma_reset();

// unmask all registers
void dma_unmask_all();

// initialize DMA for floppy disk controller
void dma_init_floppy(uint8_t *buffer, unsigned length);

#endif // !DMA_H
