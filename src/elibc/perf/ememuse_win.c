/*
    Memory usage tools for Windows
*/

#include "../elibc_config.h"

/* check if enabled */
#ifdef _ELIBC_ENABLE_PERFORMANCE_TOOLS

#include <windows.h>
#include "ememuse.h"

#include <crtdbg.h>

/*----------------------------------------------------------------------*/
/* constants */
#define _ELIBC_MAX_HEAP_COUNT       64

/*----------------------------------------------------------------------*/
/* memory usage */
size_t elibc_get_heap_memory_usage(HANDLE hHeap)
{
    PROCESS_HEAP_ENTRY heapEntry;
    size_t uTotalSize = 0;

    /* init process heap structure */
    ZeroMemory(&heapEntry, sizeof(heapEntry));

    /* protect heap */
    HeapLock(hHeap);

    /* count heap usage */
    while(HeapWalk(hHeap, &heapEntry))
    {
        /* update size */
        /* if(heapEntry.wFlags == PROCESS_HEAP_ENTRY_BUSY) */
        {
            uTotalSize += heapEntry.cbData;
        }
    }

    /* release heap */
    HeapUnlock(hHeap);

    return uTotalSize;
}

size_t elibc_get_memory_usage()
{
    HANDLE pHeaps[_ELIBC_MAX_HEAP_COUNT];
    size_t uTotalSize = 0;
    DWORD dwHeapIdx;

    /* get process heaps */
    DWORD dwHeapCount = GetProcessHeaps(_ELIBC_MAX_HEAP_COUNT, pHeaps);

    /* count total memory usage */
    for(dwHeapIdx = 0; dwHeapIdx < dwHeapCount; ++dwHeapIdx)
    {
        uTotalSize += elibc_get_heap_memory_usage(pHeaps[dwHeapIdx]);
    }

    return uTotalSize;
}

/*----------------------------------------------------------------------*/
size_t elibc_get_stdlib_memory_usage()
{

#ifdef _DEBUG
    /* get memory state */
    _CrtMemState memState;

    /* get memory snapshot */
    _CrtMemCheckpoint(&memState);

    /* count total memory usage */
    return memState.lSizes[_NORMAL_BLOCK] + memState.lSizes[_CLIENT_BLOCK];

#else

    /* sElibGetCrtMemoryUsage works only in debug mode */
    return 0;

#endif /* _DEBUG */

}

/* check if there is some memory left */
int elibc_check_memory_leaks()
{
    return _CrtDumpMemoryLeaks();
}

/*----------------------------------------------------------------------*/

#endif /* _ELIBC_ENABLE_PERFORMANCE_TOOLS */



