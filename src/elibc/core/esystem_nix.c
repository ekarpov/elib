/*
    System specific helpers
*/

#include <unistd.h>

#include "../elibc_config.h"
#include "../stdlib/estdlib.h"

#include "esystem.h"

/*----------------------------------------------------------------------*/

/* sleep (in milliseconds) */
void esleep(int sleep_ms)
{
    /* usleep takes sleep time in us (1 millionth of a second) */
    usleep(sleep_ms * 1000);   
}

/*----------------------------------------------------------------------*/
