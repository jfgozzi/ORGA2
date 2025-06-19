
; ==============================================================================
; TALLER System Programming - ORGANIZACION DE COMPUTADOR II - FCEN
; ==============================================================================

extern task

BITS 32
_start:
	call task
	jmp $



; AGREGAR A RESPUESTAS.TXT
; a. ¿Por qué la tarea termina en un loop infinito?

; Porque si no se queda en un bucle, el programa no tiene a dónde volver cuando la tarea termina. 
; No hay un "exit", así que si volviera podría crashear el sistema.
; El jmp $ asegura que la tarea se quede ahí quieta y no cause problemas.

; b. ¿Qué podríamos hacer para que esto no sea necesario?

; Podríamos agregar alguna syscall o una manera para hacer termine la ejecucion.
; Eso permitiría que el kernel la saque de la lista de tareas activas y libere sus recursos.
; También podríamos ajustar el scheduler para que detecte tareas muertas y no las ejecute más.
; Pero como no tenemos nada de eso, el loop infinito es la solución fácil y segura