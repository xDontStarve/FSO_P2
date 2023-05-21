/*****************************************************************************/
/*									                                         */
/*				     cocos2.c				                                 */
/*									                                         */
/*     Programa inicial d'exemple per a les practiques 2.1 i 2.2 de FSO.     */
/*     Es tracta del joc del menjacocos: es dibuixa un laberint amb una      */
/*     serie de punts (cocos), els quals han de ser "menjats" pel menja-     */
/*     cocos. Aquest menjacocos es representara amb el caracter '0', i el    */
/*     moura l'usuari amb les tecles 'w' (adalt), 's' (abaix), 'd' (dreta)   */
/*     i 'a' (esquerra). Simultaniament hi haura un conjunt de fantasmes,    */
/*     representats per numeros de l'1 al 9, que intentaran capturar al      */
/*     menjacocos. En la primera versio del programa, nomes hi ha un fan-    */
/*     tasma.								                                 */
/*     Evidentment, es tracta de menjar tots els punts abans que algun fan-  */
/*     tasma atrapi al menjacocos.					                         */
/*									                                         */
/*  Arguments del programa:						                             */
/*     per controlar la posicio de tots els elements del joc, cal indicar    */
/*     el nom d'un fitxer de text que contindra la seguent informacio:	     */
/*		n_fil n_col fit_tauler creq				                         */
/*		mc_f mc_c mc_d mc_r						                             */
/*		fantasmes_f fantasmes_c fantasmes_d fantasmes_r						                             */
/*									                                         */
/*     on 'n_fil', 'n_col' son les dimensions del taulell de joc, mes una   */
/*     fila pels missatges de text a l'ultima linia. "fit_tauler" es el nom  */
/*     d'un fitxer de text que contindra el dibuix del laberint, amb num. de */
/*     files igual a 'n_fil'-1 i num. de columnes igual a 'n_col'. Dins     */
/*     d'aquest fitxer, hi hauran caracter ASCCII que es representaran en    */
/*     pantalla tal qual, excepte el caracters iguals a 'creq', que es visua-*/
/*     litzaran invertits per representar la paret.			                 */
/*     Els parametres 'mc_f', 'mc_c' indiquen la posicio inicial de fila i   */
/*     columna del menjacocos, aixi com la direccio inicial de moviment      */
/*     (0 -> amunt, 1-> esquerra, 2-> avall, 3-> dreta). Els parametres	     */
/*     'fantasmes_f', 'fantasmes_c' i 'fantasmes_d' corresponen a la mateixa informacio per al    */
/*     fantasma 1. El programa verifica que la primera posicio del menja-    */
/*     cocos o del fantasma no coincideixi amb un bloc de paret del laberint.*/
/*	   'mc_r' 'fantasmes_r' son dos reals que multipliquen el retard del moviment.  */
/*     A mes, es podra afegir un segon argument opcional per indicar el      */
/*     retard de moviment del menjacocos i dels fantasmes (en ms);           */
/*     el valor per defecte d'aquest parametre es 100 (1 decima de segon).   */
/*									                                         */
/*  Compilar i executar:					  	                             */
/*     El programa invoca les funcions definides a 'winsuport.h', les        */
/*     quals proporcionen una interficie senzilla per crear una finestra     */
/*     de text on es poden escriure caracters en posicions especifiques de   */
/*     la pantalla (basada en CURSES); per tant, el programa necessita ser   */
/*     compilat amb la llibreria 'curses':				                     */
/*									                                         */
/*	   $ gcc -Wall cocos0.c winsuport.o -o cocos0 -lcurses		             */
/*	   $ ./cocos0 fit_param [retard]				                         */
/*									                                         */
/*  Codis de retorn:						  	                             */
/*     El programa retorna algun dels seguents codis al SO:		             */
/*	0  ==>  funcionament normal					                             */
/*	1  ==>  numero d'arguments incorrecte 				                     */
/*	2  ==>  fitxer de configuracio no accessible			                 */
/*	3  ==>  dimensions del taulell incorrectes			                     */
/*	4  ==>  parametres del menjacocos incorrectes			                 */
/*	5  ==>  parametres d'algun fantasma incorrectes			                 */
/*	6  ==>  no s'ha pogut crear el camp de joc			                     */
/*	7  ==>  no s'ha pogut inicialitzar el joc			                     */
/*****************************************************************************/

#include <stdio.h>      /* incloure definicions de funcions estandard */
#include <stdlib.h>     /* per exit() */
#include <unistd.h>     /* per getpid() */
#include "winsuport2.h" /* incloure definicions de funcions propies */
#include <pthread.h>    /* threads library */
#include <string.h>
#include <stdbool.h>
#include <sys/wait.h>   /* waitpid() */
#include "menjacocos3.h"
#include "memoria.h"
#include "semafor.h"
#include <stdarg.h>

// Define limits
#define MIN_FIL 7
#define MAX_FIL 25
#define MIN_COL 10
#define MAX_COL 80
// Define default values
#define DEFAULT_RETARD 100

// -- GLOBAL VARIABLES -- //

// Number of rows and columns of the Maze
int n_fil, n_col;
// File name of the Maze
char tauler[70];
// Character for the Wall of the Maze
char c_req;
// Info about the 'menjacocos'
objecte menjacocos;
// Number of loaded 'fantasmes'
int num_fantasmes = 0;
// Number of 'cocos' remaining
int cocos;
// Pointer to the Shared Memory for InterProcess Comunication
void *ptr_shmem_ipc;
// Retard between processing loop executions
int retard;
 // Semaphore to control critical sections while accessing the field
int id_sem_field;
// Pointer to the shared 9 'fantasmes'
objecte *ptr_sh_fantasmes;
// Index of next ghost
int seguent_fantasma;
// Process id of each 'fantasma'
pid_t fantasmes_id[9];
// Identifier of the Shared Field/Window Memory region
u_int32_t id_shmem_field;
// Identifier of the Shared Memory for comunicating processes
u_int32_t id_shmem_ipc;

/* funcio per realitzar la carrega dels parametres de joc emmagatzemats */
/* dins d'un fitxer de text, el nom del qual es passa per referencia a  */
/* 'nom_fit'; si es detecta algun problema, la funcio avorta l'execucio */
/* enviant un missatge per la sortida d'error i retornant el codi per-	*/
/* tinent al SO (segons comentaris al principi del programa).		    */
void carrega_parametres(const char *nom_fit)
{
  FILE *fit;
  objecte *fantasma = ptr_sh_fantasmes;

  fit = fopen(nom_fit, "rt"); /* intenta obrir fitxer */
  if (fit == NULL)
  {
    fprintf(stderr, "No s'ha pogut obrir el fitxer \'%s\'\n", nom_fit);
    exit(2);
  }

  if (!feof(fit))
    fscanf(fit, "%d %d %s %c\n", &n_fil, &n_col, tauler, &c_req);
  else
  {
    fprintf(stderr, "Falten parametres al fitxer \'%s\'\n", nom_fit);
    fclose(fit);
    exit(2);
  }
  if ((n_fil < MIN_FIL) || (n_fil > MAX_FIL) ||
      (n_col < MIN_COL) || (n_col > MAX_COL))
  {
    fprintf(stderr, "Error: dimensions del camp de joc incorrectes:\n");
    fprintf(stderr, "\t%d =< n_fil (%d) =< %d\n", MIN_FIL, n_fil, MAX_FIL);
    fprintf(stderr, "\t%d =< n_col (%d) =< %d\n", MIN_COL, n_col, MAX_COL);
    fclose(fit);
    exit(3);
  }

  if (!feof(fit))
    fscanf(fit, "%d %d %d %f\n", &menjacocos.f, &menjacocos.c, &menjacocos.d, &menjacocos.r);
  else
  {
    fprintf(stderr, "Falten parametres al fitxer \'%s\'\n", nom_fit);
    fclose(fit);
    exit(2);
  }
  if ((menjacocos.f < 1) || (menjacocos.f > n_fil - 3) ||
      (menjacocos.c < 1) || (menjacocos.c > n_col - 2) ||
      (menjacocos.d < 0) || (menjacocos.d > 3))
  {
    fprintf(stderr, "Error: parametres menjacocos incorrectes:\n");
    fprintf(stderr, "\t1 =< menjacocos.f (%d) =< n_fil-3 (%d)\n", menjacocos.f, (n_fil - 3));
    fprintf(stderr, "\t1 =< menjacocos.c (%d) =< n_col-2 (%d)\n", menjacocos.c, (n_col - 2));
    fprintf(stderr, "\t0 =< menjacocos.d (%d) =< 3\n", menjacocos.d);
    fclose(fit);
    exit(4);
  }

  do
  {
    if (feof(fit))
      break;

    fscanf(fit, "%d %d %d %f\n", &fantasma->f, &fantasma->c, &fantasma->d, &fantasma->r);
    if ((fantasma->f < 1) || (fantasma->f > n_fil - 3) ||
        (fantasma->c < 1) || (fantasma->c > n_col - 2) ||
        (fantasma->d < 0) || (fantasma->d > 3))
    {
      fprintf(stderr, "Error: parametres fantasma %d incorrectes:\n", num_fantasmes);
      fprintf(stderr, "\t1 =< fantasmes.f (%d) =< n_fil-3 (%d)\n", fantasma->f, (n_fil - 3));
      fprintf(stderr, "\t1 =< fantasmes.c (%d) =< n_col-2 (%d)\n", fantasma->c, (n_col - 2));
      fprintf(stderr, "\t0 =< fantasmes.d (%d) =< 3\n", fantasma->d);
      fclose(fit);
      exit(5);
    }
    num_fantasmes++;
    // Posicion del siguiente fantasma
    fantasma++;
  } while (num_fantasmes < 9);

  fclose(fit); /* fitxer carregat: tot OK! */
  printf("Joc del MenjaCocos\n\tTecles: \'%c\', \'%c\', \'%c\', \'%c\', RETURN-> sortir\n",
         TEC_AMUNT, TEC_AVALL, TEC_DRETA, TEC_ESQUER);
  printf("prem una tecla per continuar:\n");
  getchar();
}

/* funcio per inicialitar les variables i visualitzar l'estat inicial del joc */
void inicialitza_joc(void)
{
  int r, i, j;
  char strin[12];
  objecte *fantasma = ptr_sh_fantasmes;

  r = win_carregatauler(tauler, n_fil - 1, n_col, c_req);
  if (r == 0)
  {
    menjacocos.a = win_quincar(menjacocos.f, menjacocos.c);
    if (menjacocos.a == c_req)
    {
      r = -6; /* error: menjacocos sobre pared */
    }
    else
    {

      /* compta el numero total de cocos */
      cocos = 0;
      for (i = 0; i < n_fil - 1; i++)
        for (j = 0; j < n_col; j++)
          if (win_quincar(i, j) == '.')
            cocos++;

      // Escriure ComeCocos
      win_escricar(menjacocos.f, menjacocos.c, '0', NO_INV);

      // Menja primer coco
      if (menjacocos.a == '.')
        cocos--;

      // Escriure marcador de cocos
      sprintf(strin, "Cocos: %d", cocos);
      win_escristr(strin);

      for (int k = 0; k < num_fantasmes; k++)
      {
        fantasma->a = win_quincar(fantasma->f, fantasma->c);
        if (fantasma->a == c_req)
          r = -7; /* error: fantasma sobre pared */
        fantasma++;
      }

    }
  }
  if (r != 0)
  {
    win_fi();
    fprintf(stderr, "Error: no s'ha pogut inicialitzar el joc:\n");
    switch (r)
    {
    case -1:
      fprintf(stderr, "  nom de fitxer erroni\n");
      break;
    case -2:
      fprintf(stderr, "  numero de columnes d'alguna fila no coincideix amb l'amplada del tauler de joc\n");
      break;
    case -3:
      fprintf(stderr, "  numero de columnes del laberint incorrecte\n");
      break;
    case -4:
      fprintf(stderr, "  numero de files del laberint incorrecte\n");
      break;
    case -5:
      fprintf(stderr, "  finestra de camp de joc no oberta\n");
      break;
    case -6:
      fprintf(stderr, "  posicio inicial del menjacocos damunt la pared del laberint\n");
      break;
    case -7:
      fprintf(stderr, "  posicio inicial del fantasma damunt la pared del laberint\n");
      break;
    }
    exit(7);
  }
}

//TODO: Make `crear_fantasma` a varargs function
void crear_fantasma()
{
  if (seguent_fantasma >= num_fantasmes) return;

  // Variables to store the arguments to 'fantasmes.c' processes
  char a1[20], a2[20], a3[20], a4[20], a5[20], a6[20], a7[20];

  objecte *fantasma = ptr_sh_fantasmes + seguent_fantasma;
  // Preparing the arguments of the `fantasma` process
  sprintf(a1, "%d", seguent_fantasma);
  sprintf(a2, "%i", id_shmem_ipc);
  sprintf(a3, "%i", id_shmem_field);
  sprintf(a4, "%i", n_fil);
  sprintf(a5, "%i", n_col);
  sprintf(a6, "%i", retard);
  sprintf(a7, "%i", id_sem_field);
  
  fantasmes_id[seguent_fantasma] = fork();
  if (fantasmes_id[seguent_fantasma] == (pid_t)0) {
    execlp("./fantasma4", "fantasma4", a1, a2, a3, a4, a5, a6, a7, (char*)0);
    fprintf(stderr, "Error: Can't execute child process `fantasma3`\n");
    exit(1);
  }
  waitS(id_sem_field);
  win_escricar(fantasma->f, fantasma->c, '1' + seguent_fantasma, NO_INV); // Escriure fantasmes
  signalS(id_sem_field);
  seguent_fantasma++;
}

/* funcio per moure el menjacocos una posicio, en funcio de la direccio de   */
/* moviment actual; retorna -1 si s'ha premut RETURN, 1 si s'ha menjat tots  */
/* els cocos, i 0 altrament */
void *mou_menjacocos(void *null)
{
  int *ptr_fi1 = (int *)(ptr_shmem_ipc + OFFSET_FI1);
  int *ptr_fi2 = (int *)(ptr_shmem_ipc + OFFSET_FI2);
  int ant_mur_f = -1;
  int ant_mur_c = -1;

  do
  {
    char strin[12];
    objecte seg;
    int tec;

    tec = win_gettec();
    if (tec != 0)
    {
      switch (tec) /* modificar direccio menjacocos segons tecla */
      {
      case TEC_AMUNT:
        menjacocos.d = 0;
        break;
      case TEC_ESQUER:
        menjacocos.d = 1;
        break;
      case TEC_AVALL:
        menjacocos.d = 2;
        break;
      case TEC_DRETA:
        menjacocos.d = 3;
        break;
      case TEC_RETURN:
        *ptr_fi1 = 1;
        break;
      }
    }
    seg.f = menjacocos.f + df[menjacocos.d]; /* calcular seguent posicio */
    seg.c = menjacocos.c + dc[menjacocos.d];
    seg.a = win_quincar(seg.f, seg.c); /* calcular caracter seguent posicio */
    if ((seg.a == ' ') || (seg.a == '.'))
    {

      // Start of a critical section
      waitS(id_sem_field);

      // esborra posicio anterior
      win_escricar(menjacocos.f, menjacocos.c, ' ', NO_INV);
    
      // actualitza posicio
      menjacocos.f = seg.f;
      menjacocos.c = seg.c;

      // redibuixa menjacocos
      win_escricar(menjacocos.f, menjacocos.c, '0', NO_INV);

      // End of the critical section
      signalS(id_sem_field);

      if (seg.a == '.')
      {
        cocos--;
        sprintf(strin, "Cocos: %d", cocos);
        win_escristr(strin);
        if (cocos == 0)
          *ptr_fi1 = 1;
      }
    } else if (seg.a == c_req) {
      // If the previous wall variable is initialized and is diferent than the actual wall then it creates a `fantasma`
      if ((ant_mur_f != -1) && (ant_mur_c != -1) && (seg.f != ant_mur_f) && (seg.c != ant_mur_c))
      { crear_fantasma();
        // reset the previous wall
        ant_mur_f = -1; ant_mur_c = -1;
      } else 
      { ant_mur_f = seg.f; ant_mur_c = seg.c;
      }
    }

    win_retard(retard);
  } while (!*ptr_fi1 && !*ptr_fi2);

  return ((void *)((u_int64_t)*ptr_fi1));
}

// --- MAIN PROGRAMM --- //
int main(int argc, const char *argv[])
{
  // -- LOCAL VARIABLES -- //

  // Pointer to the shared memory location of fi1
  u_int32_t *ptr_sh_fi1;
  // Pointer to the shared memory location of fi2
  u_int32_t *ptr_sh_fi2;
  // Pointer to the Shared Field/Window Memory
  u_int32_t *ptr_shmem_field;
  // Size (bytes) of the Window/Field
  u_int32_t field_size;
  // Identifier of the 'menjacocos' thread
  pthread_t thread_menjacocos;

  // -- CHECKING ARGUMENTS -- //

  if ((argc != 2) && (argc != 3))
  {
    fprintf(stderr, "Comanda: %s fit_param [retard]\n", argv[0]);
    exit(1);
  }

  // -- INITIALIZATION OF VARIABLES -- //

  srand(getpid());

  // Initializing the semaphore to 1
  id_sem_field = ini_sem(1);

  // Initializing IPC Shared Memory
  id_shmem_ipc = ini_mem(
      sizeof(int) +       // fi1
      sizeof(int) +       // fi2
      9 * sizeof(objecte) // 9 'fantasmes'
  );

  // Mapping the shared memory
  ptr_shmem_ipc = map_mem(id_shmem_ipc);
  assert(ptr_shmem_ipc);
  ptr_sh_fantasmes = (objecte *)(ptr_shmem_ipc + OFFSET_FANTASMES);
  ptr_sh_fi1 = (u_int32_t *)(ptr_shmem_ipc + OFFSET_FI1);
  ptr_sh_fi2 = (u_int32_t *)(ptr_shmem_ipc + OFFSET_FI2);

  // Loading the parameters
  carrega_parametres(argv[1]);

  // Asigning or defaulting 'retard'
  if (argc == 3)
    retard = atoi(argv[2]);
  else
    retard = DEFAULT_RETARD;

  // Initializing the field
  field_size = win_ini(&n_fil, &n_col, '+', INVERS);
  if (field_size < 0)
  {
    switch (field_size)
    {
    case -1:
      fprintf(stderr, "camp de joc ja creat!\n");
      break;
    case -2:
      fprintf(stderr, "no s'ha pogut inicialitzar l'entorn de curses!\n");
      break;
    case -3:
      fprintf(stderr, "les mides del camp demanades son massa grans!\n");
      break;
    case -4:
      fprintf(stderr, "no s'ha pogut crear la finestra!\n");
      break;
    }
    elim_mem(id_shmem_ipc); /* elimina zona de memoria compartida */
    exit(2);
  }

  // Memoria compartida del campo de juego
  id_shmem_field = ini_mem(field_size);      			/* crear zona mem. compartida */
  ptr_shmem_field = (u_int32_t *) map_mem(id_shmem_field); 	/* obtenir adres. de mem. compartida */
  win_set(ptr_shmem_field, n_fil, n_col);    			/* crea acces a finestra oberta */
  
  inicialitza_joc();
  // Crear el hilo del comecocos
  pthread_create(&thread_menjacocos, NULL, mou_menjacocos, NULL);

  //// Crear un proceso por cada fantasma
  //for (int i = 0; i < num_fantasmes; i++)
  //{
  //  sprintf(a1, "%d", i);
  //  fantasmes_id[i] = fork();
  //  if (fantasmes_id[i] == (pid_t)0) {
  //    execlp("./fantasma4", "fantasma4", a1, a2, a3, a4, a5, a6, a7, (char*)0);
  //    fprintf(stderr, "Error: Can't execute child process `fantasma3`\n");
  //    exit(1);
  //  }
  //}
  //

  crear_fantasma();

  // Main loop: Refresh the field
  do {
    win_update();
    win_retard(100);
  } while (!*ptr_sh_fi1 && !*ptr_sh_fi2);

  // Wait for the `menjacocos` thread to finish execution
  void* return_value;
  pthread_join(thread_menjacocos, &return_value);
  // Get the return code of the `menjacocos` thread
  u_int64_t fi1 = (u_int64_t) return_value;

  for (int i = 0; i < num_fantasmes; i++)
  {
    int f2;
    waitpid(fantasmes_id[i], &f2, 0);
  }

  win_fi();

  if (fi1 < 0)
    printf("S'ha aturat el joc amb tecla RETURN!\n");
  else if (fi1)
    printf("Ha guanyat l'usuari!\n");
  else
    printf("Ha guanyat l'ordinador!\n");

  // Eliminem la zona de memoria compartida
  elim_mem(id_shmem_ipc);
  elim_mem(id_shmem_field);
  // Eliminem el semafor
  elim_sem(id_sem_field);
  return (0);
}
