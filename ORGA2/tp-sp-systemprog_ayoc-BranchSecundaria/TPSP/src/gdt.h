/* ** por compatibilidad se omiten tildes **
==============================================================================
TALLER System Programming - Arquitectura y Organizacion de Computadoras - FCEN
==============================================================================

  Declaracion de la tabla de descriptores globales
*/

#ifndef __GDT_H__
#define __GDT_H__

#include "defines.h"
#include "types.h"

// Define el descriptor de la GDT
// Es una estructura de 48 bits con dos elementos
typedef struct str_gdt_descriptor {
  uint16_t gdt_length;
  uint32_t gdt_addr;
} __attribute__((__packed__)) gdt_descriptor_t;

// Entrada de la GDT. Es una estructura
// Los campos con dos puntos (:) por ejemplo, uint8_t s : 1; significa que usa solo un bit de los 8 definidos en el tipo uint8_t
// Asi tambien uint8_t type : 4; significa que usa 4 bits menos significativos del tipo.
// De esta manera, se pueden definir campos que no sean múltiplos del tamaño de un byte

typedef struct str_gdt_entry {
  uint16_t limit_15_0;  // bits 15-0 del tamaño limite en bytes del segmento.
  uint16_t base_15_0;  // bits 15-0 de la direccion base del segmento en memoria.
  uint8_t base_23_16;  // bits 23-16 de la direccion base del segmento en memoria.
  uint8_t type : 4;  // bits que marcan si el segmento es de codigo-ejecucion/lectura o datos-lectura/escritura.
  uint8_t s : 1;  // bit que marca el tipo de descriptor, de sistema(0) o de codigo-datos(1).
  uint8_t dpl : 2;  // bits que marcan el nivel de privilegio (0-3) del segmento.
  uint8_t p : 1;  // bit que marca que el segmento "presente".
  uint8_t limit_19_16 : 4;  // bits 19-16 del tamaño limite en bytes del segmento.
  uint8_t avl : 1;  // bit "available for use by system sofware", se deja en 0.
  uint8_t l : 1;  // bit que marca si el segmento es de 32bits(0) o 64bits(1).
  uint8_t db : 1;  // bit que marca el tamaño de las operaciones que se van a hacer sobre el, 16bits(0) o 32bits(1).
  uint8_t g : 1;  // bit que marca en que unidad se mide el limite del segmento, bytes(0) o paginas de 4KiB(1).
  uint8_t base_31_24;  // bits 31-24 de la direccion base del segmento en memoria.
} __attribute__((__packed__, aligned(8))) gdt_entry_t;

/* Declaracion variables de la Tabla GDT */
extern gdt_entry_t gdt[];
extern gdt_descriptor_t GDT_DESC;  

#endif // __GDT_H__
