#include <bbs.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <iostream>
#include <vector>

#define InputSeed 866
#define InputP  3
#define InputQ  263
#define vector_size 1024

bool stop = false;

void signalHandler( int signum ) {
   std::cout << "Interrupt signal (" << signum << ") received.\n";
   stop = true;
}

int main( int argc, char **argv )
{
	signal(SIGINT, signalHandler);

    // open a connection to the server (fd == coid)
    int fd = open("/dev/cryptobbs", O_RDWR);
    if(fd < 0)
    {
        std::cerr << "E: unable to open server connection: " << strerror(errno ) << std::endl;
        return EXIT_FAILURE;
    }

    bbs::BBSParams param ;
    param.seed = InputSeed;
    param.p = InputP;
    param.q = InputQ;

    std::cout << "debug 1" << std::endl;
    int error;
    if ((error = devctl(fd, GEN_PARAM_INIT , &param, sizeof(param), NULL)) != EOK )
       {
       fprintf(stderr, "Error setting RTS: %s\n",
           strerror ( error ));
           exit(EXIT_FAILURE);
       };

    std::vector<std::uint32_t> buff(vector_size);
    std::uint32_t elem;
    int i = 0;
    std::cout << "debug 2" << std::endl;
    while(!stop)
    {
    	if ((error = devctl(fd, GET_ELEMENT , &elem, sizeof(elem), NULL)) != EOK )
    	       {
    	       fprintf(stderr, "Error setting RTS: %s\n",
    	           strerror ( error ));
    	           exit(EXIT_FAILURE);
    	       };
    	buff[i] = elem;
    	if(i==1023) raise(SIGINT);
    	if(i!=vector_size-1)
    	{
    		++i;
    	}
    	else
    	{
    		i=0;
    	}

    }

    std::cout << "Output vector" << std::endl;
       for (i = 0; i < vector_size; i++) {
           std::cout << buff[i] << std::endl;
       }

    close(fd);

    return EXIT_SUCCESS;
}
