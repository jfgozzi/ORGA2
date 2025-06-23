/* ** por compatibilidad se omiten tildes **
================================================================================
 TRABAJO PRACTICO 3 - System Programming - ORGANIZACION DE COMPUTADOR II - FCEN
================================================================================

  Definicion de funciones del manejador de memoria
*/

#include "mmu.h"
#include "i386.h"
#include "sched.h"
#include "kassert.h"

// Puntero a direccion fisica del page-directory del kernel.
static pd_entry_t* kpd = (pd_entry_t*)KERNEL_PAGE_DIR;

// Puntero a direccion fisica de la primer entrada del kernel.
static pt_entry_t* kpt = (pt_entry_t*)KERNEL_PAGE_TABLE_0;

// Constante que marca el valor tope de direccion fisica para el mapeo del kernel.
static const uint32_t identity_mapping_end = 0x003FFFFF;
static const uint32_t user_memory_pool_end = 0x02FFFFFF;

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
    kassert(page < identity_mapping_end, "El kernel no tiene mas memoria disponible.");
    next_free_kernel_page += PAGE_SIZE;  // avanzar una página (4KB)
    return page;
}

/**
 * mmu_next_free_user_page devuelve la dirección de la próxima página de usuarix disponible
 * @return devuelve la dirección de memoria de comienzo de la próxima página libre de usuarix
 */
paddr_t mmu_next_free_user_page(void) {
    paddr_t page = next_free_user_page;
    kassert(page < user_memory_pool_end, "No hay mas espacio disponible.");
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

    zero_page((paddr_t)kpd);  
    zero_page((paddr_t)kpt);      
    
    for (int i = 0; i < 1024; i++) {
        kpt[i].page = i;  
        kpt[i].attrs = 0x3;
    }

    kpd[0].pt = ((paddr_t)kpt) >> 12;
    kpd[0].attrs = 0x3;  

     //Resto de las entradas quedan en 0 (no mapeadas)
    for (int i = 1; i < 1024; i++) {
        kpd[i].pt = 0;
        kpd[i].attrs = 0;
    }

    return (paddr_t)kpd;
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
    pd_entry_t* pd = (pd_entry_t*)(cr3 & 0xFFFFF000);
    uint32_t pd_idx = (virt >> 22) & 0x3FF;
    uint32_t pt_idx = (virt >> 12) & 0x3FF;
    
    if(!(pd[pd_idx].pt & 0x1)){
        paddr_t nueva_dir_fisica_pt = mmu_next_free_kernel_page();
        zero_page(nueva_dir_fisica_pt);
        pt_entry_t* nueva_pt = (pt_entry_t*)nueva_dir_fisica_pt;
        pd[pd_idx].pt = nueva_dir_fisica_pt >> 12;
        pd[pd_idx].attrs = attrs | 0x1;
    }

    pt_entry_t* pt = (pt_entry_t*)((pd[pd_idx].pt) << 12);
    pt[pt_idx].attrs = attrs | 0x1;
    pt[pt_idx].page = phy >> 12;

    tlbflush(); // Importante: forzar el refresco de la TLB
}



/**
 * mmu_unmap_page elimina la entrada vinculada a la dirección virt en la tabla de páginas correspondiente
 * @param virt la dirección virtual que se ha de desvincular
 * @return la dirección física de la página desvinculada
 */
paddr_t mmu_unmap_page(uint32_t cr3, vaddr_t virt) {
    pd_entry_t* pd = (pd_entry_t*)(cr3 & 0xFFFFF000);
    uint32_t pd_idx = (virt >> 22) & 0x3FF;
    uint32_t pt_idx = (virt >> 12) & 0x3FF;
    
    if (!(pd[pd_idx].attrs & 0x1)) {
        return 0; // No hay tabla presente
    }

    pt_entry_t* pt = (pt_entry_t*)(pd[pd_idx].pt << 12);

    if(!(pt[pt_idx].attrs & 0x1)){
        return 0;
    }

    paddr_t phy = pt[pt_idx].page << 12;
    pt[pt_idx].attrs = 0;
    pt[pt_idx].page = 0;

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
    paddr_t pd_fisica = mmu_next_free_kernel_page();
    paddr_t pt_fisica = mmu_next_free_kernel_page();
    paddr_t stack_fisica = mmu_next_free_user_page();

    zero_page(pd_fisica);
    zero_page(pt_fisica);

    pd_entry_t* pd = (pd_entry_t*) pd_fisica;
    pt_entry_t* pt = (pt_entry_t*) pt_fisica;

    // Identity mapping nivel 0 (0x00000000 - 0x003FFFFF)
    paddr_t pt_identity = mmu_next_free_kernel_page();
    zero_page(pt_identity);
    pt_entry_t* pt0 = (pt_entry_t*) pt_identity;
    for (int i = 0; i < 1024; i++) {
        pt0[i].page = i;
        pt0[i].attrs = 0x3;  // RW + P
    }
    pd[0].pt = ((paddr_t)pt0) >> 12;
    pd[0].attrs = 0x3;  // RW + P

    // Mapeo de código, stack y página compartida
    uint32_t idx_pd_user = 0x08000000 >> 22;
    pd[idx_pd_user].pt = ((paddr_t)pt) >> 12;
    pd[idx_pd_user].attrs = 0x7;  // RW + User + Present

    // Entrada 0: Código página 1
    pt[0].page = phy_start >> 12;
    pt[0].attrs = 0x5;  // RO + User + Present

    // Entrada 1: Código página 2
    pt[1].page = (phy_start + PAGE_SIZE) >> 12;
    pt[1].attrs = 0x5;

    // Entrada 2: Stack (en 0x08002000)
    pt[2].page = stack_fisica >> 12;
    pt[2].attrs = 0x7;  // RW + User + Present

    // Entrada 3: Página compartida (en 0x08003000)
    pt[3].page = SHARED >> 12;
    pt[3].attrs = 0x5;  // RO + User + Present

    return pd_fisica;
}


// Devuelve true si se atendió el page fault y puede continuar la ejecución 
// y false si no se pudo atender
bool page_fault_handler(vaddr_t virt) {
        print("PF!", 20, 0, C_FG_WHITE | C_BG_RED);
    vaddr_t page_base = virt & ~(PAGE_SIZE - 1);

    // Caso 1: Memoria on-demand compartida
    if (page_base >= ON_DEMAND_MEM_START_VIRTUAL && page_base <= ON_DEMAND_MEM_END_VIRTUAL) {
        mmu_map_page(rcr3(), page_base, ON_DEMAND_MEM_START_PHYSICAL, 0x7);
        return true;
    }

    // Caso 2: Código de tarea
    if (page_base >= TASK_CODE_VIRTUAL && page_base < TASK_CODE_VIRTUAL + TASK_CODE_PAGES * PAGE_SIZE) {
        print("Mapeando código", 0, 1, C_FG_WHITE | C_BG_BLUE);
        paddr_t phys = 0x100000 + current_task * TASK_CODE_PAGES * PAGE_SIZE + (page_base - TASK_CODE_VIRTUAL);
        mmu_map_page(rcr3(), page_base, phys, 0x5);
        return true;
    }

    // Caso 3: Página compartida (stack)
    if (page_base == TASK_SHARED_PAGE) {
        mmu_map_page(rcr3(), page_base, SHARED, 0x7);
        return true;
    }

    return false;
}
