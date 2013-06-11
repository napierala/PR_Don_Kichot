#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "def.h"
#include "unistd.h"
#define _XOPEN_SOURCE


/***************GLOBALNE***************/
int mytid;
int tid_master;
int czas_do_czekania ;

int tids_rycerze[RYCERZE];

int ile_odpowiedzialo = 0 ;
char wszyscy_odpowiedzieli = 0 ;

struct timeval timeout;

/***************RYCERZE****************/
//Kolejnosc Rycerzy
rycerz kolejka[RYCERZE];
void init_kolejka(){
	int i ;	
	for(i = 0 ; i<RYCERZE; i++){
		kolejka[i].czas_id = 0 ;
		kolejka[i].rycerz = tids_rycerze[i];
	}
}

void sort_kolejka(){
	int i, j;
	rycerz temp;
 
  	for (i = (RYCERZE - 1); i > 0; i--){
    		for (j = 1; j <= i; j++){
      			if (kolejka[j-1].czas_id > kolejka[j].czas_id){
				temp.czas_id = kolejka[j-1].czas_id;
				temp.rycerz = kolejka[j-1].rycerz;

				kolejka[j-1].czas_id = kolejka[j].czas_id;
				kolejka[j-1].rycerz = kolejka[j].rycerz;

				kolejka[j].czas_id = temp.czas_id;
				kolejka[j].rycerz = temp.rycerz;
      			}
			else if(kolejka[j-1].czas_id == kolejka[j].czas_id){
	
				if (kolejka[j-1].rycerz > kolejka[j].rycerz){

					temp.czas_id = kolejka[j-1].czas_id;
					temp.rycerz = kolejka[j-1].rycerz;

					kolejka[j-1].czas_id = kolejka[j].czas_id;
					kolejka[j-1].rycerz = kolejka[j].rycerz;

					kolejka[j].czas_id = temp.czas_id;
					kolejka[j].rycerz = temp.rycerz;
      				}	
			}
    		}
 	}
}

void update_rycerz(int tid,int new_czasid){
	//SendMessagetoMaster("update_rycerz");
	int i ;
	for(i = 0 ; i<RYCERZE ;i++){
		if(kolejka[i].rycerz == tid){

			kolejka[i].czas_id = new_czasid;
			break ;
		}
	}
	//SendMessagetoMaster("PO update_rycerz");
	sort_kolejka();
}

int getCzasId(int tid){
	//SendMessagetoMaster("getCzasId");
	int i ;
	for(i = 0 ; i<RYCERZE ;i++){
		if(kolejka[i].rycerz == tid){
			
			int czasid = kolejka[i].czas_id ;
			//SendMessagetoMaster("PO getCzasId");
			return czasid;
			
		}
	}
	//SendMessagetoMaster("PO getCzasId");	
	return 0 ;
}

int getPos(int tid){
	int i ;
	for(i = 0 ; i<RYCERZE ;i++){
		if(kolejka[i].rycerz == tid){
			return i ;	
		}
	}
	//SendMessagetoMaster("PO getCzasId");	
	return RYCERZE ;
}
/***************DO MASTER**************/
void SendMessagetoMaster(char* message){
	struct timeval tv ;

	pvm_initsend(PvmDataDefault);
	pvm_pkint(&mytid, 1, 1);

	gettimeofday(&tv,NULL);
	pvm_pklong(&tv.tv_sec,1,1);
	pvm_pklong(&tv.tv_usec,1,1);

	pvm_pkstr(message);	

	pvm_send(tid_master, MSG_DBG);

}

void SendMessagetoMasterTyp(char* message,int typ){
	struct timeval tv ;

	pvm_initsend(PvmDataDefault);
	pvm_pkint(&typ, 1, 1);

	gettimeofday(&tv,NULL);
	pvm_pklong(&tv.tv_sec,1,1);
	pvm_pklong(&tv.tv_usec,1,1);

	pvm_pkstr(message);	

	pvm_send(tid_master, MSG_DBG);

}

/***************ALGORYTM***************/

int update_kolejka(){
	int tid_rycerza;
	int czasid_rycerza;
	int typ_wiadomosci;

		//pthread_mutex_lock(&sem_recv);

		//SendMessagetoMaster("PRZED MESSAGE");
		//Odbieramy
		if(pvm_trecv(-1, MSG_CZASID, &timeout) > 0){
			//pvm_recv(-1,MSG_CZASID);
			pvm_upkint(&typ_wiadomosci,1,1);	
			pvm_upkint(&tid_rycerza,1,1);
			pvm_upkint(&czasid_rycerza,1,1);

			//pthread_mutex_unlock(&sem_recv);

			//SendMessagetoMaster("START MESSAGE");


			if(typ_wiadomosci == CONFIRM){
				//SendMessagetoMaster("Przyszla wiadomosc od rycerza do rycerza: CONFIRM");
					ile_odpowiedzialo ++ ;
					if(ile_odpowiedzialo == RYCERZE-1){
						wszyscy_odpowiedzieli = 1 ;
						ile_odpowiedzialo = 0 ;	
						//SendMessagetoMaster("WSZYSCY: CONFIRM");		
					}
				//Update
				update_rycerz(tid_rycerza,czasid_rycerza);

			
			}
			else if (typ_wiadomosci == REQUEST){
				//SendMessagetoMaster("Przyszla wiadomosc od rycerza do rycerza: REQUEST");		
		
				//Update
				update_rycerz(tid_rycerza,czasid_rycerza);

				//SendMessagetoMaster("Po update REQUEST");		
				//Wysylamy

				pvm_initsend(PvmDataDefault);
				int typ = CONFIRM ;
				pvm_pkint(&typ,1,1);
				pvm_pkint(&tid_rycerza,1,1);
				int moj_czasid = getCzasId(mytid);
				pvm_pkint(&moj_czasid,1,1);
				pvm_send(tid_rycerza, MSG_CZASID);

				//SendMessagetoMaster("PO WYSLANIU CONFIRM");

			}
			else{
				SendMessagetoMasterTyp("WTF ? Co to za typ_wiadomosci ?",typ_wiadomosci);	
			}

			//SendMessagetoMaster("END MESSAGE");
		
			if(wszyscy_odpowiedzieli == 1){
				return 1 ;	
			}else{
				return 0 ;
			}
		}
		else{
			return 0 ;		
		}
		
}

int czas_w_sekcji(){
	srand (time(NULL));
	return rand() % MAX_WAIT ;
}

int bezpieczny_czas_id(int czas_id){
	//Czas id Musi byc wiekszy lub rowny najwiekszemu dotychczasowemu czas id
	//Ostatnia pozycja w kolejce to najgorszy czas_id
	rycerz max_rycerz = kolejka[RYCERZE-1];
	//Jak to nie ja
	if(max_rycerz.rycerz != mytid){
		return (max_rycerz.czas_id + czas_w_sekcji());
	}
	else{
		return (kolejka[RYCERZE-2].czas_id + czas_w_sekcji());
	}
}

void sekcja_lokalna(){
	//Czeka tyle ile mial
	struct timeval current_time;

	timeout.tv_sec = 1;
	
	while(1){
		update_kolejka();
		gettimeofday(&current_time,NULL) ;
		int aktualne_sec = current_time.tv_sec ;
		
		//To mozemy isc dalej
		if(aktualne_sec >= czas_do_czekania){
			break ;
		}
	}
	
}

void sekcja_krytyczna(){
	struct timeval current_time;
	timeout.tv_sec = 1;
	
	gettimeofday(&current_time,NULL) ;
	int czas_sekcji = current_time.tv_sec + czas_w_sekcji();
	
	//Walczy z wiatrakiem
	while(1){
		update_kolejka();
		gettimeofday(&current_time,NULL) ;
		int aktualne_sec = current_time.tv_sec ;
		//To mozemy isc dalej
		if(aktualne_sec >= czas_sekcji){
			break ;
		}
	}
	
}

char req_sekcja_krytyczna(){

	timeout.tv_sec = 1;
	//Sprawdza czy nie ma wiadomosci dla niego
	update_kolejka(&timeout);

	//Sprawdza czy moze wejsc w sekcje kryczyna
	if(getPos(mytid) <= RUMAKI-1){
		if(getPos(mytid) <= MIEJSCA_PRZY_WIATRAKACH-1)
			return 1 ;
		
		return 0 ;
	}
	return 0 ;
}

void update_my_czasid(){
	int czas_sekcji = czas_w_sekcji();
	struct timeval current_time;
	
	gettimeofday(&current_time,NULL) ;

	int new_czasid = current_time.tv_sec + czas_sekcji ;
	czas_do_czekania = bezpieczny_czas_id(new_czasid);

	//SendMessagetoMasterTyp("CZASID:",czas_do_czekania);

	//SendMessagetoMaster("Wszystkim moj czasid");
	//Wysylamy wszystkim nasz czasid
	
	int i ;
	for(i = 0 ; i<RYCERZE ; i++){
		int tid_rycerza = kolejka[i].rycerz ;
		if(tid_rycerza != mytid){

			pvm_initsend(PvmDataDefault);
			int typ = REQUEST;
			pvm_pkint(&typ,1,1);
			pvm_pkint(&mytid,1,1);
			pvm_pkint(&new_czasid,1,1);
			pvm_send(tid_rycerza,MSG_CZASID);

		}
	}
	

	//SendMessagetoMaster("CZEKAM NA ODP");

	struct timeval timeout;
	timeout.tv_sec = 1;
	
	wszyscy_odpowiedzieli = 0 ;
	ile_odpowiedzialo = 0 ;

	while(update_kolejka(&timeout) == 0){
		//SendMessagetoMaster("Nadal czekam");	
	}
	
	//SendMessagetoMaster("PO CZEKANIU");
	update_rycerz(mytid,new_czasid);
	

}

void Algorytm(){
	//Sam Algorytm
	while(1){
		//SendMessagetoMaster("Before update_my_czasid();");
		update_my_czasid();
		//SendMessagetoMaster("After update_my_czasid();");

		//SendMessagetoMaster("Before sekcja_lokalna();");
		sekcja_lokalna();
		//SendMessagetoMaster("After sekcja_lokalna();");

		//SendMessagetoMaster("Before req_sekcja_krytyczna();");
		while(req_sekcja_krytyczna() == 0);
		//SendMessagetoMaster("After req_sekcja_krytyczna();");

		SendMessagetoMaster("IN");
		sekcja_krytyczna();
		//SendMessagetoMaster("After sekcja_krytyczna();");
		SendMessagetoMaster("OUT");
	}

}

main(){
	long clock_sec, clock_usec ;
	struct timeval tv ;

	mytid = pvm_mytid();
	

	pvm_recv( -1, MSG_MSTR);
	//Dostajemy Tids wszystkich
	pvm_upkint(&tid_master,1,1);
	pvm_upkint(tids_rycerze, RYCERZE, 1 );
	
	init_kolejka();

	SendMessagetoMaster("Zaczynamy");
	 
	Algorytm();
	

	pvm_exit();

}
