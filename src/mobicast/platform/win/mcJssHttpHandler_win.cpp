/*
 * Copyright (C) 2018  Bal Chettri
 * Licensed under GNU GPL(https://www.gnu.org/licenses/gpl.html)
 */
#include "stdafx.h"
#include <mobicast/mcJssHttpHandler.h>
#include <mobicast/platform/win/COM/mcComHttpRequest.h>
#include <mobicast/platform/win/mcWindow.h>
#include <mobicast/mcThread.h>
#include <mobicast/mcDebug.h>

extern DWORD g_mainThreadId;
extern MobiCast::Window *g_window;

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

    _HttpRequest *pHttpRequest = new MobiCast::CHttpRequest(pParams->pHttpRequest);

    _HttpResponse *pHttpResponse;
    pHttpRequest->get_response(&pHttpResponse);

    if(!g_window->GetBrowser()->InvokeService(pParams->path, pHttpRequest, pHttpResponse))
    {
        MC_LOGE("InvokeService returned false for service '%s'.", pParams->path);
    }

    pHttpResponse->Release();
    pHttpRequest->Release();
}

void JssHttpHandler::InvokeJssService(http::Request *req, const char *srvcPath)
{
    // Create dispatch parameters to invoke the service on main thread synchronously.
    ExecServiceBlockParams params;
    params.path = srvcPath;
    params.pHttpRequest = req;
    
    // Dispatch service block for execution on main thread.
    ThreadDispatchBlock(g_mainThreadId, &params, &ExecServiceBlock);
}

} // MobiCast namespace
