/*
 * Copyright (C) 2018  Bal Chettri
 * Licensed under GNU GPL(https://www.gnu.org/licenses/gpl.html)
 */
#include "stdafx.h"
#include <mobicast/mcThread.h>
#include <mobicast/mcDebug.h>

namespace MobiCast
{

static void ThreadDispatchBlockHelper(DWORD threadId, THREAD_DISPATCH_INFO *pInfo)
{
    MC_ASSERT(threadId != GetCurrentThreadId() && pInfo != NULL);

    // Create Event to wait for target thread to complete.
    pInfo->hCompleteEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

    // Post message to the target thread, which should call ThreadDispatchHandle
    // to process the message.
    BOOL success = PostThreadMessage(threadId, THREAD_DISPATCH_MSGID, 0, (LPARAM)pInfo);
    MC_ASSERT(success == TRUE);

    // Wait for target thread to signal the completion.
    DWORD retCode = WaitForSingleObject(pInfo->hCompleteEvent, INFINITE);

    CloseHandle(pInfo->hCompleteEvent);
}

static void ThreadDispatchBlockAsyncHelper(DWORD threadId, THREAD_DISPATCH_INFO *pInfo)
{
    MC_ASSERT(threadId != GetCurrentThreadId() && pInfo != NULL);

    // Set Event to NULL for async mode.
    pInfo->hCompleteEvent = NULL;

    // Post message to the target thread, which should call ThreadDispatchHandle
    // to process the message.
    BOOL success = PostThreadMessage(threadId, THREAD_DISPATCH_MSGID, 0, (LPARAM)pInfo);
    MC_ASSERT(success == TRUE);
}

void ThreadDispatchBlock(DWORD threadId, void *pExecBlockContext, ThreadBlockFunc pExecBlock)
{
    THREAD_DISPATCH_INFO info;

    info.pExecBlock = pExecBlock;
    info.pExecBlockContext = pExecBlockContext;

    ThreadDispatchBlockHelper(threadId, &info);
}

void ThreadDispatchBlockAsync(DWORD threadId, void *pExecBlockContext, ThreadBlockFunc pExecBlock)
{
    THREAD_DISPATCH_INFO info;

    info.pExecBlock = pExecBlock;
    info.pExecBlockContext = pExecBlockContext;

    ThreadDispatchBlockAsyncHelper(threadId, &info);
}

void ThreadDispatchHandle(THREAD_DISPATCH_INFO *pInfo)
{    
    MC_ASSERT(pInfo != NULL);

    // Invoke the execution block.
    pInfo->pExecBlock(pInfo->pExecBlockContext);

    // Signal the sender thread about the completion of block.
    if(pInfo->hCompleteEvent != NULL) {
        SetEvent(pInfo->hCompleteEvent);
    }
}

} // MobiCast namespace
