/*
 * Copyright (C) 2018  Bal Chettri
 * Licensed under GNU GPL(https://www.gnu.org/licenses/gpl.html)
 */
#include "stdafx.h"
#include <mobicast/mcJssHttpHandler.h>
#include <mobicast/platform/mac/objc-js/mcJsHttpRequest.h>
#include <mobicast/platform/mac/mcAppDelegate.h>
#include <mobicast/mcDebug.h>
#include <dispatch/dispatch.h>

namespace MobiCast
{

// Context for executing service in main thread.
struct ExecServiceBlockParams
{
    const char *    path;
    http::Request * pHttpRequest;
};

static void ExecServiceBlock(void *pParam)
{
    ExecServiceBlockParams *pParams = reinterpret_cast<ExecServiceBlockParams *>(pParam);
    MC_ASSERT(pParams != NULL);

    MCJSHttpRequest *request = [[MCJSHttpRequest alloc] initWithHttpRequest:pParams->pHttpRequest];
    [[MCAppDelegate instance].webview invokeService:[NSString stringWithUTF8String:pParams->path] httpRequest:request httpResponse:request.response];       
}

void JssHttpHandler::InvokeJssService(http::Request *req, const char *srvcPath)
{
    // Create dispatch parameters to invoke the service on main thread
    // synchronously.
    ExecServiceBlockParams params;
    params.path = srvcPath;
    params.pHttpRequest = req;
    
    dispatch_sync_f(dispatch_get_main_queue(), &params, &ExecServiceBlock);
}

} // MobiCast namespace
