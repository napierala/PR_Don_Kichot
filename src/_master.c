#include "def.h"

main()
{
	int mytid;
	int tids[SLAVENUM];		/* slave task ids */
	char slave_name[NAMESIZE];
	char slave_pass[NAMESIZE];
	char hash[NAMESIZE];
	int nproc, i, who;

	mytid = pvm_mytid();

	int inst = pvm_joingroup(GRP);
	printf("%d\n",inst);
	nproc=pvm_spawn(SLAVENAME, NULL, PvmTaskDefault, "", SLAVENUM, tids);
	
	int info = pvm_barrier(GRP,SLAVENUM + 1);	

	pvm_initsend(PvmDataDefault);
	pvm_pkint(&mytid, 1, 1);
	strcpy(hash,"aa7uckMQPpN46");
	pvm_pkstr(hash);

	pvm_bcast(GRP,MSG_MSTR);

	/*
	for( i=0 ; i<nproc ; i++ )
	{
		pvm_initsend(PvmDataDefault);
		pvm_pkint(&mytid, 1, 1);
		pvm_pkint(&i, 1, 1);
		pvm_send(tids[i], MSG_MSTR);
	}
	*/
	for( i=0 ; i<nproc ; i++ )
	{
		pvm_recv( -1, MSG_SLV );
		//pvm_upkint(&who, 1, 1 );
		pvm_upkstr(slave_name );
		printf("%d: %s\n",who, slave_name);
	}
	pvm_lvgroup(GRP);
	pvm_exit();
}

