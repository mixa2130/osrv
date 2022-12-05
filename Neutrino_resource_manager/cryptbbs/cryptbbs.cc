#include <bbs.h>
#include <iostream>
#include <errno.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/iofunc.h>
#include <sys/dispatch.h>
#include <devctl.h>
#include <string.h>
#include <sys/neutrino.h>


static resmgr_connect_funcs_t    connect_funcs;
static resmgr_io_funcs_t         io_funcs;
static iofunc_attr_t             attr;

struct Params{
	
	std::uint32_t LastElement = 0;	
	bbs::BBSParams Par;
	
};


std::map <std::int32_t, Params*> contexts;


std::uint32_t getElement(std::uint32_t cId){

	std::uint32_t CyphElement = 0,NewElement = 0;
	bool bit = false;
	std::uint32_t place= 1;

	for(int i=0; i<sizeof(std::uint32_t)*CHAR_BIT ; i++)
	{
		NewElement = contexts[cID]->LastElement*contexts[cID]->LastElement % (contexts[cID]->Par.p*contexts[cID]->Par.q);
		bit = NewElement % 2;
		CyphElement +=  place*bit;
		contexts[cID]->LastElement = NewElement;
		place = place*2;

	}

	return CyphElement;

}

int io_open (resmgr_context_t * ctp , io_open_t * msg , RESMGR_HANDLE_T * handle , void * extra )
{
	Params *c ;
	contexts[ctp->info.scoid] = c;
	return (iofunc_open_default (ctp, msg, handle, extra));
}

int io_close(resmgr_context_t *ctp, io_close_t *msg, iofunc_ocb_t *ocb)
{
	context.deleteContext(ctp->info.scoid);
	return (iofunc_close_dup_default(ctp, msg, ocb));
}

int io_devctl(resmgr_context_t *ctp, io_devctl_t *msg,

 iofunc_ocb_t *ocb) {

	int  sts;

	void *data;

	int  nbytes;

	if ((sts = iofunc_devctl_default(ctp, msg, ocb)) !=_RESMGR_DEFAULT) {

		return (sts);

	}

	std::int32_t cId = ctp->info.scoid;
	
	data = _DEVCTL_DATA(msg->i);

	nbytes = 0;

	switch (msg->i.dcmd) {


		case GEN_PARAM_INIT:

		contexts[cId]->Par = reinterpret_cast<bbs::BBSParams*> (data);
		contexts[cId]->LastElement= contexts[cId]->Par.seed;

	break;

		case GET_ELEMENT:

		*(std::uint32_t*)data = getElement(cId);

		nbytes = sizeof(std::uint32_t);

	break;

 };


	memset(&(msg->o), 0, sizeof(msg->o));

	msg->o.nbytes = nbytes;

	SETIOV(ctp->iov, &msg->o, sizeof(msg->o) + nbytes);

	return (_RESMGR_NPARTS(1));

}



int main(int argc, char **argv)
{
    /* declare variables we'll be using */
    resmgr_attr_t        resmgr_attr;
    dispatch_t           *dpp;
    dispatch_context_t   *ctp;
    int                  id;

    /* initialize dispatch interface */
    if((dpp = dispatch_create()) == NULL) {
        fprintf(stderr,
                "%s: Unable to allocate dispatch handle.\n",
                argv[0]);
        return EXIT_FAILURE;
    }

    /* initialize resource manager attributes */
    memset(&resmgr_attr, 0, sizeof resmgr_attr);
    resmgr_attr.nparts_max = 1;
    resmgr_attr.msg_max_size = 2048;

    /* initialize functions for handling messages */
    iofunc_func_init(_RESMGR_CONNECT_NFUNCS, &connect_funcs,
                     _RESMGR_IO_NFUNCS, &io_funcs);
    io_funcs.devctl = io_devctl;
	connect_funcs.open 	= io_open;
	io_funcs.close_dup = io_close;
    /* initialize attribute structure used by the device */
    iofunc_attr_init(&attr, S_IFNAM | 0666, 0, 0);

    /* attach our device name */
    id = resmgr_attach(
            dpp,            /* dispatch handle        */
            &resmgr_attr,   /* resource manager attrs */
            "/dev/cryptobbs",  /* device name            */
            _FTYPE_ANY,     /* open type              */
            0,              /* flags                  */
            &connect_funcs, /* connect routines       */
            &io_funcs,      /* I/O routines           */
            &attr);         /* handle                 */
    if(id == -1) {
        fprintf(stderr, "%s: Unable to attach name.\n", argv[0]);
        return EXIT_FAILURE;
    }

    /* инициализация атрибутов пула потоков */
        memset(&pool_attr, 0, sizeof pool_attr);
        pool_attr.handle = dpp;
        pool_attr.context_alloc = dispatch_context_alloc;
        pool_attr.block_func = dispatch_block;
        pool_attr.unblock_func = dispatch_unblock;
        pool_attr.handler_func = dispatch_handler;
        pool_attr.context_free = dispatch_context_free;
        pool_attr.lo_water = 2;
        pool_attr.hi_water = 4;
        pool_attr.increment = 1;
        pool_attr.maximum = 50;

	/* инициализация пула потоков */
		if((tpp = thread_pool_create(&pool_attr,
									 POOL_FLAG_EXIT_SELF)) == NULL) {
			fprintf(stderr, "%s: Unable to initialize thread pool.\n",
					argv[0]);
			return EXIT_FAILURE;
		}

		/* запустить потоки, блокирующая функция */
		thread_pool_start(tpp);
		
    return EXIT_SUCCESS; // never go here
}
