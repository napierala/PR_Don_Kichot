#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "def.h"
#include "unistd.h"
#define _XOPEN_SOURCE


//Globalne
int mytid;
int tid_master;
int czas_do_czekania ;

int tids_rycerze[RYCERZE];

int ile_odpowiedzialo = 0 ;
char wszyscy_odpowiedzieli = 0 ;

//Kolejnosc Rycerzy
rycerz kolejka[RYCERZE];
void init_kolejka(){
	int i ;	
	for(i = 0 ; i<RYCERZE; i++){
		kolejka[i].czas_id = 0 ;
		kolejka[i].rycerz = tids_rycerze[i];
	}
}



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


void sekcja_lokalna(){
	//Czeka tyle ile mial
	struct timeval current_time;

	struct timeval timeout;
	timeout.tv_sec = 1;
	
	while(1){
		update_kolejka(&timeout);
		gettimeofday(&current_time,NULL) ;
		int aktualne_sec = current_time.tv_sec ;
		
		//To mozemy isc dalej
		if(aktualne_sec >= czas_do_czekania){
			break ;
		}
	}
	
}

void sekcja_krytyczna(){
	struct timeval timeout;
	timeout.tv_sec = 1;
	update_kolejka(&timeout);

	//Walczy z wiatrakiem
	
}

char req_sekcja_krytyczna(){
	//Sprawdza czy nie ma wiadomosci dla niego

	//Sprawdza czy moze wejsc w sekcje kryczyna
		
	return 1 ;
}

int czas_w_sekcji_lokalnej(){
	return rand() % MAX_WAIT ;
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

void update_my_czasid(){
	int czas_sekcji = czas_w_sekcji_lokalnej();
	struct timeval current_time;
	
	gettimeofday(&current_time,NULL) ;

	int new_czasid = current_time.tv_sec + czas_sekcji ;
	int czas_do_czekania = new_czasid ;

	SendMessagetoMasterTyp("CZASID:",czas_do_czekania);

	SendMessagetoMaster("Wszystkim moj czasid");
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
	

	SendMessagetoMaster("CZEKAM NA ODP");

	struct timeval timeout;
	timeout.tv_sec = 1;
	
	wszyscy_odpowiedzieli = 0 ;
	ile_odpowiedzialo = 0 ;

	while(update_kolejka(&timeout) == 0){
		SendMessagetoMaster("Nadal czekam");	
	}
	
	SendMessagetoMaster("PO CZEKANIU");
	update_rycerz(mytid,new_czasid);
	

}

void Algorytm(){
	//Sam Algorytm
	while(1){
		SendMessagetoMaster("Before update_my_czasid();");
		update_my_czasid();
		SendMessagetoMaster("After update_my_czasid();");

		SendMessagetoMaster("Before sekcja_lokalna();");
		sekcja_lokalna();
		SendMessagetoMaster("After sekcja_lokalna();");

		SendMessagetoMaster("Before req_sekcja_krytyczna();");
		while(req_sekcja_krytyczna() == 0);
		SendMessagetoMaster("After req_sekcja_krytyczna();");

		SendMessagetoMaster("Before sekcja_krytyczna();");
		sekcja_krytyczna();
		SendMessagetoMaster("After sekcja_krytyczna();");
	}

}

int update_kolejka(struct timeval *timeout){
	int tid_rycerza;
	int czasid_rycerza;
	int typ_wiadomosci;

		//pthread_mutex_lock(&sem_recv);

		SendMessagetoMaster("PRZED MESSAGE");
		//Odbieramy
		if(pvm_trecv(-1, MSG_CZASID, timeout) > 0){
			//pvm_recv(-1,MSG_CZASID);
			pvm_upkint(&typ_wiadomosci,1,1);	
			pvm_upkint(&tid_rycerza,1,1);
			pvm_upkint(&czasid_rycerza,1,1);

			//pthread_mutex_unlock(&sem_recv);

			SendMessagetoMaster("START MESSAGE");


			if(typ_wiadomosci == CONFIRM){
				SendMessagetoMaster("Przyszla wiadomosc od rycerza do rycerza: CONFIRM");
					ile_odpowiedzialo ++ ;
					if(ile_odpowiedzialo == RYCERZE-1){
						wszyscy_odpowiedzieli = 1 ;
						ile_odpowiedzialo = 0 ;	
						SendMessagetoMaster("WSZYSCY: CONFIRM");		
					}
				//Update
				update_rycerz(tid_rycerza,czasid_rycerza);

			
			}
			else if (typ_wiadomosci == REQUEST){
				SendMessagetoMaster("Przyszla wiadomosc od rycerza do rycerza: REQUEST");		
		
				//Update
				update_rycerz(tid_rycerza,czasid_rycerza);

				SendMessagetoMaster("Po update REQUEST");		
				//Wysylamy

				pvm_initsend(PvmDataDefault);
				int typ = CONFIRM ;
				pvm_pkint(&typ,1,1);
				pvm_pkint(&tid_rycerza,1,1);
				int moj_czasid = getCzasId(mytid);
				pvm_pkint(&moj_czasid,1,1);
				pvm_send(tid_rycerza, MSG_CZASID);

				SendMessagetoMaster("PO WYSLANIU CONFIRM");

			}
			else{
				SendMessagetoMasterTyp("WTF ? Co to za typ_wiadomosci ?",typ_wiadomosci);	
			}

			SendMessagetoMaster("END MESSAGE");
		
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
