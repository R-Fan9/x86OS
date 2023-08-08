#include "keyboard.h"
#include "keymap.h"
#include "C/stdint.h"
#include "ports/io.h"
#include "debug/display.h"
#include "hal/pic.h"

void keyboard_irq1_handler() { 
  uint8_t status = inb(KEYBOARD_STATUS); 
  if(status & 0x01){
    int8_t keycode = inb(KEYBOARD_DATA);
    if(keycode >= 0){
      print_char(keyboard_map[(uint8_t)keycode]);
    }
  }
  send_pic_eoi(1);
}