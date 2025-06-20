; ** por compatibilidad se omiten tildes **
; ==============================================================================
; TALLER System Programming - Arquitectura y Organizacion de Computadoras - FCEN
; ==============================================================================

%include "print.mac"

global start


; COMPLETAR - Agreguen declaraciones extern según vayan necesitando
extern GDT_DESC
extern gdt
extern A20_enable
extern _start
extern screen_draw_layout
extern print_text_pm
extern idt_init
extern IDT_DESC
extern pic_reset
extern pic_enable
extern mmu_init_kernel_dir
extern mmu_init_task_dir
extern rcr3
extern tss_init
extern tasks_screen_draw
extern sched_init
extern tasks_init







; COMPLETAR - Definan correctamente estas constantes cuando las necesiten
%define CS_RING_0_SEL 0x08  ; GDT_IDX_CODE_0 << 3 | 0x0
%define DS_RING_0_SEL 0x18  ; GDT_IDX_DATA_0 << 3 | 0x0
%define C_FG_BLACK         (0x0)
%define C_FG_GREEN         (0x2)
%define C_FG_WHITE         (0xF)
%define GDT_IDX_VIDEO  5
%define GDT_VIDEO_SEL (GDT_IDX_VIDEO << 3 | 0x0)
%define GDT_IDX_TASK_INITIAL 11
%define GDT_TASK_INITIAL_SEL (GDT_IDX_TASK_INITIAL << 3)
%define GDT_IDX_TASK_IDLE    12
%define GDT_TASK_IDLE_SEL    (GDT_IDX_TASK_IDLE << 3)
%define DIVISOR 10   ; Ajustá el divisor como desees

BITS 16
;; Saltear seccion de datos
jmp start

;;
;; Seccion de datos.
;; -------------------------------------------------------------------------- ;;
start_rm_msg db     'Iniciando kernel en Modo Real'
start_rm_len equ    $ - start_rm_msg

start_pm_msg db     'Iniciando kernel en Modo Protegido'
start_pm_len equ    $ - start_pm_msg

;;
;; Seccion de código.
;; -------------------------------------------------------------------------- ;;

;; Punto de entrada del kernel.
BITS 16
start:
    ; COMPLETAR - Deshabilitar interrupciones
    cli

    ; Cambiar modo de video a 80 X 50
    mov ax, 0003h
    int 10h ; set mode 03h
    xor bx, bx
    mov ax, 1112h
    int 10h ; load 8x8 font


    
    ; COMPLETAR - Imprimir mensaje de bienvenida - MODO REAL
    ; (revisar las funciones definidas en print.mac y los mensajes se encuentran en la
    ; sección de datos)
    mov ax, cs
    mov ds, ax

breakpoint_mensaje_modo_real:
    print_text_rm start_rm_msg, start_rm_len, C_FG_WHITE | C_FG_GREEN, 10, 10
    
    
    

    ; ; COMPLETAR - Habilitar A20
    ; ; (revisar las funciones definidas en a20.asm)
    call A20_enable


    ; COMPLETAR - Cargar la GDT
    lgdt [GDT_DESC]

    

    

    ; COMPLETAR - Setear el bit PE del registro CR0
    mov eax, cr0        ; Cargar CR0 en EAX
    or eax, 0x1         ; Setear el bit PE (Protection Enable)
    mov cr0, eax        ; Escribir de vuelta en CR0

    ; COMPLETAR - Saltar a modo protegido (far jump)
    ; (recuerden que un far jmp se especifica como jmp CS_selector:address)
    ; Pueden usar la constante CS_RING_0_SEL definida en este archivo
    jmp CS_RING_0_SEL:modo_protegido
;; -------------------------------------------------------------------------- ;;

BITS 32
modo_protegido:
    ; COMPLETAR - A partir de aca, todo el codigo se va a ejectutar en modo protegido
    ; Establecer selectores de segmentos DS, ES, GS, FS y SS en el segmento de datos de nivel 0
    ; Pueden usar la constante DS_RING_0_SEL definida en este archivo
    ; Establecer selectores de segmentos DS, ES, FS, GS y SS al de datos de nivel 0
    mov ax, DS_RING_0_SEL
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    ; COMPLETAR - Establecer el tope y la base de la pila
    mov esp, 0x25000
    mov ebp, esp
breakpoint_mensaje_modo_protegido:
    ; COMPLETAR - Imprimir mensaje de bienvenida - MODO PROTEGIDO
    print_text_pm start_pm_msg, start_pm_len, C_FG_WHITE | C_FG_GREEN, 10, 10
    
breakpoint_integrantes:
    ; COMPLETAR - Inicializar pantalla
    call screen_draw_layout
    
    ; ------------------- MMU: Habilitar paginación -------------------
;breakpoint_paginacion:

    call mmu_init_kernel_dir

    mov cr3, eax
 
    mov eax, cr0
    or  eax, 0x80000000  
    mov cr0, eax

    call tss_init

    mov ax, GDT_TASK_INITIAL_SEL  
    ltr ax
    
breakpoint_IDT:

    call idt_init
    lea eax, [IDT_DESC]
    lidt [eax]

    call pic_reset
    call pic_enable

    ; mov al, 0x36         
    ; out 0x43, al

   mov ax, DIVISOR
    out 0x40, al          
    rol ax, 8
    out 0x40, al        

    ;sti

    
    ;int 33

    ;int 88


;breakpoint_tareas:

    call tss_init

    ;call tasks_screen_draw

    call sched_init

    call tasks_init


    mov ax, GDT_TASK_INITIAL_SEL  
    ltr ax

;breakpoint_idle:
    ;jmp far [tss_idle_ptr]
    jmp GDT_TASK_IDLE_SEL:0



    ; Ciclo infinito
    mov eax, 0xFFFF
    mov ebx, 0xFFFF
    mov ecx, 0xFFFF
    mov edx, 0xFFFF
    jmp $

;; -------------------------------------------------------------------------- ;;

%include "a20.asm"
tss_initial_ptr:
    dd 0x00000000                ; offset (ignorado, pero necesario)
    dw GDT_TASK_INITIAL_SEL      ; selector de la TSS

tss_idle_ptr:
    dd 0x00000000         ; offset (ignorado)
    dw GDT_TASK_IDLE_SEL  ; selector de la TSS Idle

