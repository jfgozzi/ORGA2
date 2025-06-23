; ** por compatibilidad se omiten tildes **
; ==============================================================================
; TALLER System Programming - Arquitectura y Organizacion de Computadoras - FCEN
; ==============================================================================

%include "print.mac"

global start

extern A20_enable
extern GDT_DESC ; Descriptor de la GDT, con el tamaño (16 bits) y direccion base (32 bits). Definido en 'gdt.c'. 
extern IDT_DESC ; Descriptor de la IDT, con el tamaño (16 bits) y direccion base (32 bits). Definido en 'idt.c'.
extern idt_init ; Funcion que inicializa la idt con las entradas 0-31, 32-33, 88 y 98. Definida en 'idt.c'.
extern tss_init ; Funcion que agrega las tss de la tarea inicial y la idle en la GDT. Definida en 'tss.c'.
extern mmu_init_kernel_dir ; Funcion que inicializa el page-directory para el kernel. Definida en 'mmu.c'.
extern mmu_init_task_dir ; Funcion que inicializa el page-directory para la tarea escrita en la direccion dada como parametro. Definida en 'mmu.c'.
extern sched_init ; Funcion que inicializa el scheduler, seteando todas las posiciones con estado "libre". 'Definida en sched.c'
extern tasks_init ; Inicializa el sistema de manejo de tareas. Definida en 'tasks.c'.
extern screen_draw_layout ; Funcion que dibuja la primer pantalla con los nombres del grupo. Definida en 'screen.c'.
extern print_text_pm ; Funcion que dibuja los primeros mensajes. Definida en 'print.mac'.
extern tasks_screen_draw ; Funcion que dibuja las pantallitas en el qemu. Definida en 'tasks.c'.
extern pic_reset ; Funcion que remapea las direcciones del PIC a las que corresponden. Definida en 'pic.c'.
extern pic_enable ; Funcion que habilita el PIC. Definida en 'pic.c'.


%define CS_RING_0_SEL 0x08  ; GDT_IDX_CODE_0 << 3 | 0x0. Selector de segmento del segmento del segmento de codigo de nivel 0
%define DS_RING_0_SEL 0x18  ; GDT_IDX_DATA_0 << 3 | 0x0. Selector de segmento del segmento del segmento de datos de nivel 0
%define C_FG_BLACK   (0x0)
%define C_FG_GREEN   (0x2)
%define C_FG_WHITE   (0xF)
%define GDT_IDX_VIDEO  5 ; Indice del segmento de video en la GDT.
%define GDT_IDX_TASK_INITIAL 11 ; Indice de la tss de la tarea inicial en la GDT.
%define GDT_IDX_TASK_IDLE    12 ; Indice de la tss de la tarea idle en la GDT.
%define GDT_VIDEO_SEL (GDT_IDX_VIDEO << 3 | 0x0) ; Selector de segmento del segmento de video.
%define GDT_TASK_INITIAL_SEL (GDT_IDX_TASK_INITIAL << 3) ; Selector de segmento de la tarea inicial.
%define GDT_TASK_IDLE_SEL (GDT_IDX_TASK_IDLE << 3) ; Selector de segmento del segmento de la tarea idle.
%define DIVISOR 0x1000

BITS 16

jmp start


; -------- SECCION DE DATOS -------- ;
start_rm_msg db     'Iniciando kernel en Modo Real'
start_rm_len equ    $ - start_rm_msg

start_pm_msg db     'Iniciando kernel en Modo Protegido'
start_pm_len equ    $ - start_pm_msg


; -------- SECCION DE CODIGO -------- ;
BITS 16
start:

    cli ; Desactiva las interrupciones.

  ; cambiar modo de video a 80 x 50
    mov ax, 0003h
    int 10h ; set mode 03h
    xor bx, bx
    mov ax, 1112h
    int 10h ; load 8x8 font

    mov ax, cs
    mov ds, ax

    print_text_rm start_rm_msg, start_rm_len, C_FG_WHITE | C_FG_GREEN, 10, 10 ; Imprime mensaje de bienvenida.
    
    call A20_enable ; Habilita el A20.

    lgdt [GDT_DESC] ; carga el descriptor de la GDT en el registro GDTR.

  ; Setear el uno el bit "Protection Enable" del registro CR0.
    mov eax, cr0       
    or eax, 0x1         
    mov cr0, eax        

    jmp CS_RING_0_SEL:modo_protegido ; jump far al modo protegido.


;; --------------------------------- MODO PROTEGIDO --------------------------------- ;;

BITS 32
modo_protegido:
 
    mov ax, DS_RING_0_SEL
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

  ; Establece base y tope de la pila
    mov esp, 0x25000
    mov ebp, esp

    print_text_pm start_pm_msg, start_pm_len, C_FG_WHITE | C_FG_GREEN, 10, 10 ; Imprime mensaje de entrada a modo protegido.
 
    call screen_draw_layout ; Imprime nombres de los integrantes del grupo.


;; --------------------------------- INTERRUPCIONES --------------------------------- ;;
interrupciones:

    call idt_init ; Inicializa la idt.
    lea eax, [IDT_DESC] ; Carga el descriptor de la IDT 
    lidt [eax] ; en el LDTR.

    call pic_reset ; Remapea las direcciones del PIC.
    call pic_enable ; Habilita el PIC.

 ; Cambia el divisor del PIC.
    mov al, 0x36         
    out 0x43, al
    mov ax, DIVISOR
    out 0x40, al          
    rol ax, 8
    out 0x40, al        
    
;; --------------------------------- PAGINACION --------------------------------- ;;
paginacion:

    call mmu_init_kernel_dir ; Inicializa el page-directory del kernel. Con una unica entrada (page-table) mapeada con 
                             ; identity-mapping, el resto no mapeadas.

    mov cr3, eax ; En eax queda la direccion fisica del page-directory creado antes.
 
  ; Setea en 1 el bit 31 (mas significativo), que es el bit PG.
    mov eax, cr0
    or  eax, 0x80000000  
    mov cr0, eax


;; --------------------------------- TAREAS --------------------------------- ;;
tareas:

    call tss_init ; Carga la GDT con los descriptores de las tss de las tareas inicial y idle.

    call sched_init ; Inicializa el scheduler, poniendo el estado de los espacios de las tareas en "espacio libre".

    call tasks_init ; Inicializa el sistema de manejo de tareas (task A y task B).

  ; Carga el selector de segmento de la tarea inicial en el registro TR.
    mov ax, GDT_TASK_INITIAL_SEL 
    ltr ax

    jmp GDT_TASK_IDLE_SEL:0 ; jump far a la tarea idle.

  ; Ciclo infinito.
    mov eax, 0xFFFF
    mov ebx, 0xFFFF
    mov ecx, 0xFFFF
    mov edx, 0xFFFF
    jmp $


%include "a20.asm"

