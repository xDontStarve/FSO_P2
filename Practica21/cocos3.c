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
/*		n_fil1 n_col fit_tauler creq				                         */
/*		mc_f mc_c mc_d mc_r						                             */
/*		fantasmes_f fantasmes_c fantasmes_d fantasmes_r						                             */
/*									                                         */
/*     on 'n_fil1', 'n_col' son les dimensions del taulell de joc, mes una   */
/*     fila pels missatges de text a l'ultima linia. "fit_tauler" es el nom  */
/*     d'un fitxer de text que contindra el dibuix del laberint, amb num. de */
/*     files igual a 'n_fil1'-1 i num. de columnes igual a 'n_col'. Dins     */
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



#include <stdio.h>		/* incloure definicions de funcions estandard */
#include <stdlib.h>		/* per exit() */
#include <unistd.h>		/* per getpid() */
#include "winsuport2.h"		/* incloure definicions de funcions propies */
#include <pthread.h>  /* threads */
#include <string.h>
#include "menjacocos3.h"
#include "memoria.h"


#define MIN_FIL 7		/* definir limits de variables globals */
#define MAX_FIL 25
#define MIN_COL 10
#define MAX_COL 80


/* variables globals */
int n_fil1, n_col;		/* dimensions del camp de joc */
char tauler[70];		/* nom del fitxer amb el laberint de joc */
char c_req;			    /* caracter de pared del laberint */

objecte mc;      		/* informacio del menjacocos */
int num_fantasma = 0;
int cocos;			/* numero restant de cocos per menjar */
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void * ptr_shmem;
int *ptr_retard;
objecte *ptr_fantasmes;

/* funcio per realitzar la carrega dels parametres de joc emmagatzemats */
/* dins d'un fitxer de text, el nom del qual es passa per referencia a  */
/* 'nom_fit'; si es detecta algun problema, la funcio avorta l'execucio */
/* enviant un missatge per la sortida d'error i retornant el codi per-	*/
/* tinent al SO (segons comentaris al principi del programa).		    */
void carrega_parametres(const char *nom_fit)
{
  FILE *fit;
  objecte *fantasma = ptr_shmem;

  fit = fopen(nom_fit,"rt");		/* intenta obrir fitxer */
  if (fit == NULL)
  {	fprintf(stderr,"No s'ha pogut obrir el fitxer \'%s\'\n",nom_fit);
  	exit(2);
  }

  if (!feof(fit)) fscanf(fit,"%d %d %s %c\n",&n_fil1,&n_col,tauler,&c_req);
  else {
    fprintf(stderr,"Falten parametres al fitxer \'%s\'\n",nom_fit);
    fclose(fit);
    exit(2);
	}
  if ((n_fil1 < MIN_FIL) || (n_fil1 > MAX_FIL) ||
	(n_col < MIN_COL) || (n_col > MAX_COL))
  {
    fprintf(stderr,"Error: dimensions del camp de joc incorrectes:\n");
    fprintf(stderr,"\t%d =< n_fil1 (%d) =< %d\n",MIN_FIL,n_fil1,MAX_FIL);
    fprintf(stderr,"\t%d =< n_col (%d) =< %d\n",MIN_COL,n_col,MAX_COL);
    fclose(fit);
    exit(3);
  }

  if (!feof(fit)) fscanf(fit,"%d %d %d %f\n",&mc.f,&mc.c,&mc.d,&mc.r);
  else {
    fprintf(stderr,"Falten parametres al fitxer \'%s\'\n",nom_fit);
    fclose(fit);
    exit(2);
	}
  if ((mc.f < 1) || (mc.f > n_fil1-3) ||
    (mc.c < 1) || (mc.c > n_col-2) ||
    (mc.d < 0) || (mc.d > 3))
  {
    fprintf(stderr,"Error: parametres menjacocos incorrectes:\n");
    fprintf(stderr,"\t1 =< mc.f (%d) =< n_fil1-3 (%d)\n",mc.f,(n_fil1-3));
    fprintf(stderr,"\t1 =< mc.c (%d) =< n_col-2 (%d)\n",mc.c,(n_col-2));
    fprintf(stderr,"\t0 =< mc.d (%d) =< 3\n",mc.d);
    fclose(fit);
    exit(4);
  }

  
  do {
    if (feof(fit)) break;

    fscanf(fit,"%d %d %d %f\n",&fantasma->f,&fantasma->c,&fantasma->d,&fantasma->r);
    fprintf(stderr, "%d %d %d %f\n",fantasma->f,fantasma->c,fantasma->d,fantasma->r);
    if ((fantasma->f < 1) || (fantasma->f > n_fil1-3) ||
	      (fantasma->c < 1) || (fantasma->c > n_col-2) ||
	      (fantasma->d < 0) || (fantasma->d > 3))
    {
	    fprintf(stderr,"Error: parametres fantasma %d incorrectes:\n", num_fantasma);
	    fprintf(stderr,"\t1 =< fantasmes.f (%d) =< n_fil1-3 (%d)\n",fantasma->f,(n_fil1-3));
	    fprintf(stderr,"\t1 =< fantasmes.c (%d) =< n_col-2 (%d)\n",fantasma->c,(n_col-2));
	    fprintf(stderr,"\t0 =< fantasmes.d (%d) =< 3\n",fantasma->d);
	    fclose(fit);
	    exit(5);
    }
    num_fantasma++;
    // Posicion del siguiente fantasma
    fantasma++;
  } while (num_fantasma < 9);

  fclose(fit);			/* fitxer carregat: tot OK! */
  printf("Joc del MenjaCocos\n\tTecles: \'%c\', \'%c\', \'%c\', \'%c\', RETURN-> sortir\n",
	TEC_AMUNT, TEC_AVALL, TEC_DRETA, TEC_ESQUER);
  printf("prem una tecla per continuar:\n");
  getchar();
}


/* funcio per inicialitar les variables i visualitzar l'estat inicial del joc */
void inicialitza_joc(void)
{
  int r,i,j;
  char strin[12];
  objecte *fantasma = ptr_shmem;

  r = win_carregatauler(tauler,n_fil1-1,n_col,c_req);
  if (r == 0)
  {
    mc.a = win_quincar(mc.f,mc.c);
    if (mc.a == c_req) 
    {
      r = -6;		/* error: menjacocos sobre pared */
    } else 
    {
      
      /* compta el numero total de cocos */
      cocos = 0;
      for (i=0; i<n_fil1-1; i++)
        for (j=0; j<n_col; j++)
          if (win_quincar(i,j)=='.') cocos++;

      // Escriure ComeCocos
      win_escricar(mc.f,mc.c,'0',NO_INV);

      // Menja primer coco
      if (mc.a == '.')
        cocos--;
      
      // Escriure marcador de cocos
      sprintf(strin,"Cocos: %d", cocos); win_escristr(strin);

      for (int k=0; k<num_fantasma; k++){
        fantasma->a = win_quincar(fantasma->f,fantasma->c);
        if (fantasma->a == c_req){
          r = -7;	/* error: fantasma sobre pared */
        } else 
        {
          win_escricar(fantasma->f,fantasma->c,'1'+k,NO_INV); //Escriure fantasmes 
        }
        num_fantasma++;
        fantasma++;
      }

    }
  }
  if (r != 0)
  {	win_fi();
    fprintf(stderr,"Error: no s'ha pogut inicialitzar el joc:\n");
    switch (r)
    { case -1: fprintf(stderr,"  nom de fitxer erroni\n"); break;
      case -2: fprintf(stderr,"  numero de columnes d'alguna fila no coincideix amb l'amplada del tauler de joc\n"); break;
      case -3: fprintf(stderr,"  numero de columnes del laberint incorrecte\n"); break;
      case -4: fprintf(stderr,"  numero de files del laberint incorrecte\n"); break;
      case -5: fprintf(stderr,"  finestra de camp de joc no oberta\n"); break;
      case -6: fprintf(stderr,"  posicio inicial del menjacocos damunt la pared del laberint\n"); break;
      case -7: fprintf(stderr,"  posicio inicial del fantasma damunt la pared del laberint\n"); break;
    }
    exit(7);
  }
}

/* funcio per moure el menjacocos una posicio, en funcio de la direccio de   */
/* moviment actual; retorna -1 si s'ha premut RETURN, 1 si s'ha menjat tots  */
/* els cocos, i 0 altrament */
void* mou_menjacocos(void * null)
{
  int *ptr_fi1 = (int *)(ptr_shmem + OFFSET_FI1);
  int *ptr_fi2 = (int *)(ptr_shmem + OFFSET_FI2);
  
  do {
    char strin[12];
    objecte seg;
    int tec;
    
    tec = win_gettec();
    if (tec != 0){
      switch (tec)		/* modificar direccio menjacocos segons tecla */
      {
        case TEC_AMUNT:	  mc.d = 0; break;
        case TEC_ESQUER:  mc.d = 1; break;
        case TEC_AVALL:	  mc.d = 2; break;
        case TEC_DRETA:	  mc.d = 3; break;
        case TEC_RETURN:  *ptr_fi1 = 1; break;
      }
    }
    seg.f = mc.f + df[mc.d];	/* calcular seguent posicio */
    seg.c = mc.c + dc[mc.d];
    seg.a = win_quincar(seg.f,seg.c);	/* calcular caracter seguent posicio */
    if ((seg.a == ' ') || (seg.a == '.'))
    {
      // esborra posicio anterior
      win_escricar(mc.f,mc.c,' ',NO_INV);
      
      // actualitza posicio
      mc.f = seg.f; mc.c = seg.c;

      // redibuixa menjacocos
      win_escricar(mc.f,mc.c,'0',NO_INV);


      if (seg.a == '.')
      {
        cocos--;
        sprintf(strin,"Cocos: %d", cocos);
        win_escristr(strin);
        if (cocos == 0) *ptr_fi1 = 1;
      }
    }
    win_retard(*ptr_retard);
  } while(!*ptr_fi1 && !*ptr_fi2);
  
  return((void *)((long)*ptr_fi1));
}

/*            programa principal				    */
int main(int n_args, const char *ll_args[])
{
  /* variables locals */
  int rc, id_win, p_win, mida_camp;
  char a1[20],a2[20],a3[20],a4[20],a5[20],a6[20];
  pthread_t thread;
  pid_t fantasmes_id[9];
  srand(getpid());		/* inicialitza numeros aleatoris */
  char str_ptr_shmem[50];

  // Inicializar la memoria compartida
  int id_shmem = ini_mem(3*sizeof(int) + 9*sizeof(objecte));
  ptr_shmem = map_mem(id_shmem);
  if (ptr_shmem == NULL)
  {
    fprintf(stderr, "ERROR: Ptr to shared memory invalid");
    return 1;
  }

  ptr_retard = (int *)(ptr_shmem + OFFSET_RETARD);
  ptr_fantasmes = (objecte *)(ptr_shmem + OFFSET_FANTASMES);

  if ((n_args != 2) && (n_args !=3))
  {	fprintf(stderr,"Comanda: %s fit_param [retard]\n", ll_args[0]);
  	exit(1);
  }
  carrega_parametres(ll_args[1]);

  if (n_args == 3) *ptr_retard = atoi(ll_args[2]);
  else *ptr_retard = 100;

  mida_camp = win_ini(&n_fil,&n_col,'+',INVERS);
  if (mida_camp < 0)
  {			/* si no pot crear l'entorn de joc amb les curses */
    switch (mida_camp)
    {	
      case -1: fprintf(stderr,"camp de joc ja creat!\n"); break;
	    case -2: fprintf(stderr,"no s'ha pogut inicialitzar l'entorn de curses!\n"); break;
	    case -3: fprintf(stderr,"les mides del camp demanades son massa grans!\n"); break;
	    case -4: fprintf(stderr,"no s'ha pogut crear la finestra!\n"); break;
    }
    elim_mem(id_shmem);	/* elimina zona de memoria compartida */
    exit(2);
  }

  // Memoria compartida del campo de juego
  id_win = ini_mem(mida_camp);	/* crear zona mem. compartida */
  p_win = map_mem(id_win);	/* obtenir adres. de mem. compartida */
  win_set(p_win,n_fil,n_col);		/* crea acces a finestra oberta */
  // Esperar a que finalizen los hilos
  int fi1 = pthread_join(thread, NULL);

  inicialitza_joc();
  // Crear el hilo del comecocos
  pthread_create(&thread, NULL, mou_menjacocos, NULL);
  
  sprintf(a2,"%i",id_shmem);
  sprintf(a3,"%i",id_win);
  sprintf(a4,"%i",n_fil);
  sprintf(a5,"%i",n_col);

  // Crear un proceso por cada fantasma
  for (int i = 0; i < num_fantasma; i++) 
  {
    sprintf(a1, "%d", i);
    fantasmes_id[i] = fork();
    if (!fantasmes_id[i]) {
      execlp("./fantasma3.c", "fantasma3.c", a1, a2, a3, a4, a5, NULL);
    }
  }
  
  win_fi();

  if (fi1 < 0) 
    printf("S'ha aturat el joc amb tecla RETURN!\n");
  else if (fi1)
    printf("Ha guanyat l'usuari!\n");
  else
    printf("Ha guanyat l'ordinador!\n"); 
    
  
 
  // Eliminem la zona de memoria compartida
  elim_mem(id_shmem);
  return(0);
}
