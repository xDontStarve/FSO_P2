#pragma once

#define OFFSET_FI1 0
#define OFFSET_FI2 4
#define OFFSET_BUSTIES 8
#define OFFSET_FANTASMES 44

#define INVALID_MOVE '?'
#define MAX_VALID_MOVES 4
#define MAX_TRANSLUCENT_SYMBOLS 2
#define COCO_SYMBOL '.'
#define MENJACOCOS_SYMBOL '0'
#define AIR_SYMBOL ' '

/* per un objecte (menjacocos o fantasma) */
typedef struct {		
	int f;				/* posicio actual: fila */
	int c;				/* posicio actual: columna */
	int d;				/* direccio actual: [0..3] */
  	float r;      		/* per indicar un retard relati */
	char a;				/* caracter anterior en pos. actual */
} objecte;

int df[] = {-1, 0, 1, 0};	/* moviments de les 4 direccions possibles */
int dc[] = {0, -1, 0, 1};	/* dalt, esquerra, baix, dreta */
