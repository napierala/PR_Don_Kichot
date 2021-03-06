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

pthread_mutex_t sem_tids_rycerze = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t sem_test = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t sem_send = PTHREAD_MUTEX_INITIALIZER;

pthread_cond_t con_answer   = PTHREAD_COND_INITIALIZER;

pthread_mutex_t sem_ogarnij = PTHREAD_MUTEX_INITIALIZER;

pthread_cond_t con_ogarnij   = PTHREAD_COND_INITIALIZER;

pthread_mutex_t sem_recv = PTHREAD_MUTEX_INITIALIZER;


pthread_t thread_algorytm,thread_update;;
int  iret;

int tids_rycerze[RYCERZE];

;

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

	pthread_mutex_lock(&sem_send);

	pvm_initsend(PvmDataDefault);
	pvm_pkint(&mytid, 1, 1);

	gettimeofday(&tv,NULL);
	pvm_pklong(&tv.tv_sec,1,1);
	pvm_pklong(&tv.tv_usec,1,1);

	pvm_pkstr(message);	

	pvm_send(tid_master, MSG_DBG);

	pthread_mutex_unlock(&sem_send);

}

void SendMessagetoMasterTyp(char* message,int typ){
	struct timeval tv ;

	pthread_mutex_lock(&sem_send);

	pvm_initsend(PvmDataDefault);
	pvm_pkint(&typ, 1, 1);

	gettimeofday(&tv,NULL);
	pvm_pklong(&tv.tv_sec,1,1);
	pvm_pklong(&tv.tv_usec,1,1);

	pvm_pkstr(message);	

	pvm_send(tid_master, MSG_DBG);

	pthread_mutex_unlock(&sem_send);
}


void sekcja_lokalna(){
	//Czeka tyle ile mial
	struct timeval current_time;


	while(1){
		gettimeofday(&current_time,NULL) ;
		int aktualne_sec = current_time.tv_sec ;

		//To mozemy isc dalej
		if(aktualne_sec >= czas_do_czekania){
			break ;
		}
	}
	
}

void sekcja_krytyczna(){
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
			//Semafor
			pthread_mutex_lock(&sem_tids_rycerze);

				kolejka[i].czas_id = new_czasid;

			pthread_mutex_unlock(&sem_tids_rycerze);

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
			pthread_mutex_lock(&sem_tids_rycerze);
			int czasid = kolejka[i].czas_id ;
			pthread_mutex_unlock(&sem_tids_rycerze);
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
	
	pthread_mutex_lock(&sem_test);
	
	int i ;
	for(i = 0 ; i<RYCERZE ; i++){
		int tid_rycerza = kolejka[i].rycerz ;
		if(tid_rycerza != mytid){

			pthread_mutex_lock(&sem_send);

			pvm_initsend(PvmDataDefault);
			int typ = REQUEST;
			pvm_pkint(&typ,1,1);
			pvm_pkint(&mytid,1,1);
			pvm_pkint(&new_czasid,1,1);
			pvm_send(tid_rycerza,MSG_CZASID);

			pthread_mutex_unlock(&sem_send);

		}
	}
	
	

	SendMessagetoMaster("CZEKAM NA ODP");

	//Czekamy na odpowiedz
	pthread_cond_wait(&con_answer, &sem_test);
	SendMessagetoMaster("Obudzilem sie");

	pthread_mutex_lock(&sem_ogarnij);

	SendMessagetoMaster("Ogarnij Biore glowny");
	
	
	pthread_mutex_unlock(&sem_test);

	SendMessagetoMaster("PO CZEKANIU");
	update_rycerz(mytid,new_czasid);


	pthread_cond_signal(&con_ogarnij);

	pthread_mutex_unlock(&sem_ogarnij);
	
	
	
}

void *Algorytm(void *dummy){
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

void *update_kolejka_thread(void *dummy){
	int tid_rycerza;
	int czasid_rycerza;
	int typ_wiadomosci;	
	int ile_odpowiedzialo = 0 ;
	char wszyscy_odpowiedzieli = 0 ;

	while(1){
		
		//pthread_mutex_lock(&sem_recv);

		SendMessagetoMaster("PRZED MESSAGE");
		//Odbieramy
		pvm_recv(-1,MSG_CZASID);
		pvm_upkint(&typ_wiadomosci,1,1);	
		pvm_upkint(&tid_rycerza,1,1);
		pvm_upkint(&czasid_rycerza,1,1);

		//pthread_mutex_unlock(&sem_recv);

		SendMessagetoMaster("START MESSAGE");

		pthread_mutex_lock(&sem_test);

		if(typ_wiadomosci == CONFIRM){
			SendMessagetoMaster("Przyszla wiadomosc od rycerza do rycerza: CONFIRM");
				ile_odpowiedzialo ++ ;
				if(ile_odpowiedzialo == RYCERZE-1){
					wszyscy_odpowiedzieli = 1 ;
					//SendMessagetoMasterTyp("Wszyscy odpowiedzieli ",wszyscy_odpowiedzieli);	
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
			pthread_mutex_lock(&sem_send);

			pvm_initsend(PvmDataDefault);
			int typ = CONFIRM ;
			pvm_pkint(&typ,1,1);
			pvm_pkint(&tid_rycerza,1,1);
			int moj_czasid = getCzasId(mytid);
			pvm_pkint(&moj_czasid,1,1);
			pvm_send(tid_rycerza, MSG_CZASID);

			pthread_mutex_unlock(&sem_send);

			SendMessagetoMaster("PO WYSLANIU CONFIRM");

		}
		else{
			SendMessagetoMasterTyp("WTF ? Co to za typ_wiadomosci ?",typ_wiadomosci);	
		}

		SendMessagetoMaster("END MESSAGE");
		
		if(wszyscy_odpowiedzieli == 1){
			
			pthread_mutex_lock(&sem_ogarnij);

			SendMessagetoMaster("Budze drugi watek");
			pthread_cond_signal(&con_answer);

			
			//Po co to ?
			//sleep(2);
			
		}

		pthread_mutex_unlock(&sem_test);
		
		if(wszyscy_odpowiedzieli == 1){
			//Koniec Moze isc dalej
			wszyscy_odpowiedzieli = 0 ;
			SendMessagetoMaster("Czekam na ogarniecie");
			pthread_cond_wait(&con_ogarnij,&sem_ogarnij);

			SendMessagetoMaster("Po ogarnieciu");

			pthread_mutex_unlock(&sem_ogarnij);
		}
		//Bez tego nie budzi sie watek ???
		//sleep(2);
		

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
	//Odpalamy watek update_kolejka
	 
	iret = pthread_create(&thread_algorytm, NULL, Algorytm, NULL);
	if(iret){
		SendMessagetoMaster("Watek Algorytm nie dziala");
	}
	iret = pthread_create(&thread_update, NULL, update_kolejka_thread, NULL);
	if(iret){
		SendMessagetoMaster("Watek Update nie dziala");
	}
	
	pthread_join(thread_algorytm, NULL);
	

	pvm_exit();

}
