/* uncompr.c -- decompress a memory buffer
 * Copyright (C) 1995-2003, 2010, 2014, 2016 Jean-loup Gailly, Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

/*
 * Adaption of zlib uncompr.c to nx hardware
 * Author: Xiao Lei Hu  <xlhu@cn.ibm.com>
 *
 */

#include <zlib.h>
#include "nx_zlib.h"

int nx_uncompress2(Bytef *dest, uLongf *destLen, const Bytef *source, uLong *sourceLen)
{
    z_stream stream;
    int err;
    const uInt max = 1<<30U;
    uLong len, left;
    Byte buf[1];    /* for detection of incomplete stream when *destLen == 0 */

    len = *sourceLen;
    if (*destLen) {
        left = *destLen;
        *destLen = 0;
    }
    else {
        left = 1;
        dest = buf;
    }

    stream.next_in = (z_const Bytef *)source;
    stream.avail_in = 0;
    stream.zalloc = (alloc_func)0;
    stream.zfree = (free_func)0;
    stream.opaque = (voidpf)0;

    err = nx_inflateInit(&stream);
    if (err != Z_OK) return err;

    stream.next_out = dest;
    stream.avail_out = 0;
    prt_info("uncompress begin: sourceLen %ld\n", *sourceLen);
    do {
        if (stream.avail_out == 0) {
            stream.avail_out = left > (uLong)max ? max : (uInt)left;
            left -= stream.avail_out;
        }
        if (stream.avail_in == 0) {
            stream.avail_in = len > (uLong)max ? max : (uInt)len;
            len -= stream.avail_in;
        }
        err = nx_inflate(&stream, Z_NO_FLUSH);
    } while (err == Z_OK);

    *sourceLen -= len + stream.avail_in;
    if (dest != buf)
        *destLen = stream.total_out;
    else if (stream.total_out && err == Z_BUF_ERROR)
        left = 1;

    prt_info("uncompress end: stream.total_out %ld\n", stream.total_out);
    nx_inflateEnd(&stream);
    return err == Z_STREAM_END ? Z_OK :
           err == Z_NEED_DICT ? Z_DATA_ERROR  :
           err == Z_BUF_ERROR && left + stream.avail_out ? Z_DATA_ERROR :
           err;
}

int nx_uncompress(Bytef *dest, uLongf *destLen, const Bytef *source, uLong sourceLen)
{
    return nx_uncompress2(dest, destLen, source, &sourceLen);
}

#ifdef ZLIB_API

int uncompress(Bytef *dest, uLongf *destLen, const Bytef *source, uLong sourceLen)
{
	return nx_uncompress(dest, destLen, source, sourceLen);
}
int uncompress2(Bytef *dest, uLongf *destLen, const Bytef *source, uLong *sourceLen)
{
	return nx_uncompress2(dest, destLen, source, sourceLen);
}

#endif

