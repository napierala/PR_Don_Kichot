#include <stdio.h>
#include <stdlib.h>
#include <pvm3.h>

#define SLAVENAME "slave"

#define MESSAGESIZE 64

#define MSG_MSTR 1
#define MSG_DBG  2
#define MSG_CZASID 3

#define REQUEST 10
#define CONFIRM 11

#define GRP "grupa"

#define RYCERZE 6
#define WIATRAKI 1
#define MIEJSCA_PRZY_WIATRAKACH 4
#define RUMAKI 5

//W sec
#define MAX_WAIT 10

typedef struct RYCERZ{
	//Czas id (Po Tym sortowane)
	int czas_id;
	//Rycerz(Tid)
	int rycerz;
}rycerz; 
