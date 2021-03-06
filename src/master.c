#include "def.h"
#include <string.h>
#include <stdlib.h>

int mytid;
int tids[RYCERZE];

int ile_w_sekcji = 0 ;
rycerz kto_w_sekcji[RYCERZE];
char error = 0 ;

void init_kto_w_sekcji(){
	int i ;
	for(i = 0 ;i<RYCERZE ; i++){
		kto_w_sekcji[i].rycerz = tids[i] ;
		kto_w_sekcji[i].czas_id = 0 ;
	}
}

void wchodze_do_sekcji(int tid){
	ile_w_sekcji ++ ;
	int i ;
	for(i = 0 ;i<RYCERZE ; i++){
		if(kto_w_sekcji[i].rycerz == tid){
			kto_w_sekcji[i].czas_id = 1 ;
			break ;
		}
	}
	
}

void wychodze_do_sekcji(int tid){
	ile_w_sekcji -- ;
	int i ;
	for(i = 0 ;i<RYCERZE ; i++){
		if(kto_w_sekcji[i].rycerz == tid){
			kto_w_sekcji[i].czas_id = 0 ;
			break ;
		}
	}
	
}


void wyswietl_stan(){
	if(error == 0){
		system("clear");
	
	
		printf("Walczacych Rycerzy[%d/%d]\n",ile_w_sekcji,RYCERZE);
		printf("RUMAKI[%d/%d]\n",ile_w_sekcji,RUMAKI);
		printf("Miejsce przy Wiatrakach[%d/%d]\n",ile_w_sekcji,MIEJSCA_PRZY_WIATRAKACH);


		//Nie powinno do tego kiedykolwiek dojsc
		if((ile_w_sekcji > RUMAKI) || (ile_w_sekcji > MIEJSCA_PRZY_WIATRAKACH)){
			//Poważny problem
			printf("********************\n");
			printf("********ERROR*******\n");
			printf("********************\n");
			error = 1 ;
		}
		int i ;
		for(i = 0 ;i<RYCERZE ; i++){
			printf("R:[%d]|{%d}\n",kto_w_sekcji[i].rycerz,kto_w_sekcji[i].czas_id);
		}
	}	
}

main()
{
	
	
	char message[MESSAGESIZE+128];	

	long clock_sec,clock_usec;

	int nproc, i, who;

	mytid = pvm_mytid();

	nproc=pvm_spawn(SLAVENAME, NULL, PvmTaskDefault, "", RYCERZE, tids);
	init_kto_w_sekcji();

	//Wysylamy tids do kazdego ze slave'ow	
	for( i=0 ; i<nproc ; i++ )
	{
		pvm_initsend(PvmDataDefault);
		//Moj TiD
		pvm_pkint(&mytid,1,1);
		//Wszyscy Inni	
		pvm_pkint(tids, RYCERZE, 1);
		
		pvm_send(tids[i], MSG_MSTR);
	}
	
	//Informacje od Rycerzy(DEBUG i informacje o stanie)
	while(1)
	{
		pvm_recv( -1, MSG_DBG );
		pvm_upkint(&who, 1, 1 );
		pvm_upklong(&clock_sec,1,1);
		pvm_upklong(&clock_usec,1,1);
		pvm_upkstr(message);
		
		if(strcmp ("IN",message) == 0){
			wchodze_do_sekcji(who);
		}
		else if(strcmp ("OUT",message) == 0) {
			wychodze_do_sekcji(who);
		}
		else{
			printf("%d:[%ld:%ld]{%s}\n",who,clock_sec,clock_usec,message);
		}
			wyswietl_stan();
	}
	
	pvm_exit();
}

