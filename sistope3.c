#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>

// Estructura para identificar un guerrero.
typedef struct guerrero {
	char nombreGuerrero[50];
	int hp;
	int ki;
	int id;
	int universo;
	int colorUniverso;
	int xPos;
	int yPos;
	int valido;
} guerrero;

// Dimensiones de la arena.
int ancho;
int alto;
int nroGuerreros; // Nro. de guerreros.
guerrero * guerreros; // Lista de guerreros.
int ** tableroGuerreros; // Tablero de los guerreros.
pthread_t * hebras; // Hebras del juego.
pthread_mutex_t * mutexPersonajes; // Lista para realizar los ataques.
pthread_mutex_t ** tableroAtaques; // Tablero para los ataques.
pthread_mutex_t ** tableroMovimientos; // Tablero para los movimientos.

int verGuerreros();
int cantidadGuerreros(char * nombreArchivo);
int realizarAtaque(int ki, int universo, int xPos, int yPos);
void crearTableros();
void iniciarTableros();
void leerArchivo(char * nombreArchivo, int n);
void * entrarArena(void * concursante);

int cantidadGuerreros(char * nombreArchivo) {
	FILE * archivoEntrada;
    int lineas;

    lineas = 0;
    archivoEntrada = fopen(nombreArchivo, "r");
    while ( !feof(archivoEntrada) ) {
        if (fgetc(archivoEntrada) == '\n') {
            if (fgetc(archivoEntrada) != (EOF || NULL)) {
                lineas++;
            }
        }
    }
    fclose(archivoEntrada);
    return lineas;
}

void leerArchivo(char * nombreArchivo, int n) {
	FILE * archivoEntrada;
    int hp, colorUniverso, universo, i;
    char nombreGuerrero[50];

    i = 0;
    hebras = (pthread_t *)malloc(sizeof(pthread_t) * n);
    guerreros = (guerrero *)malloc(sizeof(guerrero) * n);
    archivoEntrada = fopen(nombreArchivo, "r");
    while (i < n) {
        fscanf(archivoEntrada, "%d %d %d %s", &hp, &colorUniverso, &universo, nombreGuerrero);
        strcpy(guerreros[i].nombreGuerrero, nombreGuerrero);
        guerreros[i].hp = hp;
        guerreros[i].ki = 0;
        guerreros[i].id = i;
        guerreros[i].universo = universo;
        guerreros[i].colorUniverso = colorUniverso;
        guerreros[i].valido = 1;
        i++;
    }
    fclose(archivoEntrada);
}

void crearTableros() {
	int i;
	// Se crean las filas de los tableros.
	mutexPersonajes = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t) * nroGuerreros);
	tableroAtaques = (pthread_mutex_t **)malloc(sizeof(pthread_mutex_t *) * alto);
	tableroMovimientos = (pthread_mutex_t **)malloc(sizeof(pthread_mutex_t *) * alto);
	tableroGuerreros = (int **)malloc(sizeof(int *) * alto);
	for (i = 0; i < alto; i++) {
		// Se crean las columnas de los tableros.
		tableroAtaques[i] = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t) * ancho);
		tableroMovimientos[i] = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t) * ancho);
		tableroGuerreros[i] = (int *)malloc(sizeof(int) * ancho);
	}
}

void iniciarTableros() {
	int i, j;
	for (i = 0; i < alto; i++) {
		if (pthread_mutex_init( &mutexPersonajes[i], NULL ) != 0) {
			fprintf(stderr, "<> ERROR: Problema al iniciar el tablero. <>");
			exit(1);
		}
		for (j = 0; j < ancho; j++) {
			// Si no se puede inicializar algún 'mutex', se cierra el juego.
			if (pthread_mutex_init( &tableroMovimientos[i][j], NULL ) != 0) {
				fprintf(stderr, "<> ERROR: Problema al iniciar el tablero. <>");
				exit(1);
			}
			if (pthread_mutex_init( &tableroAtaques[i][j], NULL ) != 0) {
				fprintf(stderr, "<> ERROR: Problema al iniciar el tablero. <>");
				exit(1);
			}
			tableroGuerreros[i][j] = -1;
		}
	}
}

int realizarAtaque(int ki, int universo, int xPos, int yPos) {
	int danoTotal, tmpX, tmpY, realizoAtaque;
	int enemigoId;
	// Configuración de si se realizó un ataque o no.
	realizoAtaque = 0;
	// Daño total a realizar a los personajes.
	danoTotal = ki * 5;
	// Se obtienen los posibles enemigos y se realiza el ataque.
	// === Enemigo de arriba ===
	tmpX = xPos - 1;
	tmpY = yPos;
	// Verificación de posición.
	if (tmpX >= 0) {
		// Se realiza el ataque.
		enemigoId = tableroGuerreros[tmpX][tmpY];
		// Se intenta hacer un 'lock' al mutex.
		pthread_mutex_lock(&mutexPersonajes[enemigoId]);
		// Si hay un posible enemigo.
		if (enemigoId != -1) {
			// Si son de universos distintos.
			if (guerreros[enemigoId].universo != universo && guerreros[enemigoId].valido != 0) {
				// Se resta la vida al guerrero.
				guerreros[enemigoId].hp = guerreros[enemigoId].hp - danoTotal;
				realizoAtaque = 1;
			}
		}
		// Se libera el mutex.
		pthread_mutex_unlock(&mutexPersonajes[enemigoId]);
	}

	// === Enemigo de abajo ===
	tmpX = xPos + 1;
	tmpY = yPos;
	// Verificación de posición.
	if (tmpX >= 0) {
		// Se realiza el ataque.
		enemigoId = tableroGuerreros[tmpX][tmpY];
		// Se intenta hacer un 'lock' al mutex.
		pthread_mutex_lock(&mutexPersonajes[enemigoId]);
		// Si hay un posible enemigo.
		if (enemigoId != -1) {
			// Si son de universos distintos.
			if (guerreros[enemigoId].universo != universo && guerreros[enemigoId].valido != 0) {
				// Se resta la vida al guerrero.
				guerreros[enemigoId].hp = guerreros[enemigoId].hp - danoTotal;
				realizoAtaque = 1;
			}
		}
		// Se libera el mutex.
		pthread_mutex_unlock(&mutexPersonajes[enemigoId]);
	}

	// === Enemigo de la izquierda ===
	tmpX = xPos;
	tmpY = yPos - 1;
	// Verificación de posición.
	if (tmpX >= 0) {
		// Se realiza el ataque.
		enemigoId = tableroGuerreros[tmpX][tmpY];
		// Se intenta hacer un 'lock' al mutex.
		pthread_mutex_lock(&mutexPersonajes[enemigoId]);
		// Si hay un posible enemigo.
		if (enemigoId != -1) {
			// Si son de universos distintos.
			if (guerreros[enemigoId].universo != universo && guerreros[enemigoId].valido != 0) {
				// Se resta la vida al guerrero.
				guerreros[enemigoId].hp = guerreros[enemigoId].hp - danoTotal;
				realizoAtaque = 1;
			}
		}
		// Se libera el mutex.
		pthread_mutex_unlock(&mutexPersonajes[enemigoId]);
	}

	// === Enemigo de la derecha ===
	xPos = xPos;
	yPos = yPos + 1;
	// Verificación de posición.
	if (tmpX >= 0) {
		// Se realiza el ataque.
		enemigoId = tableroGuerreros[tmpX][tmpY];
		// Se intenta hacer un 'lock' al mutex.
		pthread_mutex_lock(&mutexPersonajes[enemigoId]);
		// Si hay un posible enemigo.
		if (enemigoId != -1) {
			// Si son de universos distintos.
			if (guerreros[enemigoId].universo != universo && guerreros[enemigoId].valido != 0) {
				// Se resta la vida al guerrero.
				guerreros[enemigoId].hp = guerreros[enemigoId].hp - danoTotal;
				realizoAtaque = 1;
			}
		}
		// Se libera el mutex.
		pthread_mutex_unlock(&mutexPersonajes[enemigoId]);
	}
	return realizoAtaque;
}

void * entrarArena(void * concursante) {
	int sigMovimiento, xPos, yPos, tmpX, tmpY, idPersonaje, ataqueHecho;
	guerrero * personaje;
	// Se castea el concursante.
	personaje = (guerrero *) concursante;
	idPersonaje = personaje->id;

	// Loop para colocarlo en un lugar disponible de la arena.
	while (1) {
		// Se obtiene una posición aleatoria del tablero.
		xPos = rand() % alto;
		yPos = rand() % ancho;
		// Si se puede ubicar al personaje en tal posición.
		if (pthread_mutex_trylock(&tableroMovimientos[xPos][yPos]) == 0) {
			pthread_mutex_lock(&mutexPersonajes[idPersonaje]);
			personaje->xPos = xPos;
			personaje->yPos = yPos;
			tableroGuerreros[xPos][yPos] = idPersonaje;
			pthread_mutex_unlock(&mutexPersonajes[idPersonaje]);
			break;
		}
	}
	//
	// Mientras el guerrero aún tenga vida.
	//
	while (personaje->hp > 0 && personaje->valido == 1) {
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
		//printf("El guerrero %s sigue vivo\n", personaje->nombreGuerrero);
		sleep(1);
		if (pthread_mutex_trylock(&tableroMovimientos[xPos][yPos]) == 0) {
			// Se obtiene temporalmente la ubicación actual del personaje.
			tmpX = personaje->xPos;
			tmpY = personaje->yPos;
			// Se ubica al personaje en las coordenadas dadas.
			tableroGuerreros[xPos][yPos] = idPersonaje;
			// Se 'elimina' al personaje de la anterior ubicación.
			tableroGuerreros[tmpX][tmpY] = -1;
			// Se asigna la nueva ubicación del personaje.
			personaje->xPos = xPos;
			personaje->yPos = yPos;
			// Se libera el mutex de la ubicación anterior del personaje.
			pthread_mutex_unlock(&tableroMovimientos[tmpX][tmpY]);
		} else {
			pthread_mutex_lock(&mutexPersonajes[idPersonaje]);
			personaje->ki = personaje->ki + 1;
			pthread_mutex_unlock(&mutexPersonajes[idPersonaje]);
		}
		//
		// Se intenta atacar a los enemigos alrededor.
		//
		ataqueHecho = realizarAtaque(personaje->ki, personaje->universo, personaje->xPos, personaje->yPos);
		if (ataqueHecho == 1) { personaje->ki = 0; }
		else { personaje->ki = personaje->ki + 1; }
	}
	//personaje->valido = 0;
	//printf("El guerrero %s acaba de perder\n", personaje->nombreGuerrero);
}

int revisarGuerreros() {
	int i, universoTmp;
	universoTmp = guerreros[0].universo;
	for (i = 1; i < nroGuerreros; i++) {
		if (guerreros[i].universo != universoTmp && guerreros[i].valido) {
			return 1;
		}
	}
	return 0;
}

void comenzarJuego() {
	int i;
	for (i = 0; i < nroGuerreros; i++) {
		if ( pthread_create(&hebras[i], NULL, entrarArena, (void *) &guerreros[i]) ) {
            printf("something is wrong creating the thread"); 
        }
	}
	for (i = 0; i < nroGuerreros; i++) {
		pthread_join(hebras[i], NULL);
	}
}

int verGuerreros() {
    int i = 0;
    while(i < nroGuerreros) {
        printf("%d %d %d %s\n", guerreros[i].hp, guerreros[i].colorUniverso, guerreros[i].universo, guerreros[i].nombreGuerrero);
        i++;
    }
    return 1;
}

void * mostrarInformacion() {
	int i, j;
	guerrero aux;
	while(1) {
		sleep(1);
		system("clear");
		// Se muestra el tablero.
		printf("\n\n");
		for (i = 0; i < alto; i++) {
			printf("|");
			for (j = 0; j < ancho; j++) {
				if (tableroGuerreros[i][j] == -1) { printf("  |"); }
				else { printf("%2d|", tableroGuerreros[i][j]); }
			}
			printf("\n");
		}
		printf("\n\n");
		for (i = 0; i < nroGuerreros; i++) {
			aux = guerreros[i];
			printf("[%d] %10s -> HP:%4d  |  Universo:%d  |  KI:%2d \n", i, aux.nombreGuerrero, aux.hp, aux.universo, aux.ki);
		}
		printf("\n\n");
	}
}

int main() {
	int i;
	time_t t;
	ancho = 7;
	alto = 7;
	// Se configura el rand();
	srand((unsigned) time(&t));
	// Se obtiene la cantidad de guerreros.
	nroGuerreros = cantidadGuerreros("entrada.txt");
	// Se lee la información sobre los guerreros y se crea
	// la lista de guerreros que entrarán a la arena.
	leerArchivo("entrada.txt", nroGuerreros);
	// Se crean e inicializan los tableros.
	crearTableros();
	iniciarTableros();
	//comenzarJuego();
	for (i = 0; i < nroGuerreros; i++) {
		if ( pthread_create(&hebras[i], NULL, entrarArena, (void *) &guerreros[i]) ) {
            printf("something is wrong creating the thread"); 
        }
	}
	pthread_t show;
	pthread_create(&show, NULL, mostrarInformacion, NULL);
	for (i = 0; i < nroGuerreros; i++) {
		pthread_join(hebras[i], NULL);
	}
	while (sleep(1) && mostrarInformacion());
	/*
	for (i = 0; i < 10; i++) {
		if ( pthread_create(&hebras[i], NULL, contadorFunc, (void *) &guerreros[i]) ) {
            printf("something is wrong creating the thread"); 
        }
	}
	for (i = 0; i < 10; i++) {
		pthread_join(hebras[i], NULL);
	}
	*/
	return 1;
}
