#include "def.h"
#include "unistd.h"
#define _XOPEN_SOURCE

main(){
	int mytid, tid_master;
	char slave_name[NAMESIZE];
	char current_pass[NAMESIZE];
	char hash[NAMESIZE];
	char current_hash[NAMESIZE];
	char znalazl = 0 ;
	gethostname(slave_name, NAMESIZE+1);

	mytid = pvm_mytid();
		
	int inst = pvm_joingroup(GRP) ;
	
	int tid = pvm_gettid(GRP ,inst);

	int info = pvm_barrier(GRP,SLAVENUM + 1);	

	pvm_recv( -1, MSG_MSTR );
	pvm_upkint(&tid_master, 1, 1 );
	pvm_upkstr(hash);
	//pvm_upkint(&k, 1, 1 );
	
	char poczatek = 97 + inst-1;
	char c1,c2,c3,c4,c5;	
	for(c1 = poczatek ; c1 <= 'z' ; c1++){
		for(c2 = 'a' ; c2 <= 'z' ; c2++){
			for(c3 = 'a' ; c3 <= 'z' ; c3++){
				for(c4 = 'a' ; c4 <= 'z' ; c4++){
					for(c5 = 'a' ; c5 <= 'z' ; c5++){
						current_hash[0] = c1 ;
						current_hash[1] = c2 ;
						current_hash[2] = c3 ;
						current_hash[3] = c4 ;
						current_hash[4] = c5 ;
						current_hash[6] = 0 ;
						current_hash = crypt(current_pass,"aa");
						if(strcmp(current_hash,hash)==0){
							znalazl = 1;
							break;	
						}	
						if(znalazl)
							break ;
					}
					if(znalazl)
						break ;
				}
				if(znalazl)
					break ;
			}
			if(znalazl)
				break ;	
		}
		if(znalazl)
			break ;	
	}
	
	pvm_initsend(PvmDataDefault);
	//pvm_pkint(&k, 1, 1);
	pvm_pkstr(slave_name);
	if(znalazl){
		pvm_pkstr("znalazlem");
	}
	else{
		pvm_pkstr("nie znalazlem");
	}	
	pvm_pkstr(slave_name);
	pvm_send(tid_master, MSG_SLV);


	pvm_exit();




}
