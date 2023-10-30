/*
 *  ELIBC
 */

#ifndef _ELIBC_H_INCLUDED_
#define _ELIBC_H_INCLUDED_

/* allow using in C++ */
#ifdef __cplusplus
extern "C" {
#endif 

/*----------------------------------------------------------------------*/
/* config */
/*----------------------------------------------------------------------*/
#include "elibc_config.h"

/*----------------------------------------------------------------------*/
/* stdlib */
/*----------------------------------------------------------------------*/
#include "stdlib/estdlib.h"

/*----------------------------------------------------------------------*/
/* core */
/*----------------------------------------------------------------------*/
#include "core/ecore_types.h"
#include "core/etrace.h"
#include "core/eassert.h"
#include "core/eerror.h"
#include "core/erandom.h"
#include "core/ebuffer.h"
#include "core/earray.h"
#include "core/estack.h"
#include "core/eset.h"
#include "core/elist.h"
#include "core/ebinsearch.h"
#include "core/efile.h"
#include "core/esystem.h"
#include "core/esort.h"

/*----------------------------------------------------------------------*/
/* performance measurements */
/*----------------------------------------------------------------------*/
#include "perf/eclock.h"
#include "perf/ememuse.h"

/*----------------------------------------------------------------------*/

/* allow using in C++ */
#ifdef __cplusplus
}
#endif 

#endif /* _ELIBC_H_INCLUDED_ */
