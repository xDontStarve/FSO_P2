#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <pthread.h>    /* threads library */
#include "memoria.h"
#include "winsuport2.h"
#include "menjacocos4.h"
#include "missatge.h"
#include "semafor.h"

#define MIN(x, y) (((x) < (y)) ? (x) : (y))

int *ptr_sh_busties;
int index_fantasma;

void * handle_messages(void * args) {
  char * message;
  receiveM(*(ptr_sh_busties + index_fantasma), &message);

  
  return (void *) 1;
}

int decide_move(int curr_f, int curr_c, int target_f, int target_c, int valid_directions[3], int num_directions)
{
  // Direction decided
  int decision;
  double min_distance = __DBL_MAX__; 

  // No target, select a random position
  if (target_f == -1 && target_c == -1)
    return valid_directions[rand() % num_directions];

  // If no decision is needed
  if (num_directions == 0)
    return -1;
  else if (num_directions == 1)
    return valid_directions[0];

  // For each valid direction
  for (int i = 0; i < num_directions; i++) {
    
    // Compute next position
    int x1 = curr_c + dc[valid_directions[i]];
    int y1 = curr_f + df[valid_directions[i]];
    
    int x2 = target_c, y2 = target_f;
    // Compute distance between next position and target
    double distance_to_target = sqrt( (x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1) );
    // Save the direction with the minimal distance
    if (distance_to_target < min_distance)
    { decision = valid_directions[i];
      min_distance = distance_to_target;
    }
  }
  // Return the best direction
  return decision;
}

// Funció que controla el moviment fantasma
int main(int argc, char *argv[])
{
  // Arguments del programa:
  // argv[1]: index del fantasma
  index_fantasma = atoi(argv[1]);
  // argv[2]: id_shmem_ipc
  int id_shmem_ipc = atoi(argv[2]);
  // argv[3]: id_shmem_field
  int id_shmem_field = atoi(argv[3]);
  // argv[4]: n_fil
  int n_fil = atoi(argv[4]);
  // argv[5]: n_col
  int n_col = atoi(argv[5]);
  // argv[6]: retard
  int retard = atoi(argv[6]);
  // argv[7]: id_sem_field
  int id_sem_field = atoi(argv[7]);
  // argv[8]: wall_symbol
  char wall_symbol = argv[8][0];
  
  // Thread that handles the messages
  pthread_t thread_busties;
  // Variables regarding the mode of the `fantasma`
  char valid_moves[5] = {COCO_SYMBOL, AIR_SYMBOL, MENJACOCOS_SYMBOL, INVALID_MOVE, '\0'};
  unsigned int invert = NO_INV;
  // Target Position
  int target_f = -1;
  int target_c = -1;
  // Array of symbols that will stop the vision of the `fantasma`
  char translucent_symbols[MAX_TRANSLUCENT_SYMBOLS+1] = {COCO_SYMBOL, AIR_SYMBOL, '\0'};

  // Map the field shared memory
  void * ptr_shmem_field = map_mem(id_shmem_field);
  win_set(ptr_shmem_field, n_fil, n_col);

  // Map the ipc shared memory  
  void * ptr_shmem_ipc = map_mem(id_shmem_ipc);
  // Pointer to the shared memory location of `fi1`
  int *ptr_sh_fi1 = (int *)(ptr_shmem_ipc + OFFSET_FI1);
  // Pointer to the shared memory location of `fi2`
  int *ptr_sh_fi2 = (int *)(ptr_shmem_ipc + OFFSET_FI2);
  // Pointer to the shared memory location of `busties` 
  ptr_sh_busties = (int *)(ptr_shmem_ipc + OFFSET_BUSTIES);
  // Pointer to the shared memory location of this process `bustia`
  int *ptr_sh_personal_bustia = ptr_sh_busties + index_fantasma;
  // Pointer to the shared memory location of `fantasmes` 
  objecte *ptr_sh_fantasmes = (objecte *)(ptr_shmem_ipc + OFFSET_FANTASMES);


  *ptr_sh_personal_bustia = ini_mis();
  // Thread that receives the messages of the other `fantasmes`
  pthread_create(&thread_busties, NULL, handle_messages, NULL);


  // Main loop
  do {
    objecte seg;
    objecte *fantasma = ptr_sh_fantasmes + index_fantasma;
    int k, vk, nd, vd[3];
    nd = 0;
    
    // Scan forward
    
    seg.f = fantasma->f;
    seg.c = fantasma->c;
    seg.a = AIR_SYMBOL;
    while(strchr(translucent_symbols, seg.a) != NULL)
    { // Compute next position with the current direction
      seg.f += df[fantasma->d];
      seg.c += dc[fantasma->d];
      // Compute next character with the current direction
      seg.a = win_quincar(seg.f,seg.c);
    }

    // If the scan stopped in the `menjacocos`
    if (seg.a == MENJACOCOS_SYMBOL)
    { // Enter hunt mode
      valid_moves[3] = wall_symbol;
      // Invert the rendering
      invert = INVERS;
      // Save the `menjacocos` position
      target_f = seg.f; target_c = seg.c;
      // TODO: Message the rest of the `fantasmes` with the `menjacocos` position
    }
    

    

    for (k=-1; k<=1; k++)		/* provar direccio actual i dir. veines */
    {
      vk = (fantasma->d + k) % 4;	/* direccio veïna */
      if (vk < 0) vk += 4;		/* corregeix negatius */
      
      /* calcular posicio en la nova dir.*/
      seg.f = fantasma->f + df[vk];
      seg.c = fantasma->c + dc[vk];

      /* calcular caracter seguent posicio */
      seg.a = win_quincar(seg.f,seg.c);
     
      // If the symbol of the next character is found in the array of valid moves
      if (strchr(valid_moves, seg.a) != NULL)
        vd[nd++] = vk;			// memoritza com a direccio possible
      
    }
    
    // Turn around if there are no valid directions 
    if (nd == 0) {
      fantasma->d = (fantasma->d + 2) % 4;
    } else 
    { // Decide which direction to take
      fantasma->d = decide_move(fantasma->f, fantasma->c, target_f, target_c, vd, nd);
  
      /* calcular seguent posicio final */
      seg.f = fantasma->f + df[fantasma->d];
      seg.c = fantasma->c + dc[fantasma->d];

      // Start of a critical section
      waitS(id_sem_field);

      /* calcular caracter seguent posicio */
      seg.a = win_quincar(seg.f,seg.c);
      
      /* esborra posicio anterior */
      win_escricar(fantasma->f,fantasma->c,fantasma->a,fantasma->a == '+');
      

      /* actualitza posicio */
      fantasma->f = seg.f;
      fantasma->c = seg.c;
      fantasma->a = seg.a;

      /* redibuixa fantasma */
      win_escricar(fantasma->f,fantasma->c,'1'+index_fantasma,invert);

      // End of the critical section
      signalS(id_sem_field);

      /* ha capturat menjacocos */
      if (fantasma->a == '0') *ptr_sh_fi2 = 1;
    }
    win_retard(retard * fantasma->r);
  } while (!*ptr_sh_fi1 && !*ptr_sh_fi2);
  
  char message[1] = {0};
  sendM(*(ptr_sh_busties + index_fantasma), message, 1);
  pthread_join(thread_busties, NULL);
  
  elim_mis(*ptr_sh_personal_bustia);

  elim_mem(id_shmem_ipc);
  elim_mem(id_shmem_field);
  return(EXIT_SUCCESS);
}
