/*
    System specific helpers
*/

#include <windows.h>

#include "../elibc_config.h"
#include "../stdlib/estdlib.h"

#include "esystem.h"

/*----------------------------------------------------------------------*/

/* sleep (in milliseconds) */
void esleep(int sleep_ms)
{
    Sleep(sleep_ms);
}

/*----------------------------------------------------------------------*/
