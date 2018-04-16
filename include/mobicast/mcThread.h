/*
 * Copyright (C) 2018  Bal Chettri
 * Licensed under GNU GPL(https://www.gnu.org/licenses/gpl.html)
 */
#ifndef __MOBICAST_THREAD_H__
#define __MOBICAST_THREAD_H__

#include <mobicast/mcTypes.h>

#ifdef MC_PLATFORM_WIN
#   include <Windows.h>
#endif

namespace MobiCast
{

#ifdef MC_PLATFORM_WIN

// ID for thread dispatch message.
#define THREAD_DISPATCH_MSGID           (WM_USER + 0x1000)

/** Function type for thread block to be used with dispatch functions. */
typedef void (* ThreadBlockFunc)(void *);

/** 
 * THREAD_DISPATCH_INFO struct. Contains info for dispatching an execution block 
 * to a target thread.
 */
struct THREAD_DISPATCH_INFO
{
    // Execution block to invoke when a message is received on target thread.
    ThreadBlockFunc pExecBlock;

    // Contextual data passed to the execution block.
    void *pExecBlockContext;
    
    // Event set on completion of the execution block.
    HANDLE hCompleteEvent;
};

/** Dispatches block for execution on specified thread. */
void ThreadDispatchBlock(DWORD threadId, void *pExecBlockContext, ThreadBlockFunc pExecBlock);

/** Dispatches block for asynchronous execution on specified thread . */
void ThreadDispatchBlockAsync(DWORD threadId, void *pExecBlockContext, ThreadBlockFunc pExecBlock);

/** 
 * Handles thread dispatch call on target thread. This should be called on the target thread when 
 * THREAD_DISPATCH_MSGID message is received.
 */
void ThreadDispatchHandle(THREAD_DISPATCH_INFO *pInfo);

#endif // MC_PLATFORM_WIN

} // MobiCast namespace

#endif // !__MOBICAST_THREAD_H__
