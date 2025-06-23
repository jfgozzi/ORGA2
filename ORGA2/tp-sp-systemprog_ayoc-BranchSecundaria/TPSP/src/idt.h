/* ** por compatibilidad se omiten tildes **
================================================================================
 TALLER System Programming - ORGANIZACION DE COMPUTADOR II - FCEN
================================================================================

  Declaracion de las rutinas de atencion de interrupciones
*/

#ifndef __IDT_H__
#define __IDT_H__

#include "types.h"

// bits 12-8 de un IDT gate descriptor. 0xE = 01110. marca que la entrada de la IDT es una interrupcion.
#define INTERRUPT_GATE_TYPE 0xE

/* Struct de descriptor de IDT */
typedef struct str_idt_descriptor {
  uint16_t idt_length;
  uint32_t idt_addr;
} __attribute__((__packed__)) idt_descriptor_t;

/* Struct de una entrada de la IDT */
typedef struct {
    uint16_t offset_15_0;  // bits 15-0 de la direccion base del codigo que maneja la interrucion. 
    uint16_t segsel;  // selector de segmento del segmento de codigo que tiene el codigo que maneja la interrupcion.
    uint8_t reserved : 5;  // bits reservados, no se tocan.
    uint8_t should_be_zero : 3;  // bits fijos en 0.
    uint8_t type : 5;  // bits que indican que tipo de gate es. "00101" = task gate, "01110" = interrupt gate, "0111" = trap gate.
    uint8_t dpl : 2;  // bits que indican el nivel de privilegio minimo que necesita el codigo que invoque la interrupcion.
    uint8_t present : 1;  // bit que indica la "presencia".
    uint16_t offset_31_16;  // bits 31-16 de la direccion base del codigo que maneja la interrucion.
} __attribute__((__packed__, aligned (8))) idt_entry_t;


extern idt_entry_t idt[];
extern idt_descriptor_t IDT_DESC;

void idt_init(void);

#endif //  __IDT_H__
