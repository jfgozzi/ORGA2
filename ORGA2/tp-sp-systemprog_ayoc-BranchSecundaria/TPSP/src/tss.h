/* ** por compatibilidad se omiten tildes **
================================================================================
 TALLER System Programming - ORGANIZACION DE COMPUTADOR II - FCEN
================================================================================

  Declaracion de estructuras para administrar tareas
*/

#ifndef __TSS_H__
#define __TSS_H__

#include "gdt.h"
#include "types.h"
#include "task_defines.h"

typedef struct str_tss {
  uint16_t ptl; // Selector de segmento de la tss de la tarea anterior a la actual.
  uint16_t unused0; 
  uint32_t esp0; // Puntero al tope de la pila de nivel 0 de la tarea.
  uint16_t ss0; // Registro selector de segmento de la pila de nivel 0. 
  uint16_t unused1;
  uint32_t esp1;
  uint16_t ss1;
  uint16_t unused2;
  uint32_t esp2;
  uint16_t ss2;
  uint16_t unused3;
  uint32_t cr3; // El registro cr3 de la tarea, con el puntero a su page-directory. 
  uint32_t eip; // Registro instruction pointer, que marca el punto de inicio de la tarea. 
  uint32_t eflags; // Registro con las flags del contexto. En 0x00000202 o (1 << 9) habilita las interrupciones.
  uint32_t eax; // Registro de proposito general.
  uint32_t ecx; // Registro de proposito general.
  uint32_t edx; // Registro de proposito general.
  uint32_t ebx; // Registro de proposito general.
  uint32_t esp; // Registro de proposito general.
  uint32_t ebp; // Registro de proposito general.
  uint32_t esi; // Registro de proposito general.
  uint32_t edi; // Registro de proposito general.
  uint16_t es; // Registro selector de segmento.
  uint16_t unused4;
  uint16_t cs; // Registro selector de segmento.
  uint16_t unused5;
  uint16_t ss; // Registro selector de segmento.
  uint16_t unused6;
  uint16_t ds; // Registro selector de segmento.
  uint16_t unused7;
  uint16_t fs; // Registro selector de segmento.
  uint16_t unused8;
  uint16_t gs; // Registro selector de segmento.
  uint16_t unused9;
  uint16_t ldt; // Selector de segmento de LDT de la tarea. No lo usamos.
  uint16_t unused10;
  uint16_t dtrap;
  uint16_t iomap;
} __attribute__((__packed__, aligned(8))) tss_t;

extern tss_t tss_initial;
extern tss_t tss_idle;

void tss_init(void);

extern tss_t tss_tasks[MAX_TASKS];
tss_t tss_create_user_task(uint32_t cr3);
gdt_entry_t tss_gdt_entry_for_task(tss_t* tss);

#endif //  __TSS_H__
