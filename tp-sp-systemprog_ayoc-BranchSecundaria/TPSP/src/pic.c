/* ** por compatibilidad se omiten tildes **
================================================================================
 TALLER System Programming - ORGANIZACION DE COMPUTADOR II - FCEN
================================================================================

  Rutinas del controlador de interrupciones.
*/
#include "pic.h"

#define PIC1_PORT 0x20
#define PIC2_PORT 0xA0

static __inline __attribute__((always_inline)) void outb(uint32_t port,
                                                         uint8_t data) {
  __asm __volatile("outb %0,%w1" : : "a"(data), "d"(port));
}
void pic_finish1(void) { outb(PIC1_PORT, 0x20); }
void pic_finish2(void) {
  outb(PIC1_PORT, 0x20);
  outb(PIC2_PORT, 0x20);
}

// COMPLETAR: implementar pic_reset()
void pic_reset() {
  // ICW1: iniciar secuencia de inicialización
  outb(PIC1_PORT, 0x11);
  outb(PIC2_PORT, 0x11);

  // ICW2: remapear interrupciones
  outb(PIC1_PORT + 1, 0x20);  // PIC1: IRQ0–7 → INT 0x20–0x27
  outb(PIC2_PORT + 1, 0x28);  // PIC2: IRQ8–15 → INT 0x28–0x2F

  // ICW3: configuración de encadenamiento
  outb(PIC1_PORT + 1, 0x04);  // PIC1: esclavo en IRQ2 (bit 2)
  outb(PIC2_PORT + 1, 0x02);  // PIC2: conectado al IRQ2 del PIC1

  // ICW4: modo 8086
  outb(PIC1_PORT + 1, 0x01);
  outb(PIC2_PORT + 1, 0x01);

  // Opcional: enmascarar todas las interrupciones por ahora
  outb(PIC1_PORT + 1, 0xFF);
  outb(PIC2_PORT + 1, 0xFF);
}


void pic_enable() {
  outb(PIC1_PORT + 1, 0x00);
  outb(PIC2_PORT + 1, 0x00);
}

void pic_disable() {
  outb(PIC1_PORT + 1, 0xFF);
  outb(PIC2_PORT + 1, 0xFF);
}
