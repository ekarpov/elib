/*
 *  ELib memory testing helpers
 */

#ifndef _ELIB_TESTS_MEMORY_H_
#define _ELIB_TESTS_MEMORY_H_

#include <gtest/gtest.h>

#if defined(WIN32) || defined(_WIN64)

/*
* Memory leak detector for Windows (ref: https://stackoverflow.com/a/70968385)
*/

#include <crtdbg.h>


class ELibGTestMemoryLeakDetector {
public:
    ELibGTestMemoryLeakDetector() {
        _CrtMemCheckpoint(&_memState);
    }

    ~ELibGTestMemoryLeakDetector() {
        _CrtMemState stateNow, stateDiff;
        _CrtMemCheckpoint(&stateNow);
        int diffResult = _CrtMemDifference(&stateDiff, &_memState, &stateNow);
        if (diffResult)
            _reportFailure(stateDiff.lSizes[1]);
    }
private:
    _CrtMemState _memState;

    void _reportFailure(size_t unfreedBytes) {
        FAIL() << "Memory leak of " << unfreedBytes << " byte(s) detected.";
    }
};

#define ELIB_GTEST_MEMORY_LEAK_DETECTOR   ELibGTestMemoryLeakDetector __memoryLeakDetector

#else

#define ELIB_GTEST_MEMORY_LEAK_DETECTOR()   

#endif /* defined(WIN32) || defined(_WIN64) */

#endif /* _ELIB_TESTS_MEMORY_H_ */

