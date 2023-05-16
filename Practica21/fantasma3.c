
#include <stdlib.h>
#include "winsuport2.h"
#include "menjacocos3.h"

// Funció que controla el moviment fantasma
int main(int argc, char *argv[])
{
  // Arguments del programa:
  // argv[1]: index del fantasma
  // argv[2]: ptr_shmem
  void *ptr_shmem = argv[1];
  int *ptr_fi1 = (int *)(ptr_shmem + OFFSET_FI1);
  int *ptr_fi2 = (int *)(ptr_shmem + OFFSET_FI2);
  int *ptr_retard = (int *)(ptr_shmem + OFFSET_RETARD);
  objecte *ptr_fantasmes = (objecte *)(ptr_shmem + OFFSET_FANTASMES);

  do {
    objecte seg;
    int k, vk, nd, vd[3];

    long index = atoi(*argv);
    nd = 0;

    for (k=-1; k<=1; k++)		/* provar direccio actual i dir. veines */
    {
      vk = (ptr_fantasmes[index].d + k) % 4;		/* direccio veïna */
      if (vk < 0) vk += 4;		/* corregeix negatius */
      
      seg.f = ptr_fantasmes[index].f + df[vk]; /* calcular posicio en la nova dir.*/
      seg.c = ptr_fantasmes[index].c + dc[vk];

      /* calcular caracter seguent posicio */
      seg.a = win_quincar(seg.f,seg.c);
      
      if ((seg.a==' ') || (seg.a=='.') || (seg.a=='0'))
      { vd[nd] = vk;			/* memoritza com a direccio possible */
        nd++;
      }
    }

    if (nd == 0)				/* si no pot continuar, */
      ptr_fantasmes[index].d = (ptr_fantasmes[index].d + 2) % 4;		/* canvia totalment de sentit */
    else
    { 
      if (nd == 1)			/* si nomes pot en una direccio */
        ptr_fantasmes[index].d = vd[0];			/* li assigna aquesta */
      else				/* altrament */
        ptr_fantasmes[index].d = vd[rand() % nd];		/* segueix una dir. aleatoria */

      /* calcular seguent posicio final */
      seg.f = ptr_fantasmes[index].f + df[ptr_fantasmes[index].d];
      seg.c = ptr_fantasmes[index].c + dc[ptr_fantasmes[index].d];

      /* calcular caracter seguent posicio */
      seg.a = win_quincar(seg.f,seg.c);	
      
      /* esborra posicio anterior */
      win_escricar(ptr_fantasmes[index].f,ptr_fantasmes[index].c,ptr_fantasmes[index].a,NO_INV);

      /* actualitza posicio */
      ptr_fantasmes[index].f = seg.f;
      ptr_fantasmes[index].c = seg.c;
      ptr_fantasmes[index].a = seg.a;

      /* redibuixa fantasma */
      win_escricar(ptr_fantasmes[index].f,ptr_fantasmes[index].c,'1'+index,NO_INV);

      /* ha capturat menjacocos */
      if (ptr_fantasmes[index].a == '0') *ptr_fi2 = 1;
    }
    win_retard(2 * *ptr_retard);
  } while (!*ptr_fi1 && !*ptr_fi2);

  return(EXIT_SUCCESS);
}