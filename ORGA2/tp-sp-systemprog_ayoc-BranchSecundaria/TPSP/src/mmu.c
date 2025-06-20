/* ** por compatibilidad se omiten tildes **
================================================================================
 TRABAJO PRACTICO 3 - System Programming - ORGANIZACION DE COMPUTADOR II - FCEN
================================================================================

  Definicion de funciones del manejador de memoria
*/

#include "mmu.h"
#include "i386.h"

#include "kassert.h"

// static pd_entry_t* kpd = (pd_entry_t*)KERNEL_PAGE_DIR;
// static pt_entry_t* kpt = (pt_entry_t*)KERNEL_PAGE_TABLE_0;

// static const uint32_t identity_mapping_end = 0x003FFFFF;
// static const uint32_t user_memory_pool_end = 0x02FFFFFF;

static paddr_t next_free_kernel_page = 0x100000;
static paddr_t next_free_user_page = 0x400000;

/**
 * kmemset asigna el valor c a un rango de memoria interpretado
 * como un rango de bytes de largo n que comienza en s
 * @param s es el puntero al comienzo del rango de memoria
 * @param c es el valor a asignar en cada byte de s[0..n-1]
 * @param n es el tamaño en bytes a asignar
 * @return devuelve el puntero al rango modificado (alias de s)
*/
static inline void* kmemset(void* s, int c, size_t n) {
  uint8_t* dst = (uint8_t*)s;
  for (size_t i = 0; i < n; i++) {
    dst[i] = c;
  }
  return dst;
}

/**
 * kmemcpy copia un bloque de memoria de src a dst, de tamaño n bytes.
 * @param dst puntero a la memoria destino
 * @param src puntero a la memoria origen
 * @param n cantidad de bytes a copiar
 * @return devuelve dst
 */
static inline void* kmemcpy(void* dst, const void* src, size_t n) {
    uint8_t* d = (uint8_t*) dst;
    const uint8_t* s = (const uint8_t*) src;
    for (size_t i = 0; i < n; i++) {
        d[i] = s[i];
    }
    return dst;
}


/**
 * zero_page limpia el contenido de una página que comienza en addr
 * @param addr es la dirección del comienzo de la página a limpiar
*/
static inline void zero_page(paddr_t addr) {
  kmemset((void*)addr, 0x00, PAGE_SIZE);
}


void mmu_init(void) {}


/**
 * mmu_next_free_kernel_page devuelve la dirección física de la próxima página de kernel disponible. 
 * Las páginas se obtienen en forma incremental, siendo la primera: next_free_kernel_page
 * @return devuelve la dirección de memoria de comienzo de la próxima página libre de kernel
 */
paddr_t mmu_next_free_kernel_page(void) {
    paddr_t page = next_free_kernel_page;
    next_free_kernel_page += 0x1000;  // avanzar una página (4KB)
    return page;
}

/**
 * mmu_next_free_user_page devuelve la dirección de la próxima página de usuarix disponible
 * @return devuelve la dirección de memoria de comienzo de la próxima página libre de usuarix
 */
paddr_t mmu_next_free_user_page(void) {
    paddr_t page = next_free_user_page;
    next_free_user_page += 0x1000;
    return page;
}


/**
 * mmu_init_kernel_dir inicializa las estructuras de paginación vinculadas al kernel y
 * realiza el identity mapping
 * @return devuelve la dirección de memoria de la página donde se encuentra el directorio
 * de páginas usado por el kernel
 */
paddr_t mmu_init_kernel_dir(void) {
    // Dirección fija para el Page Directory del kernel
    paddr_t page_directory = 0x00025000;

    // Primera tabla de páginas, para los primeros 4MB
    paddr_t page_table = 0x00026000;

    zero_page(page_directory);  // 🔧 limpiar memoria del directorio
    zero_page(page_table);      // 🔧 limpiar memoria de la tabla

    uint32_t* pd = (uint32_t*) page_directory;
    uint32_t* pt = (uint32_t*) page_table;

    // Completar Page Table con identity mapping
    for (int i = 0; i < 1024; i++) {
        pt[i] = (i * 0x1000) | 0x3;  // Present + RW
    }

    // Completar Page Directory apuntando a la tabla anterior
    pd[0] = page_table | 0x3;  // Present + RW

    // Resto de las entradas quedan en 0 (no mapeadas)
    for (int i = 1; i < 1024; i++) {
        pd[i] = 0x00000000;
    }

    return page_directory;
}


/**
 * mmu_map_page agrega las entradas necesarias a las estructuras de paginación de modo de que
 * la dirección virtual virt se traduzca en la dirección física phy con los atributos definidos en attrs
 * @param cr3 el contenido que se ha de cargar en un registro CR3 al realizar la traducción
 * @param virt la dirección virtual que se ha de traducir en phy
 * @param phy la dirección física que debe ser accedida (dirección de destino)
 * @param attrs los atributos a asignar en la entrada de la tabla de páginas
 */
void mmu_map_page(uint32_t cr3, vaddr_t virt, paddr_t phy, uint32_t attrs) {
    uint32_t* pd = (uint32_t*)(cr3 & ~0xFFF);
    uint32_t pd_idx = (virt >> 22) & 0x3FF;
    uint32_t pt_idx = (virt >> 12) & 0x3FF;
    

    // Si no hay tabla presente, reservar una
    if ((pd[pd_idx] & 0x1) == 0) {
        paddr_t new_pt = mmu_next_free_kernel_page();
        zero_page(new_pt);
        pd[pd_idx] = (new_pt & ~0xFFF) | 0x3; // Present + RW
    }

    // Obtener la base de la tabla de páginas
    uint32_t* pt = (uint32_t*)(pd[pd_idx] & ~0xFFF);
    pt[pt_idx] = (phy & ~0xFFF) | (attrs & 0xFFF);

    tlbflush(); // Importante: forzar el refresco de la TLB
}



/**
 * mmu_unmap_page elimina la entrada vinculada a la dirección virt en la tabla de páginas correspondiente
 * @param virt la dirección virtual que se ha de desvincular
 * @return la dirección física de la página desvinculada
 */
paddr_t mmu_unmap_page(uint32_t cr3, vaddr_t virt) {
    uint32_t* pd = (uint32_t*)(cr3 & ~0xFFF);
    uint32_t pd_idx = (virt >> 22) & 0x3FF;
    uint32_t pt_idx = (virt >> 12) & 0x3FF;
    

    if (!(pd[pd_idx] & 0x1)) {
        return 0; // No hay tabla presente
    }

    uint32_t* pt = (uint32_t*)(pd[pd_idx] & ~0xFFF);
    paddr_t phy = pt[pt_idx] & ~0xFFF;
    pt[pt_idx] = 0x0;

    tlbflush();
    return phy;
}


#define DST_VIRT_PAGE 0xA00000
#define SRC_VIRT_PAGE 0xB00000

/**
 * copy_page copia el contenido de la página física localizada en la dirección src_addr a la página física ubicada en dst_addr
 * @param dst_addr la dirección a cuya página queremos copiar el contenido
 * @param src_addr la dirección de la página cuyo contenido queremos copiar
 *
 * Esta función mapea ambas páginas a las direcciones SRC_VIRT_PAGE y DST_VIRT_PAGE, respectivamente, realiza
 * la copia y luego desmapea las páginas. Usar la función rcr3 definida en i386.h para obtener el cr3 actual
 */
void copy_page(paddr_t dst, paddr_t src) {
    uint32_t cr3 = rcr3();  // Obtener el page directory actual

    // Mapear páginas físicas a direcciones virtuales temporales
    mmu_map_page(cr3, SRC_VIRT_PAGE, src, 0x3);  // RW + Present
    mmu_map_page(cr3, DST_VIRT_PAGE, dst, 0x3);

    // Copiar contenido: como ambas son virtuales, simplemente usamos memcpy
    kmemcpy((void*)DST_VIRT_PAGE, (void*)SRC_VIRT_PAGE, PAGE_SIZE);

    // Desmapear páginas temporales
    mmu_unmap_page(cr3, SRC_VIRT_PAGE);
    mmu_unmap_page(cr3, DST_VIRT_PAGE);
}


 /**
 * mmu_init_task_dir inicializa las estructuras de paginación vinculadas a una tarea cuyo código se encuentra en la dirección phy_start
 * @pararm phy_start es la dirección donde comienzan las dos páginas de código de la tarea asociada a esta llamada
 * @return el contenido que se ha de cargar en un registro CR3 para la tarea asociada a esta llamada
 */
paddr_t mmu_init_task_dir(paddr_t phy_start) {
    // 1. Crear directorio de páginas
    paddr_t pd_dir = mmu_next_free_kernel_page();
    zero_page(pd_dir);
    uint32_t* pd = (uint32_t*)(pd_dir);

    // 2. Identity mapping de primeros 4MB (nivel 0, RW)
    paddr_t pt_identity = mmu_next_free_kernel_page();
    zero_page(pt_identity);
    uint32_t* pt0 = (uint32_t*)pt_identity;
    for (int i = 0; i < 1024; i++) {
        pt0[i] = (i * 0x1000) | 0x3;
    }
    pd[0] = pt_identity | 0x3;

    // 3. Tabla para código, pila
    paddr_t pt_task = mmu_next_free_kernel_page();
    zero_page(pt_task);
    uint32_t* pt1 = (uint32_t*)pt_task;
    pd[0x20] = pt_task | 0x3;

    // Código: RO, User
    pt1[0x000] = (phy_start & ~0xFFF) | 0x5;
    pt1[0x001] = ((phy_start + 0x1000) & ~0xFFF) | 0x5;

    // Stack: RW, User (usar página de usuario!)
    paddr_t stack_page = mmu_next_free_user_page();
    pt1[0x002] = stack_page | 0x7;

    // 4. Tabla para memoria compartida
    paddr_t pt_shared = mmu_next_free_kernel_page();
    zero_page(pt_shared);
    uint32_t* pt3 = (uint32_t*)pt_shared;

    pd[0x1C] = pt_shared | 0x3;
    // No mapeamos la entrada aún, lo hará el page_fault_handler

    



    return pd_dir;
}


// COMPLETAR: devuelve true si se atendió el page fault y puede continuar la ejecución 
// y false si no se pudo atender
// bool page_fault_handler(vaddr_t virt) {
//     print("Atendiendo page fault...", 20, 0, C_FG_WHITE | C_BG_BLUE);

//     if (virt >= 0x07000000 && virt <= 0x07000FFF) {
//         mmu_map_page(rcr3(), virt, 0x03000000, 0x7); // RW | User | Present
//         return true;
//     }
//     return false;
// }
bool page_fault_handler(vaddr_t virt) {
    if (virt >= ON_DEMAND_MEM_START_VIRTUAL && virt <= ON_DEMAND_MEM_END_VIRTUAL) {
        mmu_map_page(rcr3(), virt, ON_DEMAND_MEM_START_PHYSICAL, MMU_P | MMU_W | MMU_U);
        return true;
    }
    return false;
}
