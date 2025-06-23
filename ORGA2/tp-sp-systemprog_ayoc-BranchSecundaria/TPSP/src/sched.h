/* ** por compatibilidad se omiten tildes **
================================================================================
 TALLER System Programming - ORGANIZACION DE COMPUTADOR II - FCEN
================================================================================

  Declaracion de funciones del scheduler.
*/

/* El scheduler es un arreglo de tamaño 4 (modificable) que alberga "sched_entry_t", estructura que contiene el 
selector de segmento donde esta la tss de dicha tarea en la GDT y el estado de la misma (slot libre, pausada o ejecutable).
Cumple con las politicas de un controlador "round robin", osea que cada que se solicite ejecutar otra tarea, va a 
recorrer el arreglo de tareas, comenzando desde la ultima ejecutada, hasta encontrar una ejecutable. Si no hay ninguna,
ejecuta la tarea idle hasta que ocurra una interrupcion que la corte.*/

#ifndef __SCHED_H__
#define __SCHED_H__

#include "types.h"
#include "task_defines.h"

extern int8_t current_task;

int8_t sched_add_task(uint16_t selector);
void sched_disable_task(int8_t task_id);
void sched_enable_task(int8_t task_id);

void sched_init();

uint16_t sched_next_task();
#endif //  __SCHED_H__
