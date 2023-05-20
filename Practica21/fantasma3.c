#include <stdlib.h>
#include <stdio.h>
#include "memoria.h"
#include "winsuport2.h"
#include "menjacocos3.h"

// Funció que controla el moviment fantasma
int main(int argc, char *argv[])
{
  // Arguments del programa:
  // argv[1]: index del fantasma
  // argv[2]: id_shmem_ipc
  // argv[3]: id_shmem_field
  // argv[4]: n_fil
  // argv[5]: n_col
  // argv[6]: retard

  int index = atoi(argv[1]);
  int id_shmem_ipc = atoi(argv[2]);
  int id_shmem_field = atoi(argv[3]);
  int n_fil = atoi(argv[4]);
  int n_col = atoi(argv[5]);
  int retard = atoi(argv[6]);

  // Map the field shared memory
  void * ptr_shmem_field = map_mem(id_shmem_field);
  win_set(ptr_shmem_field, n_fil, n_col);

  // Map the ipc shared memory  
  void * ptr_shmem_ipc = map_mem(id_shmem_ipc);
  // Pointer to the shared memory location of `fi1`
  int *ptr_sh_fi1 = (int *)(ptr_shmem_ipc + OFFSET_FI1);
  // Pointer to the shared memory location of `fi2`
  int *ptr_sh_fi2 = (int *)(ptr_shmem_ipc + OFFSET_FI2);
  // Pointer to the shared memory location of `fantasmes` 
  objecte *ptr_sh_fantasmes = (objecte *)(ptr_shmem_ipc + OFFSET_FANTASMES);

  // Main loop
  do {
    objecte seg;
    objecte *fantasma = ptr_sh_fantasmes + index;
    int k, vk, nd, vd[3];

    long index = atoi(*argv);
    nd = 0;

    for (k=-1; k<=1; k++)		/* provar direccio actual i dir. veines */
    {
      vk = (fantasma->d + k) % 4;	/* direccio veïna */
      if (vk < 0) vk += 4;		/* corregeix negatius */
      
      seg.f = fantasma->f + df[vk];	/* calcular posicio en la nova dir.*/
      seg.c = fantasma->c + dc[vk];

      /* calcular caracter seguent posicio */
      seg.a = win_quincar(seg.f,seg.c);
      
      if ((seg.a==' ') || (seg.a=='.') || (seg.a=='0'))
      { vd[nd] = vk;			/* memoritza com a direccio possible */
        nd++;
      }
    }

    if (nd == 0)				/* si no pot continuar, */
      fantasma->d = (fantasma->d + 2) % 4;	/* canvia totalment de sentit */
    else
    { 
      if (nd == 1)			/* si nomes pot en una direccio */
        fantasma->d = vd[0];			/* li assigna aquesta */
      else				/* altrament */
        fantasma->d = vd[rand() % nd];		/* segueix una dir. aleatoria */

      /* calcular seguent posicio final */
      seg.f = fantasma->f + df[fantasma->d];
      seg.c = fantasma->c + dc[fantasma->d];

      /* calcular caracter seguent posicio */
      seg.a = win_quincar(seg.f,seg.c);	
      
      /* esborra posicio anterior */
      win_escricar(fantasma->f,fantasma->c,fantasma->a,NO_INV);

      /* actualitza posicio */
      fantasma->f = seg.f;
      fantasma->c = seg.c;
      fantasma->a = seg.a;

      /* redibuixa fantasma */
      win_escricar(fantasma->f,fantasma->c,'1'+index,NO_INV);

      /* ha capturat menjacocos */
      if (fantasma->a == '0') *ptr_sh_fi2 = 1;
    }
    win_retard(2 * retard);
  } while (!*ptr_sh_fi1 && !*ptr_sh_fi2);

  return(EXIT_SUCCESS);
}
