/* p3helper.c
 * Armando Perez
 * Program 3
 * CS570
 * John Carroll
 * SDSU
 * Program 3 is due on Nov. 7th (2nd is extra credit) at 11 PM.
 *
 *
 */
#include "p3.h"

/* You may put any semaphore (or other global) declarations/definitions here: */

sem_t jog_sem, shoot_sem, allow_jog, allow_shoot;
int shootc = 0, jogc = 0;



/* General documentation for the following functions is in p3.h
 * Here you supply the code:
 */
void initstudentstuff(void) {

  CHK(sem_init(&jog_sem, LOCAL, 1));
  CHK(sem_init(&shoot_sem, LOCAL, 1));
  CHK(sem_init(&allow_jog, LOCAL, 1));
  CHK(sem_init(&allow_shoot, LOCAL, 1));

}

void prolog(int kind) {

  if(kind == JOGGER) {
    CHK(sem_wait(&allow_jog));
    CHK(sem_wait(&jog_sem));

    jogc++;
    if(jogc == 1) {
      CHK(sem_wait(&allow_shoot));
    }
      CHK(sem_post(&jog_sem));
      CHK(sem_post(&allow_jog));

  }
  else if (kind == SHOOTER) {
    CHK(sem_wait(&allow_shoot));
    CHK(sem_wait(&shoot_sem));

    shootc++;
    if(shootc == 1) {
      CHK(sem_wait(&allow_jog));
    }
      CHK(sem_post(&shoot_sem));
      CHK(sem_post(&allow_shoot));

  }
}

void epilog(int kind) {

  if(kind == JOGGER) {
    CHK(sem_wait(&jog_sem));
    jogc--;

    if(jogc == 0) {
      CHK(sem_post(&allow_shoot));
    }

    CHK(sem_post(&jog_sem));

  }
  else if(kind == SHOOTER) {

    CHK(sem_wait(&shoot_sem));
    shootc--;

    if(shootc == 0) {
      CHK(sem_post(&allow_jog));
    }

    CHK(sem_post(&shoot_sem));

  }
}  
