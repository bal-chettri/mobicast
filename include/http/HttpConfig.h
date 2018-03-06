/*
 * HttpConfig.h    Configuration header for http lib.
 *
 * Copyright (C) 2018  Bal Chettri
 * Licensed under GNU GPL(https://www.gnu.org/licenses/gpl.html)
 */
#ifndef __HTTP_CONFIG_H__
#define __HTTP_CONFIG_H__

// Size of http stream buffers.
#ifndef HTTP_BUFFER_SIZE
#define HTTP_BUFFER_SIZE            1024
#endif

// Initial size of http stream buffers.
#ifndef HTTP_STREAM_DEF_SIZE
#define HTTP_STREAM_DEF_SIZE        0
#endif

// Grow size for http stream buffers.
#ifndef HTTP_STREAM_GROW_SIZE
#define HTTP_STREAM_GROW_SIZE       512
#endif

// HTTP poll duration for `select()`.
#ifndef HTTP_POLL_DURATION
#define HTTP_POLL_DURATION          5
#endif

// Enables forcefull binding to port. Default is 1.
#ifndef HTTP_REUSE_PORT
#define HTTP_REUSE_PORT             1
#endif

// Stack size for client threads.
#ifndef HTTP_THREAD_STACK_SIZE
#define HTTP_THREAD_STACK_SIZE      (1 * 1024)
#endif

#endif // !__HTTP_CONFIG_H__
