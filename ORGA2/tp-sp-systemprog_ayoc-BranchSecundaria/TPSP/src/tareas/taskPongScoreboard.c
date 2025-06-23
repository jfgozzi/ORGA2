#include "task_lib.h"

#define WIDTH TASK_VIEWPORT_WIDTH
#define HEIGHT TASK_VIEWPORT_HEIGHT

#define SHARED_SCORE_BASE_VADDR (PAGE_ON_DEMAND_BASE_VADDR + 0xF00)
#define CANT_PONGS 3

void task(void) {
	screen pantalla;

	while (true) {
		// Limpiar pantalla
		task_draw_box(pantalla, 0, 0, WIDTH, HEIGHT, ' ', C_BG_BLACK);

		// Leer e imprimir los puntajes de cada Pong
		for (int i = 0; i < CANT_PONGS; i++) {
			uint32_t* score_ptr = (uint32_t*)(SHARED_SCORE_BASE_VADDR + i * 2 * sizeof(uint32_t));
			uint32_t score1 = score_ptr[0];
			uint32_t score2 = score_ptr[1];

			// Mostrar los scores
			char label[16];
			int y = 2 + i * 3;

			// Mostrar título
			task_print(pantalla, "Pong", 2, y, C_FG_WHITE);
			task_print_dec(pantalla, i, 1, 7, y, C_FG_WHITE);

			// Mostrar score del player 1
			task_print(pantalla, "P1:", 2, y + 1, C_FG_CYAN);
			task_print_dec(pantalla, score1, 2, 6, y + 1, C_FG_CYAN);

			// Mostrar score del player 2
			task_print(pantalla, "P2:", 12, y + 1, C_FG_MAGENTA);
			task_print_dec(pantalla, score2, 2, 16, y + 1, C_FG_MAGENTA);
		}

		// Dibujar en pantalla
		syscall_draw(pantalla);

		// Esperar un poco para no actualizar tan rápido
		task_sleep(50);
	}
}

