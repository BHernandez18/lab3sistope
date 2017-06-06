#include <curses.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

typedef struct luchador {
	char * nombreLuchador;
	int hp;
	int ki;
	int realizoAtaque?;
	int universo;
	int colorUniverso;
	int xPos;
	int yPos;
} luchador;

int ancho;
int alto;
luchador ** tableroLuchadores;
pthread_mutex_t ** tableroThreads;

void crearTableros() {
	int i;
	tableroThreads = (pthread_mutex_t **)malloc(sizeof(pthread_mutex_t *) * ancho);
	tableroLuchadores = (luchador **)malloc(sizeof(luchador *) * ancho);
	for (i = 0; i < ancho; i++) {
		tableroThreads[i] = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t) * alto);
		tableroLuchadores[i] = (luchador *)malloc(sizeof(luchador) * alto);
	}

}

void iniciarTableros() {
	int i, j;
	for (i = 0; i < ancho; i++) {
		for (j = 0; j < alto; j++) {
			if (pthread_mutex_init( &tableroThreads[i][j], NULL ) != 0) {
				fprintf(stderr, "<> ERROR: Problema al iniciar el tablero. <>\n");
				exit(1);
			}
			tableroLuchadores[i][j] = NULL;
		}
	}
}

void realizarAtaque(int ki, int universo, int xPos, int yPos) {
	int dañoTotal, tmpX, tmpY;
	// Daño total a realizar a los personajes.
	dañoTotal = ki * 5;
	// Se obtienen los posibles enemigos y se realiza el ataque.
	// --> Enemigo de arriba <--
	xPos = personaje->xPos - 1;
	yPos = personaje->yPos;
	//
	// Verificación y actualización de posiciones.
	//
	if (xPos < 0) {	xPos = xPos + 1; }
	if (yPos < 0) { yPos = yPos + 1; }
	if (xPos >= alto) { xPos = xPos - 1; }
	if (yPos >= ancho) { yPos = yPos - 1; }

	// --> Enemigo de abajo <--
	xPos = personaje->xPos + 1;
	yPos = personaje->yPos;
	//
	// Verificación y actualización de posiciones.
	//
	if (xPos < 0) {	xPos = xPos + 1; }
	if (yPos < 0) { yPos = yPos + 1; }
	if (xPos >= alto) { xPos = xPos - 1; }
	if (yPos >= ancho) { yPos = yPos - 1; }

	// --> Enemigo de la izquierda <--
	xPos = personaje->xPos;
	yPos = personaje->yPos - 1;
	//
	// Verificación y actualización de posiciones.
	//
	if (xPos < 0) {	xPos = xPos + 1; }
	if (yPos < 0) { yPos = yPos + 1; }
	if (xPos >= alto) { xPos = xPos - 1; }
	if (yPos >= ancho) { yPos = yPos - 1; }

	// --> Enemigo de la derecha <--
	xPos = personaje->xPos;
	yPos = personaje->yPos + 1;
	//
	// Verificación y actualización de posiciones.
	//
	if (xPos < 0) {	xPos = xPos + 1; }
	if (yPos < 0) { yPos = yPos + 1; }
	if (xPos >= alto) { xPos = xPos - 1; }
	if (yPos >= ancho) { yPos = yPos - 1; }

}

void * entrarArena(void * concursante) {
	int sigMovimiento, xPos, yPos, tmpX, tmpY;
	luchador * personaje = (luchador *) concursante;
	//
	// Mientras el luchador aún tenga vida.
	//
	while (personaje->hp > 0) {
		// Se obtiene el movimiento hacia donde avanzar.
		sigMovimiento = rand() % 4;
		switch (sigMovimiento) {
			// Se mueve hacia arriba.
			case 0:
				xPos = personaje->xPos - 1;
				yPos = personaje->yPos;
				break;
			// Se mueve hacia abajo.
			case 1:
				xPos = personaje->xPos + 1;
				yPos = personaje->yPos;
				break;
			// Se mueve hacia la izquierda.
			case 2:
				xPos = personaje->xPos;
				yPos = personaje->yPos - 1;
				break;
			// Se mueve hacia la derecha.
			case 3:
				xPos = personaje->xPos;
				yPos = personaje->yPos + 1;
				break;
		}
		//
		// Verificación y actualización de posiciones.
		//
		if (xPos < 0) {	xPos = xPos + 1; }
		if (yPos < 0) { yPos = yPos + 1; }
		if (xPos >= alto) { xPos = xPos - 1; }
		if (yPos >= ancho) { yPos = yPos - 1; }
		//
		// Se intenta avanzar a la posición indicada.
		//
		if (pthread_mutex_trylock(&tableroThreads[xPos][yPos]) == 0) {
			// Se obtiene temporalmente la ubicación actual del personaje.
			tmpX = personaje->xPos;
			tmpY = personaje->yPos;
			// Se ubica al personaje en las coordenadas dadas.
			tableroLuchadores[xPos][yPos] = *luchador
			// Se asigna la nueva ubicación del personaje.
			personaje->xPos = xPos;
			personaje->yPos = yPos;
			// Se libera el mutex de la ubicación anterior del personaje.
			pthread_mutex_unlock(&tableroThreads[tmpX][tmpY]);
		}
		//
		// Se intenta atacar a los enemigos alrededor.
		//

	}
	printf("El luchador Nro: %d ha llegado a 100 conteos\n", personaje->nro);
}

int main() {
	ancho = 1;
	alto = 1;
	int i;
	time_t t;
	srand((unsigned) time(&t));
	luchador concursantes[10];
	pthread_t hebras[10];

	crearTableros();
	iniciarTableros();
	for (i = 0; i < 10; i++) {
		concursantes[i].nro = i + 1;
		if ( pthread_create(&hebras[i], NULL, contadorFunc, (void *) &(concursantes[i])) ) {
            printf("something is wrong creating the thread"); 
        }
	}
	for (i = 0; i < 10; i++) {
		pthread_join(hebras[i], NULL);
	}
	return 1;
}
