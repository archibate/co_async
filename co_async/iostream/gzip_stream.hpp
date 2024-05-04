#pragma once

#if CO_ASYNC_ZLIB
#include <zlib.h>
#endif
#include <co_async/std.hpp>
#include <co_async/system/system_loop.hpp>
#include <co_async/awaiter/task.hpp>
#include <co_async/iostream/stream_base.hpp>
#include <co_async/iostream/zlib_stream.hpp>

namespace co_async {

#if CO_ASYNC_ZLIB
namespace gzip_stream_details {

// borrowed from: https://github.com/intel/zlib/blob/master/examples/gun.c

/* gun.c -- simple gunzip to give an example of the use of inflateBack()
 * Copyright (C) 2003, 2005, 2008, 2010, 2012 Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
   Version 1.7  12 August 2012  Mark Adler */

/* Version history:
   1.0  16 Feb 2003  First version for testing of inflateBack()
   1.1  21 Feb 2005  Decompress concatenated gzip streams
                     Remove use of "this" variable (C++ keyword)
                     Fix return value for in()
                     Improve allocation failure checking
                     Add typecasting for void * structures
                     Add -h option for command version and usage
                     Add a bunch of comments
   1.2  20 Mar 2005  Add Unix compress (LZW) decompression
                     Copy file attributes from input file to output file
   1.3  12 Jun 2005  Add casts for error messages [Oberhumer]
   1.4   8 Dec 2006  LZW decompression speed improvements
   1.5   9 Feb 2008  Avoid warning in latest version of gcc
   1.6  17 Jan 2010  Avoid signed/unsigned comparison warnings
   1.7  12 Aug 2012  Update for z_const usage in zlib 1.2.8
 */

/*
   gun [ -t ] [ name ... ]

   decompresses the data in the named gzip files.  If no arguments are given,
   gun will decompress from stdin to stdout.  The names must end in .gz, -gz,
   .z, -z, _z, or .Z.  The uncompressed data will be written to a file name
   with the suffix stripped.  On success, the original file is deleted.  On
   failure, the output file is deleted.  For most failures, the command will
   continue to process the remaining names on the command line.  A memory
   allocation failure will abort the command.  If -t is specified, then the
   listed files or stdin will be tested as gzip files for integrity (without
   checking for a proper suffix), no output will be written, and no files
   will be deleted.

   Like gzip, gun allows concatenated gzip streams and will decompress them,
   writing all of the uncompressed data to the output.  Unlike gzip, gun allows
   an empty file on input, and will produce no error writing an empty output
   file.

   gun will also decompress files made by Unix compress, which uses LZW
   compression.  These files are automatically detected by virtue of their
   magic header bytes.  Since the end of Unix compress stream is marked by the
   end-of-file, they cannot be concatenated.  If a Unix compress stream is
   encountered in an input file, it is the last stream in that file.

   Like gunzip and uncompress, the file attributes of the original compressed
   file are maintained in the final uncompressed file, to the extent that the
   user permissions allow it.

   On my Mac OS X PowerPC G4, gun is almost twice as fast as gunzip (version
   1.2.4) is on the same file, when gun is linked with zlib 1.2.2.  Also the
   LZW decompression provided by gun is about twice as fast as the standard
   Unix uncompress command.
 */

/* function declaration */
#define local inline

/* buffer constants */
#define SIZE 32768U /* input and output buffer sizes */
#define PIECE 16384 /* limits i/o chunks for 16-bit int case */

/* structure for infback() to pass to input function in() -- it maintains the
   input file and a buffer of size SIZE */
struct Ind {
    BorrowedStream *infile;
    unsigned char *inbuf;
};

/* Load input buffer, assumed to be empty, and return bytes loaded and a
   pointer to them.  read() is called until the buffer is full, or until it
   returns end-of-file or error.  Return 0 on error. */
local Task<Expected<unsigned>> in(void *in_desc, z_const unsigned char **buf) {
    int ret;
    unsigned len;
    unsigned char *next;
    struct Ind *me = (struct Ind *)in_desc;

    next = me->inbuf;
    *buf = next;
    len = 0;
    do {
        ret = PIECE;
        if ((unsigned)ret > SIZE - len)
            ret = (int)(SIZE - len);
        co_await co_await me->infile->read(next, ret);
        next += ret;
        len += ret;
    } while (ret != 0 && len < SIZE);
    co_return len;
}

/* structure for infback() to pass to output function out() -- it maintains the
   output file, a running CRC-32 check on the output and the total number of
   bytes output, both for checking against the gzip trailer.  (The length in
   the gzip trailer is stored modulo 2^32, so it's ok if a long is 32 bits and
   the output is greater than 4 GB.) */
struct Outd {
    BorrowedStream *outfile;
    int check; /* true if checking crc and total */
    unsigned long crc;
    unsigned long total;
};

/* Write output buffer and update the CRC-32 and total bytes written.  write()
   is called until all of the output is written or an error is encountered.
   On success out() returns 0.  For a write failure, out() returns 1.  If the
   output file descriptor is -1, then nothing is written.
 */
local Task<Expected<>> out(void *out_desc, unsigned char *buf, unsigned len) {
    int ret;
    struct Outd *me = (struct Outd *)out_desc;

    if (me->check) {
        me->crc = crc32(me->crc, buf, len);
        me->total += len;
    }
    if (me->outfile) [[likely]]
        do {
            ret = PIECE;
            if ((unsigned)ret > len)
                ret = (int)len;
            ret = co_await co_await me->outfile->write(buf, ret);
            buf += ret;
            len -= ret;
        } while (len != 0);
    co_return {};
}

local unsigned ib_in(void FAR *in_desc, z_const unsigned char FAR *FAR *buf) {
    int ret;
    unsigned len;
    unsigned char *next;
    struct Ind *me = (struct Ind *)in_desc;

    next = me->inbuf;
    *buf = next;
    len = 0;
    do {
        ret = PIECE;
        if ((unsigned)ret > SIZE - len)
            ret = (int)(SIZE - len);
        /* debug(), "reading", ret; */
        ret = (int)me->infile->tryread(next, ret);
        /* debug(), "reading got", ret; */
        next += ret;
        len += ret;
    } while (ret != 0 && len < SIZE);
    return len;
}

local int ib_out(void FAR *out_desc, unsigned char FAR *buf, unsigned len) {
    int ret;
    struct Outd *me = (struct Outd *)out_desc;

    if (me->check) {
        me->crc = crc32(me->crc, buf, len);
        me->total += len;
    }
    if (me->outfile) [[likely]]
        do {
            ret = PIECE;
            if ((unsigned)ret > len)
                ret = (int)len;
            auto oldret = ret;
            /* debug(), "writing", ret, std::string_view((char *)buf, ret); */
            ret = (int)me->outfile->trywrite(buf, ret);
            /* debug(), "writing got", ret; */
            if (ret == -1 || ret != oldret)
                return 1;
            buf += ret;
            len -= ret;
        } while (len != 0);
    return 0;
}

/* next input byte macro for use inside lunpipe() and gunpipe() */
#define NEXT()                               \
    (have ? 0 : (have = ib_in(indp, &next)), \
     last = have ? (have--, (int)(*next++)) : -1)

/* memory for gunpipe() and lunpipe() --
   the first 256 entries of prefix[] and suffix[] are never used, could
   have offset the index, but it's faster to waste the memory */
local unsigned char inbuf[SIZE];      /* input buffer */
local unsigned char outbuf[SIZE];     /* output buffer */
local unsigned short prefix[65536];   /* index to LZW prefix string */
local unsigned char suffix[65536];    /* one-character LZW suffix */
local unsigned char match[65280 + 2]; /* buffer for reversed match or gzip
                                           32K sliding window */

/* throw out what's left in the current bits byte buffer (this is a vestigial
   aspect of the compressed data format derived from an implementation that
   made use of a special VAX machine instruction!) */
#define FLUSHCODE()               \
    do {                          \
        left = 0;                 \
        rem = 0;                  \
        if (chunk > have) {       \
            chunk -= have;        \
            have = 0;             \
            if (NEXT() == -1)     \
                break;            \
            chunk--;              \
            if (chunk > have) {   \
                chunk = have = 0; \
                break;            \
            }                     \
        }                         \
        have -= chunk;            \
        next += chunk;            \
        chunk = 0;                \
    } while (0)

/* Decompress a compress (LZW) file from indp to outfile.  The compress magic
   header (two bytes) has already been read and verified.  There are have bytes
   of buffered input at next.  strm is used for passing error information back
   to gunpipe().

   lunpipe() will return Z_OK on success, Z_BUF_ERROR for an unexpected end of
   file, read error, or write error (a write error indicated by strm->next_in
   not equal to Z_NULL), or Z_DATA_ERROR for invalid input.
 */
local Task<int> lunpipe(unsigned have, z_const unsigned char *next,
                        struct Ind *indp, BorrowedStream *outfile,
                        z_stream *strm) {
    int last;         /* last byte read by NEXT(), or -1 if EOF */
    unsigned chunk;   /* bytes left in current chunk */
    int left;         /* bits left in rem */
    unsigned rem;     /* unused bits from input */
    int bits;         /* current bits per code */
    unsigned code;    /* code, table traversal index */
    unsigned mask;    /* mask for current bits codes */
    int max;          /* maximum bits per code for this stream */
    unsigned flags;   /* compress flags, then block compress flag */
    unsigned end;     /* last valid entry in prefix/suffix tables */
    unsigned temp;    /* current code */
    unsigned prev;    /* previous code */
    unsigned final;   /* last character written for previous code */
    unsigned stack;   /* next position for reversed string */
    unsigned outcnt;  /* bytes in output buffer */
    struct Outd outd; /* output structure */
    unsigned char *p;

    /* set up output */
    outd.outfile = outfile;
    outd.check = 0;

    /* process remainder of compress header -- a flags byte */
    flags = NEXT();
    if (last == -1)
        co_return Z_BUF_ERROR;
    if (flags & 0x60) {
        strm->msg = (char *)"unknown lzw flags set";
        co_return Z_DATA_ERROR;
    }
    max = flags & 0x1f;
    if (max < 9 || max > 16) {
        strm->msg = (char *)"lzw bits out of range";
        co_return Z_DATA_ERROR;
    }
    if (max == 9) /* 9 doesn't really mean 9 */
        max = 10;
    flags &= 0x80; /* true if block compress */

    /* clear table */
    bits = 9;
    mask = 0x1ff;
    end = flags ? 256 : 255;

    /* set up: get first 9-bit code, which is the first decompressed byte, but
       don't create a table entry until the next code */
    if (NEXT() == -1) /* no compressed data is ok */
        co_return Z_OK;
    final = prev = (unsigned)last; /* low 8 bits of code */
    if (NEXT() == -1)              /* missing a bit */
        co_return Z_BUF_ERROR;
    if (last & 1) { /* code must be < 256 */
        strm->msg = (char *)"invalid lzw code";
        co_return Z_DATA_ERROR;
    }
    rem = (unsigned)last >> 1; /* remaining 7 bits */
    left = 7;
    chunk = bits - 2;                  /* 7 bytes left in this chunk */
    outbuf[0] = (unsigned char) final; /* write first decompressed byte */
    outcnt = 1;

    /* decode codes */
    stack = 0;
    for (;;) {
        /* if the table will be full after this, increment the code size */
        if (end >= mask && bits < max) {
            FLUSHCODE();
            bits++;
            mask <<= 1;
            mask++;
        }

        /* get a code of length bits */
        if (chunk == 0) /* decrement chunk modulo bits */
            chunk = bits;
        code = rem;         /* low bits of code */
        if (NEXT() == -1) { /* EOF is end of compressed data */
            /* write remaining buffered output */
            if (outcnt && !ib_out(&outd, outbuf, outcnt)) {
                strm->next_in = outbuf; /* signal write error */
                co_return Z_BUF_ERROR;
            }
            co_return Z_OK;
        }
        code += (unsigned)last << left; /* middle (or high) bits of code */
        left += 8;
        chunk--;
        if (bits > left) {    /* need more bits */
            if (NEXT() == -1) /* can't end in middle of code */
                co_return Z_BUF_ERROR;
            code += (unsigned)last << left; /* high bits of code */
            left += 8;
            chunk--;
        }
        code &= mask;                       /* mask to current code length */
        left -= bits;                       /* number of unused bits */
        rem = (unsigned)last >> (8 - left); /* unused bits from last byte */

        /* process clear code (256) */
        if (code == 256 && flags) {
            FLUSHCODE();
            bits = 9; /* initialize bits and mask */
            mask = 0x1ff;
            end = 255; /* empty table */
            continue;  /* get next code */
        }

        /* special code to reuse last match */
        temp = code; /* save the current code */
        if (code > end) {
            /* Be picky on the allowed code here, and make sure that the code
               we drop through (prev) will be a valid index so that random
               input does not cause an exception.  The code != end + 1 check is
               empirically derived, and not checked in the original uncompress
               code.  If this ever causes a problem, that check could be safely
               removed.  Leaving this check in greatly improves gun's ability
               to detect random or corrupted input after a compress header.
               In any case, the prev > end check must be retained. */
            if (code != end + 1 || prev > end) {
                strm->msg = (char *)"invalid lzw code";
                co_return Z_DATA_ERROR;
            }
            match[stack++] = (unsigned char) final;
            code = prev;
        }

        /* walk through linked list to generate output in reverse order */
        p = match + stack;
        while (code >= 256) {
            *p++ = suffix[code];
            code = prefix[code];
        }
        stack = p - match;
        match[stack++] = (unsigned char)code;
        final = code;

        /* link new table entry */
        if (end < mask) {
            end++;
            prefix[end] = (unsigned short)prev;
            suffix[end] = (unsigned char) final;
        }

        /* set previous code for next iteration */
        prev = temp;

        /* write output in forward order */
        while (stack > SIZE - outcnt) {
            while (outcnt < SIZE)
                outbuf[outcnt++] = match[--stack];
            if (!ib_out(&outd, outbuf, outcnt)) {
                strm->next_in = outbuf; /* signal write error */
                co_return Z_BUF_ERROR;
            }
            outcnt = 0;
        }
        p = match + stack;
        do {
            outbuf[outcnt++] = *--p;
        } while (p > match);
        stack = 0;

        /* loop for next code with final and prev as the last match, rem and
           left provide the first 0..7 bits of the next code, end is the last
           valid table entry */
    }
    co_return Z_OK;
}

/* Decompress a gzip file from infile to outfile.  strm is assumed to have been
   successfully initialized with inflateBackInit().  The input file may consist
   of a series of gzip streams, in which case all of them will be decompressed
   to the output file.  If outfile is -1, then the gzip stream(s) integrity is
   checked and nothing is written.

   The return value is a zlib error code: Z_MEM_ERROR if out of memory,
   Z_DATA_ERROR if the header or the compressed data is invalid, or if the
   trailer CRC-32 check or length doesn't match, Z_BUF_ERROR if the input ends
   prematurely or a write error occurs, or Z_ERRNO if junk (not a another gzip
   stream) follows a valid gzip stream.
 */
local Task<int> gunpipe(z_stream *strm, BorrowedStream *infile,
                        BorrowedStream *outfile) {
    infile->allocinbuf(SIZE);
    outfile->allocoutbuf(SIZE);

    int ret, first, last;
    unsigned have, flags, len;
    z_const unsigned char *next = NULL;
    struct Ind ind, *indp;
    struct Outd outd;

    /* setup input buffer */
    ind.infile = infile;
    ind.inbuf = inbuf;
    indp = &ind;

    /* decompress concatenated gzip streams */
    have = 0;               /* no input data read in yet */
    first = 1;              /* looking for first gzip header */
    strm->next_in = Z_NULL; /* so Z_BUF_ERROR means EOF */
    for (;;) {
        if (infile->bufempty()) {
            debug(), "fillbuf";
            auto ok = (co_await infile->fillbuf()).has_value();
            debug(), "fillbuf", ok;
        }
        /* look for the two magic header bytes for a gzip stream */
        if (NEXT() == -1) {
            ret = Z_OK;
            break; /* empty gzip stream is ok */
        }
        if (last != 31 || (NEXT() != 139 && last != 157)) {
            strm->msg = (char *)"incorrect header check";
            ret = first ? Z_DATA_ERROR : Z_ERRNO;
            break; /* not a gzip or compress header */
        }
        first = 0; /* next non-header is junk */

        /* process a compress (LZW) file -- can't be concatenated after this */
        if (last == 157) {
            ret = co_await lunpipe(have, next, indp, outfile, strm);
            break;
        }

        /* process remainder of gzip header */
        ret = Z_BUF_ERROR;
        if (NEXT() != 8) { /* only deflate method allowed */
            if (last == -1)
                break;
            strm->msg = (char *)"unknown compression method";
            ret = Z_DATA_ERROR;
            break;
        }
        flags = NEXT(); /* header flags */
        NEXT();         /* discard mod time, xflgs, os */
        NEXT();
        NEXT();
        NEXT();
        NEXT();
        NEXT();
        if (last == -1)
            break;
        if (flags & 0xe0) {
            strm->msg = (char *)"unknown header flags set";
            ret = Z_DATA_ERROR;
            break;
        }
        if (flags & 4) { /* extra field */
            len = NEXT();
            len += (unsigned)(NEXT()) << 8;
            if (last == -1)
                break;
            while (len > have) {
                len -= have;
                have = 0;
                if (NEXT() == -1)
                    break;
                len--;
            }
            if (last == -1)
                break;
            have -= len;
            next += len;
        }
        if (flags & 8) /* file name */
            while (NEXT() != 0 && last != -1)
                ;
        if (flags & 16) /* comment */
            while (NEXT() != 0 && last != -1)
                ;
        if (flags & 2) { /* header crc */
            NEXT();
            NEXT();
        }
        if (last == -1)
            break;

        /* set up output */
        outd.outfile = outfile;
        outd.check = 1;
        outd.crc = crc32(0L, Z_NULL, 0);
        outd.total = 0;

        /* decompress data to output */
        strm->next_in = next;
        strm->avail_in = have;
        ret = inflateBack(strm, ib_in, indp, ib_out, &outd);
        if (ret != Z_STREAM_END)
            break;
        next = strm->next_in;
        have = strm->avail_in;
        strm->next_in = Z_NULL; /* so Z_BUF_ERROR means EOF */
        (void)co_await outfile->flush();

        /* check trailer */
        ret = Z_BUF_ERROR;
        if (NEXT() != (int)(outd.crc & 0xff) ||
            NEXT() != (int)((outd.crc >> 8) & 0xff) ||
            NEXT() != (int)((outd.crc >> 16) & 0xff) ||
            NEXT() != (int)((outd.crc >> 24) & 0xff)) {
            /* crc error */
            if (last != -1) {
                strm->msg = (char *)"incorrect data check";
                ret = Z_DATA_ERROR;
            }
            break;
        }
        if (NEXT() != (int)(outd.total & 0xff) ||
            NEXT() != (int)((outd.total >> 8) & 0xff) ||
            NEXT() != (int)((outd.total >> 16) & 0xff) ||
            NEXT() != (int)((outd.total >> 24) & 0xff)) {
            /* length error */
            if (last != -1) {
                strm->msg = (char *)"incorrect length check";
                ret = Z_DATA_ERROR;
            }
            break;
        }

        /* go back and look for another gzip stream */
    }

    /* clean up and return */
    co_return ret;
}

/* Process the gun command line arguments.  See the command syntax near the
   beginning of this source file. */
local Task<int> gunzip(BorrowedStream &infile, BorrowedStream &outfile) {
    int ret;
    unsigned char *window;
    z_stream strm;

    /* initialize inflateBack state for repeated use */
    window = match; /* reuse LZW match buffer */
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    ret = inflateBackInit(&strm, 15, window);
    if (ret != Z_OK) [[unlikely]] {
        co_return Z_MEM_ERROR;
    }
    auto res = co_await gunpipe(&strm, &infile, &outfile);
    inflateBackEnd(&strm);
    co_return res;
}

} // namespace gzip_stream_details

#undef NEXT
#undef SIZE
#undef PIECE
#undef local
#undef FLUSHCODE

inline Task<Expected<void, std::errc>> zlib_gunzip(BorrowedStream &infile,
                                                   BorrowedStream &outfile) {
    int ret = co_await gzip_stream_details::gunzip(infile, outfile);
    if (ret != Z_OK) [[unlikely]] {
        switch (ret) {
        case Z_MEM_ERROR: co_return Unexpected{std::errc::not_enough_memory};
        default: co_return Unexpected{std::errc::io_error};
        };
    }
    co_return {};
}
#else
inline Task<Expected<void, std::errc>> zlib_gunzip(BorrowedStream &infile,
                                                   BorrowedStream &outfile) {
    co_return Unexpected{std::errc::function_not_supported};
}
#endif

inline Task<Expected<void, std::errc>> zlib_gzip(BorrowedStream &infile,
                                                 BorrowedStream &outfile) {
    // TODO: how to support gzip?
    co_return Unexpected{std::errc::function_not_supported};
}

} // namespace co_async
