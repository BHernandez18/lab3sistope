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
	int realizoAtaque;
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
guerrero nullGuerrero;
guerrero * guerreros; // Lista de guerreros.
guerrero ** tableroGuerreros; // Tablero de los guerreros para la vista.
pthread_t * hebras; // Hebras del juego.
pthread_mutex_t ** tableroThreads; // Tablero para los movimientos.
// Quizas se deba añadir un 3er tablero para los ataques.

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
    nullGuerrero.valido = 0;
    hebras = (pthread_t *)malloc(sizeof(pthread_t) * n);
    guerreros = (guerrero *)malloc(sizeof(guerrero) * n);
    archivoEntrada = fopen(nombreArchivo, "r");
    while (i < n) {
        fscanf(archivoEntrada, "%d %d %d %s", &hp, &colorUniverso, &universo, nombreGuerrero);
        strcpy(guerreros[i].nombreGuerrero, nombreGuerrero);
        guerreros[i].hp = hp;
        guerreros[i].ki = 0;
        guerreros[i].realizoAtaque = 0;
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
	tableroThreads = (pthread_mutex_t **)malloc(sizeof(pthread_mutex_t *) * alto);
	tableroGuerreros = (guerrero **)malloc(sizeof(guerrero *) * alto);
	for (i = 0; i < alto; i++) {
		// Se crean las columnas de los tableros.
		tableroThreads[i] = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t) * ancho);
		tableroGuerreros[i] = (guerrero *)malloc(sizeof(guerrero) * ancho);
	}
}

void iniciarTableros() {
	int i, j;
	for (i = 0; i < alto; i++) {
		for (j = 0; j < ancho; j++) {
			// Si no se puede inicializar algún 'mutex', se cierra el juego.
			if (pthread_mutex_init( &tableroThreads[i][j], NULL ) != 0) {
				fprintf(stderr, "<> ERROR: Problema al iniciar el tablero. <>");
				exit(1);
			}
		}
	}
}

int realizarAtaque(int ki, int universo, int xPos, int yPos) {
	int danoTotal, tmpX, tmpY, realizoAtaque;
	guerrero * enemigo;
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
		enemigo = &tableroGuerreros[tmpX][tmpY];
		// Si hay un posible enemigo.
		if (enemigo->valido != 0) {
			// Si son de universos distintos.
			if (enemigo->universo != universo) {
				// Se resta la vida al guerrero.
				enemigo->hp = enemigo->hp - danoTotal;
				realizoAtaque = 1;
			}
		}	
	}

	// === Enemigo de abajo ===
	tmpX = xPos + 1;
	tmpY = yPos;
	// Verificación de posición.
	if (tmpX >= 0) {
		// Se realiza el ataque.
		enemigo = &tableroGuerreros[tmpX][tmpY];
		// Si hay un posible enemigo.
		if (enemigo->valido != 0) {
			// Si son de universos distintos.
			if (enemigo->universo != universo) {
				// Se resta la vida al guerrero.
				enemigo->hp = enemigo->hp - danoTotal;
				realizoAtaque = 1;
			}
		}	
	}

	// === Enemigo de la izquierda ===
	tmpX = xPos;
	tmpY = yPos - 1;
	// Verificación de posición.
	if (tmpX >= 0) {
		// Se realiza el ataque.
		enemigo = &tableroGuerreros[tmpX][tmpY];
		// Si hay un posible enemigo.
		if (enemigo->valido != 0) {
			// Si son de universos distintos.
			if (enemigo->universo != universo) {
				// Se resta la vida al guerrero.
				enemigo->hp = enemigo->hp - danoTotal;
				realizoAtaque = 1;
			}
		}	
	}

	// === Enemigo de la derecha ===
	xPos = xPos;
	yPos = yPos + 1;
	// Verificación de posición.
	if (tmpX >= 0) {
		// Se realiza el ataque.
		enemigo = &tableroGuerreros[tmpX][tmpY];
		// Si hay un posible enemigo.
		if (enemigo->valido != 0) {
			// Si son de universos distintos.
			if (enemigo->universo != universo) {
				// Se resta la vida al guerrero.
				enemigo->hp = enemigo->hp - danoTotal;
				realizoAtaque = 1;
			}
		}	
	}
	return realizoAtaque;
}

void * entrarArena(void * concursante) {
	int sigMovimiento, xPos, yPos, tmpX, tmpY;
	// Se castea el concursante.
	guerrero * personaje = (guerrero *) concursante;
	// Loop para colocarlo en un lugar disponible de la arena.
	while (1) {
		// Se obtiene una posición aleatoria del tablero.
		xPos = rand() % alto;
		yPos = rand() % ancho;
		// Si se puede ubicar al personaje en tal posición.
		if (pthread_mutex_trylock(&tableroThreads[xPos][yPos]) == 0) {
			personaje->xPos = xPos;
			personaje->yPos = yPos;
			tableroGuerreros[xPos][yPos] = *personaje;
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
		printf("El guerrero %s sigue vivo\n", personaje->nombreGuerrero);
		sleep(1);
		if (pthread_mutex_trylock(&tableroThreads[xPos][yPos]) == 0) {
			// Se obtiene temporalmente la ubicación actual del personaje.
			tmpX = personaje->xPos;
			tmpY = personaje->yPos;
			// Se ubica al personaje en las coordenadas dadas.
			tableroGuerreros[xPos][yPos] = *personaje;
			// Se asigna la nueva ubicación del personaje.
			personaje->xPos = xPos;
			personaje->yPos = yPos;
			// Se 'elimina' al personaje de la anterior ubicación.
			tableroGuerreros[tmpX][tmpY] = nullGuerrero;
			// Se libera el mutex de la ubicación anterior del personaje.
			pthread_mutex_unlock(&tableroThreads[tmpX][tmpY]);
		}
		//
		// Se intenta atacar a los enemigos alrededor.
		//
		personaje->realizoAtaque = realizarAtaque(personaje->ki, personaje->universo, personaje->xPos, personaje->yPos);
		if (personaje->realizoAtaque == 1) { personaje->ki = 0; }
		else { personaje->ki = personaje->ki + 1; }
	}
	personaje->valido = 0;
	printf("El guerrero %s acaba de perder\n", personaje->nombreGuerrero);
}

int revisarGuerreros() {
	int i, universoTmp;
	universoTmp = guerreros[0].universo;
	for (i = 1; i < nroGuerreros; i++) {
		if (guerreros[i].universo != universoTmp) {
			return 1;
		}
	}
	return 0;
}

void comenzarJuego() {
	int i;
	for (i = 0; i < 10; i++) {
		if ( pthread_create(&hebras[i], NULL, entrarArena, (void *) &guerreros[i]) ) {
            printf("something is wrong creating the thread"); 
        }
	}
	for (i = 0; i < 10; i++) {
		pthread_join(hebras[i], NULL);
	}
}

int verGuerreros() {
    int i = 0;
    while(i < nroGuerreros) {
    	printf("----------\n");
        printf("%d %d %d %s\n", guerreros[i].hp, guerreros[i].colorUniverso, guerreros[i].universo, guerreros[i].nombreGuerrero);
        printf("----------\n");
        i++;
    }
    return 1;
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
	comenzarJuego();
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
