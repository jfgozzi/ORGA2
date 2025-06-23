/* ** por compatibilidad se omiten tildes **
==============================================================================
TALLER System Programming - Arquitectura y Organizacion de Computadoras - FCEN
==============================================================================

  Definicion de la tabla de descriptores globales
*/

#include "gdt.h"

/* Aca se inicializa un arreglo de forma estatica
GDT_COUNT es la cantidad de líneas de la GDT y esta definido en defines.h */

gdt_entry_t gdt[GDT_COUNT] = {
    /* Descriptor nulo*/
    /* Offset = 0x00 */
    [GDT_IDX_NULL_DESC] =
        {
            /* El descriptor nulo es el primero que debemos definir siempre.
             Cada campo del struct se matchea con el formato que figura en el manual de intel
             Es una entrada en la GDT. */
            .limit_15_0 = 0x0000,
            .base_15_0 = 0x0000,
            .base_23_16 = 0x00,
            .type = 0x0,
            .s = 0x00,
            .dpl = 0x00,
            .p = 0x00,
            .limit_19_16 = 0x00,
            .avl = 0x0,
            .l = 0x0,
            .db = 0x0,
            .g = 0x00,
            .base_31_24 = 0x00,
        },
        /* Segmento para código nivel 0 (ejecución/lectura) */
    [GDT_IDX_CODE_0] =
        {
            .limit_15_0 = GDT_LIMIT_LOW(GDT_LIMIT_4KIB(FLAT_SEGM_SIZE)),  // bits 15-0 del limite del segmento.
            .base_15_0 = GDT_BASE_LOW(0x00000000),  // bits 15-0 de la direccion base del segmento en memoria.
            .base_23_16 = GDT_BASE_MID(0x00000000),  // bits 23-16 de la direccion base del segmento en memoria.
            .type = DESC_TYPE_EXECUTE_READ,  // tipo de segmento (datos-lectura/escritura, codigo-ejecucion/lectura).
            .s = DESC_CODE_DATA,  // tipo de descriptor de segmento (1 para codigo/datos, 0 para sistema).
            .dpl = 0x00,   // nivel de privilegio.
            .p = 0x1,      // bit de presencia.
            .limit_19_16 = GDT_LIMIT_HIGH(GDT_LIMIT_4KIB(FLAT_SEGM_SIZE)), // bits 19-16 del limite del segmento.
            .avl = 0x0,    
            .l = 0x0,      // 0 para segmentos de 32 bits.
            .db = 0x1,     // D/B = 1, segmento de 32 bits.
            .g = 0x1,      // granularidad 4KB.
            .base_31_24 = GDT_BASE_HIGH(0x00000000),  // bits 31-24 de la direccion base del segmento en memoria.
        },

    /* Segmento para código nivel 3 (ejecución/lectura) */
    [GDT_IDX_CODE_3] =
        {
            .limit_15_0 = GDT_LIMIT_LOW(GDT_LIMIT_4KIB(FLAT_SEGM_SIZE)),
            .base_15_0 = GDT_BASE_LOW(0x00000000),
            .base_23_16 = GDT_BASE_MID(0x00000000),
            .type = DESC_TYPE_EXECUTE_READ,
            .s = DESC_CODE_DATA,
            .dpl = 0x3,   // Nivel de privilegio 3
            .p = 0x1,     // Presente
            .limit_19_16 = GDT_LIMIT_HIGH(GDT_LIMIT_4KIB(FLAT_SEGM_SIZE)),
            .avl = 0x0,
            .l = 0x0,     // Segmento de 32 bits
            .db = 0x1,    // D/B = 1, segmento de 32 bits
            .g = 0x1,     // Granularidad 4KB
            .base_31_24 = GDT_BASE_HIGH(0x00000000),
        },

    /* Segmento para datos nivel 0 (lectura/escritura) */
    [GDT_IDX_DATA_0] =
        {
            .limit_15_0 = GDT_LIMIT_LOW(GDT_LIMIT_4KIB(FLAT_SEGM_SIZE)),
            .base_15_0 = GDT_BASE_LOW(0x00000000),
            .base_23_16 = GDT_BASE_MID(0x00000000),
            .type = DESC_TYPE_READ_WRITE,
            .s = DESC_CODE_DATA,
            .dpl = 0x00,  // Nivel de privilegio 0
            .p = 0x1,     // Presente
            .limit_19_16 = GDT_LIMIT_HIGH(GDT_LIMIT_4KIB(FLAT_SEGM_SIZE)),
            .avl = 0x0,
            .l = 0x0,     // Segmento de 32 bits
            .db = 0x1,    // D/B = 1, segmento de 32 bits
            .g = 0x1,     // Granularidad 4KB
            .base_31_24 = GDT_BASE_HIGH(0x00000000),
        },

    /* Segmento para datos nivel 3 (lectura/escritura) */
    [GDT_IDX_DATA_3] =
        {
            .limit_15_0 = GDT_LIMIT_LOW(GDT_LIMIT_4KIB(FLAT_SEGM_SIZE)),
            .base_15_0 = GDT_BASE_LOW(0x00000000),
            .base_23_16 = GDT_BASE_MID(0x00000000),
            .type = DESC_TYPE_READ_WRITE,
            .s = DESC_CODE_DATA,
            .dpl = 0x3,   // Nivel de privilegio 3
            .p = 0x1,     // Presente
            .limit_19_16 = GDT_LIMIT_HIGH(GDT_LIMIT_4KIB(FLAT_SEGM_SIZE)),
            .avl = 0x0,
            .l = 0x0,     // Segmento de 32 bits
            .db = 0x1,    // D/B = 1, segmento de 32 bits
            .g = 0x1,     // Granularidad 4KB
            .base_31_24 = GDT_BASE_HIGH(0x00000000),
        },

    [GDT_IDX_VIDEO] = {
            .limit_15_0 = GDT_LIMIT_LOW(GDT_LIMIT_BYTES(VIDEO_SEGM_SIZE)),
            .base_15_0 = GDT_BASE_LOW(VIDEO),
            .base_23_16 = GDT_BASE_MID(VIDEO),
            .type = 0x2,      // Segmento de datos, lectura/escritura
            .s = 1,           // Descriptor de código/datos
            .dpl = 0,         // Nivel de privilegio 0
            .p = 1,           // Presente
            .limit_19_16 = GDT_LIMIT_HIGH(GDT_LIMIT_BYTES(VIDEO_SEGM_SIZE)),
            .avl = 0,         // Reservado
            .l = 0,           // Segmento de 32 bits
            .db = 1,          // Segmento de 32 bits
            .g = 0,           // Granularidad en bytes
            .base_31_24 = GDT_BASE_HIGH(VIDEO),
    },    
};

// Instanciacion de un descriptor de la GDT, con su tamaño (16 bits) y su direccion base (32 bits). 
// Esto luego se carga en el kernel al 'gdtr'.
gdt_descriptor_t GDT_DESC = {sizeof(gdt) - 1, (uint32_t)&gdt};
