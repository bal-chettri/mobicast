/*
 * HttpCore.h    Defines core types, enumerations and structures for HTTP lib.
 *
 * Copyright (C) 2018  Bal Chettri
 * Licensed under GNU GPL(https://www.gnu.org/licenses/gpl.html)
 */
#ifndef __HTTP_CORE_H__
#define __HTTP_CORE_H__

#include <http/HttpTypes.h>

namespace http
{

/** HttpError enum. HTTP lib internal error codes. */
enum Error
{
    kErrorNone = 0,
    kErrorReadClient,
    kErrorWriteClient,
    kErrorProtocol,
    kErrorProtocolVersion,
    kErrorInvalidState,
    kErrorMemory,
    kErrorIO,
};

/** StatusCode enum. HTTP response status code enumerations. */
enum StatusCode
{
    kStatusContinue = 100,
    kStatusSwitchingProtocol = 101,
    kStatusOk = 200,
    kStatusCreated = 201,
    kStatusAccepted = 202,
    kStatusNonAuthorativeInformation = 203,
    kStatusNoContent = 204,
    kStatusResetContent = 205,
    kStatusPartialContent = 206,
    kStatusMultipleChoices = 300,
    kStatusMovedPermanently = 301,
    kStatusFound = 302,
    kStatusSeeOther = 303,
    kStatusNotModified = 304,
    kStatusUseProxy = 305,
    kStatusTemporaryRedirect = 307,
    kStatusBadRequest = 400,
    kStatusUnauthorized = 401,
    kStatusPaymentRequired = 402,
    kStatusForbidden = 403,
    kStatusNotFound = 404,
    kStatusMethodNotAllowed = 405,
    kStatusNotAcceptable = 406,
    kStatusProxyAuthenticationRequired = 407,
    kStatusRequestTimeout = 408,
    kStatusConflict = 409,
    kStatusGone = 410,
    kStatusLengthRequired = 411,
    kStatusPreconditionFailed = 412,
    kStatusRequestEntityTooLarge = 413,
    kStatusRequestUriTooLong = 414,
    kStatusUnsupportedMediaType = 415,
    kStatusRequestedRangeNotSatisfiable = 416,
    kStatusExpectationFailed = 417,
    kStatusInternalServerError = 500,
    kStatusNotImplemented = 501,
    kStatusBadGateway = 502,
    kStatusServiceUnavailable = 503,
    kStatusGatewayTimeout = 504,
    kStatusHttpVersionNotSupported = 505
};

/** Version struct.    HTTP protocol version info structure. */
struct Version
{
    short maj;
    short min;
};

} // http namespace

#endif // !__HTTP_CORE_H__
