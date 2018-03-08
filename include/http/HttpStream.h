/*
 * HttpStream.h    Output memory stream.
 *
 * Copyright (C) 2018  Bal Chettri
 * Licensed under GNU GPL(https://www.gnu.org/licenses/gpl.html)
 */
#ifndef __HTTP_STREAM_H__
#define __HTTP_STREAM_H__

#include <http/HttpTypes.h>

namespace http
{

/** Stream class. Output stream with linear growth. */
class Stream
{
public:
    Stream(size_t size, size_t grow_size);
    ~Stream();

    /** Closes the stream releasing internal memory resources. */
    void Close();

    /** Writes a single character to the stream. */
    int Write(char ch);

    /** Writes a formatted string to the stream. */
    int Printf(const char *format, ...);

    /** Returns pointer to data buffer. */
    inline const char *GetData() const
    {
        return _buff;
    }

    /** Returns length of data. */
    inline size_t GetLength() const
    {
        return _len;
    }

    /** Resets the stream. */
    void Reset(bool free_buff = true);

private:
    Stream(const Stream &);
    Stream &operator=(const Stream &);

    char *_buff;
    size_t _size;
    size_t _growsize;
    size_t _len;
};

} // http namespace

#endif /* !__HTTP_STREAM_H__ */
