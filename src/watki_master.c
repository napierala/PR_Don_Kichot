#include "def.h"

main()
{
	int mytid;
	int tids[RYCERZE];
	
	char message[MESSAGESIZE+128];	

	long clock_sec,clock_usec;

	int nproc, i, who;

	mytid = pvm_mytid();

	nproc=pvm_spawn(SLAVENAME, NULL, PvmTaskDefault, "", RYCERZE, tids);
	
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
		
		printf("%d:[%ld:%ld]{%s}\n",who,clock_sec,clock_usec,message);
	}
	
	pvm_exit();
}

